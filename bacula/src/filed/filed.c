/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2020 Kern Sibbald

   The original author of Bacula is Kern Sibbald, with contributions
   from many others, a complete list can be found in the file AUTHORS.

   You may use this file and others of this release according to the
   license defined in the LICENSE file, which includes the Affero General
   Public License, v3.0 ("AGPLv3") and some additional permissions and
   terms pursuant to its AGPLv3 Section 7.

   This notice must be preserved when any source code is
   conveyed and/or propagated.

   Bacula(R) is a registered trademark of Kern Sibbald.
*/
/*
 *  Bacula File Daemon
 *
 *    Kern Sibbald, March MM
 *
 */

#include "bacula.h"
#include "filed.h"

/* Imported Functions */
extern void *handle_connection_request(void *dir_sock);
extern bool parse_fd_config(CONFIG *config, const char *configfile, int exit_code);

/* Forward referenced functions */
static bool check_resources();

/* Exported variables */
CLIENT *me;                           /* my resource */
bool no_signals = false;
extern struct s_cmds cmds[];
bstatcollect *statcollector = NULL;
fdstatmetrics_t fdstatmetrics;

#ifndef CONFIG_FILE                   /* Might be overwritten */
 #define CONFIG_FILE "bacula-fd.conf" /* default config file */
 #define PROG_NAME   "bacula-fd"
#endif

char *configfile = NULL;
static bool test_config = false;
static bool foreground = false;
static bool make_pid_file = true;     /* create pid file */
static workq_t dir_workq;             /* queue of work from Director */
static pthread_t server_tid;
static CONFIG *config;

static void usage()
{
   fprintf(stderr, _(
      PROG_COPYRIGHT
      "\n%sVersion: %s (%s)\n\n"
      "Usage: bacula-fd [-f -s] [-c config_file] [-d debug_level]\n"
      "     -c <file>        use <file> as configuration file\n"
      "     -d <n>[,<tags>]  set debug level to <nn>, debug tags to <tags>\n"
      "     -dt              print a timestamp in debug output\n"
      "     -f               run in foreground (for debugging)\n"
      "     -g               groupid\n"
      "     -k               keep readall capabilities\n"
      "     -m               print kaboom output (for debugging)\n"
      "     -P               do not create pid file\n"
      "     -s               no signals (for debugging)\n"
      "     -t               test configuration file and exit\n"
      "     -T               set trace on\n"
      "     -u               userid\n"
      "     -v               verbose user messages\n"
      "     -?               print this message.\n"
      "\n"), 2000, BDEMO, VERSION, BDATE);

   exit(1);
}

/*********************************************************************
 *
 *  Main Bacula Unix Client Program
 *
 */
#if defined(HAVE_WIN32)
#define main BaculaMain
#endif

int main (int argc, char *argv[])
{
   int ch;
   bool keep_readall_caps = false;
   char *uid = NULL;
   char *gid = NULL;

   mark_heap();
   setlocale(LC_ALL, "");
   bindtextdomain("bacula", LOCALEDIR);
   textdomain("bacula");

   init_stack_dump();
   my_name_is(argc, argv, PROG_NAME);
   init_msg(NULL, NULL);
   daemon_start_time = time(NULL);
   setup_daemon_message_queue();

   while ((ch = getopt(argc, argv, "c:d:fg:kmPstTu:v?D:")) != -1) {
      switch (ch) {
      case 'c':                    /* configuration file */
         if (configfile != NULL) {
            free(configfile);
         }
         configfile = bstrdup(optarg);
         break;

      case 'd':                    /* debug level */
         if (*optarg == 't') {
            dbg_timestamp = true;
         } else {
            char *p;
            /* We probably find a tag list -d 10,sql,bvfs */
            if ((p = strchr(optarg, ',')) != NULL) {
               *p = 0;
            }
            debug_level = atoi(optarg);
            if (debug_level <= 0) {
               debug_level = 1;
            }
            if (p) {
               debug_parse_tags(p+1, &debug_level_tags);
            }
         }
         break;

      case 'f':                    /* run in foreground */
         foreground = true;
         break;

      case 'g':                    /* set group */
         gid = optarg;
         break;

      case 'k':
         keep_readall_caps = true;
         break;

      case 'm':                    /* print kaboom output */
         prt_kaboom = true;
         break;

      case 'P':
         make_pid_file = false;
         break;

      case 's':
         no_signals = true;
         break;

      case 't':
         test_config = true;
         break;

      case 'T':
         set_trace(true);
         break;

      case 'u':                    /* set userid */
         uid = optarg;
         break;

      case 'v':                    /* verbose */
         verbose++;
         break;

      case '?':
      default:
         usage();

      }
   }
   argc -= optind;
   argv += optind;

   if (argc) {
      if (configfile != NULL)
         free(configfile);
      configfile = bstrdup(*argv);
      argc--;
      argv++;
   }
   if (argc) {
      usage();
   }

   if (!uid && keep_readall_caps) {
      Emsg0(M_ERROR_TERM, 0, _("-k option has no meaning without -u option.\n"));
   }

   server_tid = pthread_self();

   if (configfile == NULL) {
      configfile = bstrdup(CONFIG_FILE);
   }

   if (!foreground && !test_config) {
      daemon_start();
      init_stack_dump();              /* set new pid */
   }

   if (!no_signals) {
      init_signals(terminate_filed);
   } else {
      /* This reduces the number of signals facilitating debugging */
      watchdog_sleep_time = 120;      /* long timeout for debugging */
   }

   config = New(CONFIG());
   parse_fd_config(config, configfile, M_ERROR_TERM);

   if (init_crypto() != 0) {
      Emsg0(M_ERROR, 0, _("Cryptography library initialization failed.\n"));
      terminate_filed(1);
   }

   if (!check_resources()) {
      Emsg1(M_ERROR, 0, _("Please correct configuration file: %s\n"), configfile);
      terminate_filed(1);
   }

   if (crypto_check_fips(me->require_fips) < 0) {
      Emsg0(M_ERROR_TERM, 0, _("Enable to set FIPS mode\n"));
   }

   set_working_directory(me->working_directory);

   if (test_config) {
      terminate_filed(0);
   }
   /* relocate trace file if needed, must be run after set_working_directory()
    * and my_name_is() */
   update_trace_file_location(false);

   set_thread_concurrency(me->MaxConcurrentJobs + 10);
   lmgr_init_thread(); /* initialize the lockmanager stack */

   /* Maximum 1 daemon at a time */
   if (make_pid_file) {
      create_pid_file(me->pid_directory, PROG_NAME,
                      get_first_port_host_order(me->FDaddrs));
   }
   read_state_file(me->working_directory, PROG_NAME,
                   get_first_port_host_order(me->FDaddrs));

   load_fd_plugins(me->plugin_directory);

   drop(uid, gid, keep_readall_caps);

#ifdef BOMB
   me += 1000000;
#endif
   /* initialize a statistics collector */
   initialize_statcollector();

   /* Setup default value for the the snapshot handler */
   if (!me->snapshot_command) {
      me->snapshot_command = snapshot_get_command();
   }

   /* The Dedup Index Directory directive is deprecated, but we still
    * support it in the code
    */
   me->allow_dedup_cache = (me->dedup_index_dir != NULL) || me->allow_dedup_cache;

   if (!no_signals) {
      start_watchdog();               /* start watchdog thread */
      init_jcr_subsystem();           /* start JCR watchdogs etc. */
   }

   start_collector_threads();    /* start collector thread for every Collector resource */

   /* Keep track of the important events */
   events_send_msg(NULL, "FD0001",
                   EVENTS_TYPE_DAEMON, "*Daemon*",
                   (intptr_t)get_first_port_host_order(me->FDaddrs), "Filed startup");

   /* Load the GPFS library if installed */
   GPFSLIB::Init();

   server_tid = pthread_self();

   /* Become server, and handle requests */
   IPADDR *p;
   foreach_dlist(p, me->FDaddrs) {
      Dmsg1(10, "filed: listening on port %d\n", p->get_port_host_order());
   }
   fdcallsdir_start_server(10, handle_connection_request);
   bnet_thread_server(me->FDaddrs, me->MaxConcurrentJobs, &dir_workq,
      handle_connection_request);

   terminate_filed(0);
   exit(0);                           /* should never get here */
}

void terminate_filed(int sig)
{
   static bool already_here = false;

   if (already_here) {
      bmicrosleep(2, 0);              /* yield */
      exit(1);                        /* prevent loops */
   }
   already_here = true;
   debug_level = 0;                   /* turn off debug */
   fdcallsdir_stop_server();
   stop_watchdog();
   terminate_collector_threads();

   bnet_stop_thread_server(server_tid);
   generate_daemon_event(NULL, "Exit");
   unload_plugins();

   free_daemon_message_queue();

   if (!test_config) {
      write_state_file(me->working_directory,
                       "bacula-fd", get_first_port_host_order(me->FDaddrs));
      if (make_pid_file) {
         delete_pid_file(me->pid_directory,
                         "bacula-fd", get_first_port_host_order(me->FDaddrs));
      }
      /* Keep track of the important events */
      events_send_msg(NULL, "FD0002", EVENTS_TYPE_DAEMON,
                      "*Daemon*", (intptr_t)get_first_port_host_order(me->FDaddrs), "Filed shutdown");
   }

   if (configfile != NULL) {
      free(configfile);
   }

   if (debug_level > 0) {
      print_memory_pool_stats();
   }

   if (config) {
      delete config;
      config = NULL;
   }
   if (statcollector){
      // statcollector->dump();
      delete(statcollector);
   }
   term_msg();
   cleanup_crypto();
   free(res_head);
   res_head = NULL;
   close_memory_pool();               /* release free memory in pool */
   lmgr_cleanup_main();
   sm_dump(false);                    /* dump orphaned buffers */
   exit(sig);
}

/*
* Make a quick check to see that we have all the
* resources needed.
*/
static bool check_resources()
{
   int i;
   bool found;
   char *cmd;
   bool OK = true;
   DIRRES *director;
   bool need_tls;

   LockRes();

   me = (CLIENT *)GetNextRes(R_CLIENT, NULL);
   if (!me) {
      Emsg1(M_FATAL, 0, _("No File daemon resource defined in %s\n"
            "Without that I don't know who I am :-(\n"), configfile);
      OK = false;
   } else {
      if (GetNextRes(R_CLIENT, (RES *) me) != NULL) {
         Emsg1(M_FATAL, 0, _("Only one Client resource permitted in %s\n"),
              configfile);
         OK = false;
      }
      my_name_is(0, NULL, me->hdr.name);
      if (!me->messages) {
         me->messages = (MSGS *)GetNextRes(R_MSGS, NULL);
         if (!me->messages) {
             Emsg1(M_FATAL, 0, _("No Messages resource defined in %s\n"), configfile);
             OK = false;
         }
      }

      /* Construct disabled command array */
      for (i=0; cmds[i].cmd; i++) { }  /* Count commands */
      if (me->disable_cmds) {
         me->disabled_cmds_array = (bool *)malloc(i);
         memset(me->disabled_cmds_array, 0, i);
         foreach_alist(cmd, me->disable_cmds) {
            found = false;
            for (i=0; cmds[i].cmd; i++) {
               if (strncasecmp(cmds[i].cmd, cmd, strlen(cmd)) == 0) {
                  me->disabled_cmds_array[i] = true;
                  found = true;
                  break;
               }
            }
            if (!found) {
               Jmsg(NULL, M_FATAL, 0, _("Disable Command \"%s\" not found.\n"),
                  cmd);
               OK = false;
            }
         }
      }
#ifdef xxxDEBUG
      for (i=0; cmds[i].cmd; i++) { }  /* Count commands */
      while (i-- >= 0) {
         if (me->disabled_cmds_array[i]) {
            Dmsg1(050, "Command: %s disabled.\n", cmds[i].cmd);
         }
      }
#endif

      /* tls_require implies tls_enable */
      if (me->tls_require) {
#ifndef HAVE_TLS
         Jmsg(NULL, M_FATAL, 0, _("TLS required but not configured in Bacula.\n"));
         OK = false;
#else
         if (me->tls_ca_certfile || me->tls_ca_certdir || me->tls_certfile || me->tls_keyfile) {
            me->tls_enable = true;
         }
#endif
      }
      need_tls = me->tls_enable || me->tls_authenticate;

      if ((!me->tls_ca_certfile && !me->tls_ca_certdir) && need_tls) {
         Emsg1(M_FATAL, 0, _("Neither \"TLS CA Certificate\""
            " or \"TLS CA Certificate Dir\" are defined for File daemon in %s.\n"),
                            configfile);
        OK = false;
      }

      /* If everything is well, attempt to initialize our per-resource TLS context */
      if (OK && need_tls) {
         /* Initialize TLS context:
          * Args: CA certfile, CA certdir, Certfile, Keyfile,
          * Keyfile PEM Callback, Keyfile CB Userdata, DHfile, Verify Peer */
         me->tls_ctx = new_tls_context(me->tls_ca_certfile,
            me->tls_ca_certdir, me->tls_certfile, me->tls_keyfile,
            NULL, NULL, NULL, true);

         if (!me->tls_ctx) {
            Emsg2(M_FATAL, 0, _("Failed to initialize TLS context for File daemon \"%s\" in %s.\n"),
                                me->hdr.name, configfile);
            OK = false;
         }
      }
      me->psk_ctx = new_psk_context(NULL); /* the shared key is generated by the DIR */

      /* In this case, we have TLS Require=Yes and TLS not setup and no PSK */
      if (OK && need_tls == false && me->tls_require) {
         if (me->psk_ctx == NULL) {
            Emsg2(M_FATAL, 0, _("Failed to initialize TLS PSK context for File daemon \"%s\" in %s.\n"),
                  me->hdr.name, configfile);
            OK = false;
         }
      }

      if (me->pki_encrypt || me->pki_sign) {
#ifndef HAVE_CRYPTO
         Jmsg(NULL, M_FATAL, 0, _("PKI encryption/signing enabled but not compiled into Bacula.\n"));
         OK = false;
#endif
      }

      /* pki_encrypt implies pki_sign */
      if (me->pki_encrypt) {
         me->pki_sign = true;
      }

      if ((me->pki_encrypt || me->pki_sign) && !me->pki_keypair_file) {
         Emsg2(M_FATAL, 0, _("\"PKI Key Pair\" must be defined for File"
            " daemon \"%s\" in %s if either \"PKI Sign\" or"
            " \"PKI Encrypt\" are enabled.\n"), me->hdr.name, configfile);
         OK = false;
      }

      /* If everything is well, attempt to initialize our public/private keys */
      if (OK && (me->pki_encrypt || me->pki_sign)) {
         char *filepath;
         /* Load our keypair */
         me->pki_keypair = crypto_keypair_new();
         if (!me->pki_keypair) {
            Emsg0(M_FATAL, 0, _("Failed to allocate a new keypair object.\n"));
            OK = false;
         } else {
            if (!crypto_keypair_load_cert(me->pki_keypair, me->pki_keypair_file)) {
               Emsg2(M_FATAL, 0, _("Failed to load public certificate for File"
                     " daemon \"%s\" in %s.\n"), me->hdr.name, configfile);
               OK = false;
            }

            if (!crypto_keypair_load_key(me->pki_keypair, me->pki_keypair_file, NULL, NULL)) {
               Emsg2(M_FATAL, 0, _("Failed to load private key for File"
                     " daemon \"%s\" in %s.\n"), me->hdr.name, configfile);
               OK = false;
            }
         }

         /*
          * Trusted Signers. We're always trusted.
          */
         me->pki_signers = New(alist(10, not_owned_by_alist));
         if (me->pki_keypair) {
            me->pki_signers->append(crypto_keypair_dup(me->pki_keypair));
         }

         /* If additional signing public keys have been specified, load them up */
         if (me->pki_signing_key_files) {
            foreach_alist(filepath, me->pki_signing_key_files) {
               X509_KEYPAIR *keypair;

               keypair = crypto_keypair_new();
               if (!keypair) {
                  Emsg0(M_FATAL, 0, _("Failed to allocate a new keypair object.\n"));
                  OK = false;
               } else {
                  if (crypto_keypair_load_cert(keypair, filepath)) {
                     me->pki_signers->append(keypair);

                     /* Attempt to load a private key, if available */
                     if (crypto_keypair_has_key(filepath)) {
                        if (!crypto_keypair_load_key(keypair, filepath, NULL, NULL)) {
                           Emsg3(M_FATAL, 0, _("Failed to load private key from file %s for File"
                              " daemon \"%s\" in %s.\n"), filepath, me->hdr.name, configfile);
                           OK = false;
                        }
                     }

                  } else {
                     Emsg3(M_FATAL, 0, _("Failed to load trusted signer certificate"
                        " from file %s for File daemon \"%s\" in %s.\n"), filepath, me->hdr.name, configfile);
                     OK = false;
                  }
               }
            }
         }

         /*
          * Crypto recipients. We're always included as a recipient.
          * The symmetric session key will be encrypted for each of these readers.
          */
         me->pki_recipients = New(alist(10, not_owned_by_alist));
         if (me->pki_keypair) {
            me->pki_recipients->append(crypto_keypair_dup(me->pki_keypair));
         }

         /* Put a default cipher (not possible in the filed_conf.c structure */
         if (!me->pki_cipher) {
            me->pki_cipher = CRYPTO_CIPHER_AES_128_CBC;
         }

         /* Put a default digest (not possible in the filed_conf.c structure */
         if (!me->pki_digest) {
            me->pki_digest = CRYPTO_DIGEST_DEFAULT;
         }

         /* If additional keys have been specified, load them up */
         if (me->pki_master_key_files) {
            foreach_alist(filepath, me->pki_master_key_files) {
               X509_KEYPAIR *keypair;

               keypair = crypto_keypair_new();
               if (!keypair) {
                  Emsg0(M_FATAL, 0, _("Failed to allocate a new keypair object.\n"));
                  OK = false;
               } else {
                  if (crypto_keypair_load_cert(keypair, filepath)) {
                     me->pki_recipients->append(keypair);
                  } else {
                     Emsg3(M_FATAL, 0, _("Failed to load master key certificate"
                        " from file %s for File daemon \"%s\" in %s.\n"), filepath, me->hdr.name, configfile);
                     OK = false;
                  }
               }
            }
         }
      }
   }


   /* Verify that a director record exists */
   LockRes();
   director = (DIRRES *)GetNextRes(R_DIRECTOR, NULL);
   UnlockRes();
   if (!director) {
      Emsg1(M_FATAL, 0, _("No Director resource defined in %s\n"),
            configfile);
      OK = false;
   }

   foreach_res(director, R_DIRECTOR) {

      /* Construct disabled command array */
      for (i=0; cmds[i].cmd; i++) { }  /* Count commands */
      if (director->disable_cmds) {
         director->disabled_cmds_array = (bool *)malloc(i);
         memset(director->disabled_cmds_array, 0, i);
         foreach_alist(cmd, director->disable_cmds) {
            found = false;
            for (i=0; cmds[i].cmd; i++) {
               if (strncasecmp(cmds[i].cmd, cmd, strlen(cmd)) == 0) {
                  director->disabled_cmds_array[i] = true;
                  found = true;
                  break;
               }
            }
            if (!found) {
               Jmsg(NULL, M_FATAL, 0, _("Disable Command \"%s\" not found.\n"),
                  cmd);
               OK = false;
            }
         }
      }

#ifdef xxxDEBUG
      for (i=0; cmds[i].cmd; i++) { }  /* Count commands */
      while (i-- >= 0) {
         if (director->disabled_cmds_array[i]) {
            Dmsg1(050, "Command: %s disabled for Director.\n", cmds[i].cmd);
         }
      }
#endif

      /* tls_require implies tls_enable */
      if (director->dirinfo.tls_require) {
#ifndef HAVE_TLS
         Jmsg(NULL, M_FATAL, 0, _("TLS required but not configured in Bacula.\n"));
         OK = false;
         continue;
#else
         if (director->dirinfo.tls_certfile || director->dirinfo.tls_keyfile) {
            director->dirinfo.tls_enable = true;
         }
#endif
      }
      need_tls = director->dirinfo.tls_enable || director->dirinfo.tls_authenticate;

      if (!director->dirinfo.tls_certfile && need_tls) {
         Emsg2(M_FATAL, 0, _("\"TLS Certificate\" file not defined for Director \"%s\" in %s.\n"),
               director->hdr.name, configfile);
         OK = false;
      }

      if (!director->dirinfo.tls_keyfile && need_tls) {
         Emsg2(M_FATAL, 0, _("\"TLS Key\" file not defined for Director \"%s\" in %s.\n"),
               director->hdr.name, configfile);
         OK = false;
      }

      if ((!director->dirinfo.tls_ca_certfile && !director->dirinfo.tls_ca_certdir) && need_tls && director->dirinfo.tls_verify_peer) {
         Emsg2(M_FATAL, 0, _("Neither \"TLS CA Certificate\""
                             " or \"TLS CA Certificate Dir\" are defined for Director \"%s\" in %s."
                             " At least one CA certificate store is required"
                             " when using \"TLS Verify Peer\".\n"),
                             director->hdr.name, configfile);
         OK = false;
      }

      /* If everything is well, attempt to initialize our per-resource TLS context */
      if (OK && need_tls) {
         /* Initialize TLS context:
          * Args: CA certfile, CA certdir, Certfile, Keyfile,
          * Keyfile PEM Callback, Keyfile CB Userdata, DHfile, Verify Peer */
         director->dirinfo.tls_ctx = new_tls_context(director->dirinfo.tls_ca_certfile,
            director->dirinfo.tls_ca_certdir, director->dirinfo.tls_certfile,
            director->dirinfo.tls_keyfile, NULL, NULL, director->dirinfo.tls_dhfile,
            director->dirinfo.tls_verify_peer);

         if (!director->dirinfo.tls_ctx) {
            Emsg2(M_FATAL, 0, _("Failed to initialize TLS context for Director \"%s\" in %s.\n"),
                                director->hdr.name, configfile);
            OK = false;
         }
      }
      director->dirinfo.psk_ctx = new_psk_context(director->dirinfo.password);

      /* In this case, we have TLS Require=Yes and TLS not setup and no PSK */
      if (OK && need_tls == false && director->dirinfo.tls_require) {
         if (director->dirinfo.psk_ctx == NULL) {
            Emsg2(M_FATAL, 0, _("Failed to initialize TLS PSK context for Director \"%s\" in %s.\n"),
                  director->hdr.name, configfile);
            OK = false;
         }
      }
   }

   CONSRES *console;
   foreach_res(console, R_CONSOLE) {
      /* tls_require implies tls_enable */
      if (console->dirinfo.tls_require) {
#ifndef HAVE_TLS
         Jmsg(NULL, M_FATAL, 0, _("TLS required but not configured in Bacula.\n"));
         OK = false;
         continue;
#else
         if (console->dirinfo.tls_certfile || console->dirinfo.tls_keyfile) {
            console->dirinfo.tls_enable = true;
         }
#endif
      }
      need_tls = console->dirinfo.tls_enable || console->dirinfo.tls_authenticate;

      if (!console->dirinfo.tls_certfile && need_tls) {
         Emsg2(M_FATAL, 0, _("\"TLS Certificate\" file not defined for Console \"%s\" in %s.\n"),
               console->hdr.name, configfile);
         OK = false;
      }

      if (!console->dirinfo.tls_keyfile && need_tls) {
         Emsg2(M_FATAL, 0, _("\"TLS Key\" file not defined for Console \"%s\" in %s.\n"),
               console->hdr.name, configfile);
         OK = false;
      }

      if ((!console->dirinfo.tls_ca_certfile && !console->dirinfo.tls_ca_certdir) && need_tls && console->dirinfo.tls_verify_peer) {
         Emsg2(M_FATAL, 0, _("Neither \"TLS CA Certificate\""
                             " or \"TLS CA Certificate Dir\" are defined for Console \"%s\" in %s."
                             " At least one CA certificate store is required"
                             " when using \"TLS Verify Peer\".\n"),
                             console->hdr.name, configfile);
         OK = false;
      }

      /* If everything is well, attempt to initialize our per-resource TLS context */
      if (OK && need_tls) {
         /* Initialize TLS context:
          * Args: CA certfile, CA certdir, Certfile, Keyfile,
          * Keyfile PEM Callback, Keyfile CB Userdata, DHfile, Verify Peer */
         console->dirinfo.tls_ctx = new_tls_context(console->dirinfo.tls_ca_certfile,
            console->dirinfo.tls_ca_certdir, console->dirinfo.tls_certfile,
            console->dirinfo.tls_keyfile, NULL, NULL, console->dirinfo.tls_dhfile,
            console->dirinfo.tls_verify_peer);

         if (!console->dirinfo.tls_ctx) {
            Emsg2(M_FATAL, 0, _("Failed to initialize TLS context for Console \"%s\" in %s.\n"),
                                console->hdr.name, configfile);
            OK = false;
         }
      }
      console->dirinfo.psk_ctx = new_psk_context(console->dirinfo.password);

      /* In this case, we have TLS Require=Yes and TLS not setup and no PSK */
      if (OK && (need_tls == false && console->dirinfo.tls_require)) {
         if (console->dirinfo.psk_ctx == NULL) {
            Emsg2(M_FATAL, 0, _("Failed to initialize TLS PSK context for Console \"%s\" in %s.\n"),
                  console->hdr.name, configfile);
            OK = false;
         }
      }
   }

   UnlockRes();

   if (OK) {
      close_msg(NULL);                /* close temp message handler */
      init_msg(NULL, me->messages);   /* open user specified message handler */
   }

   return OK;
}
