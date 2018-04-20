/*
   Bacula(R) - The Network Backup Solution

   Copyright (C) 2000-2018 Kern Sibbald

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
 * Network Utility Routines
 *
 *  Written by Kern Sibbald
 */

#include "bacula.h"
#include "jcr.h"
#include "lz4.h"
#include <netdb.h>
#include <netinet/tcp.h>

#if !defined(ENODATA)              /* not defined on BSD systems */
#define ENODATA  EPIPE
#endif 
 
#if !defined(SOL_TCP)              /* Not defined on some systems */
#define SOL_TCP  IPPROTO_TCP
#endif 

#ifdef HAVE_WIN32
#include <mswsock.h>
#define socketRead(fd, buf, len)  ::recv(fd, buf, len, 0)
#define socketWrite(fd, buf, len) ::send(fd, buf, len, 0)
#define socketClose(fd)           ::closesocket(fd)
static void win_close_wait(int fd);
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif
#else
#define socketRead(fd, buf, len)  ::read(fd, buf, len)
#define socketWrite(fd, buf, len) ::write(fd, buf, len)
#define socketClose(fd)           ::close(fd)
#endif


/*
 * make a nice dump of a message
 */
void dump_bsock_msg(int sock, uint32_t msgno, const char *what, uint32_t rc, int32_t pktsize, uint32_t flags, POOLMEM *msg, int32_t msglen)
{
   char buf[54];
   bool is_ascii;
   int dbglvl = DT_ASX;

   if (msglen<0) {
      Dmsg4(dbglvl, "%s %d:%d SIGNAL=%s\n", what, sock, msgno, bnet_sig_to_ascii(msglen));
      // data
      smartdump(msg, msglen, buf, sizeof(buf)-9, &is_ascii);
      if (is_ascii) {
         Dmsg5(dbglvl, "%s %d:%d len=%d \"%s\"\n", what, sock, msgno, msglen, buf);
      } else {
         Dmsg5(dbglvl, "%s %d:%d len=%d %s\n", what, sock, msgno, msglen, buf);
      }
   }
}


BSOCKCallback::BSOCKCallback()
{
}

BSOCKCallback::~BSOCKCallback()
{
}


/*
 * This is a non-class BSOCK "constructor"  because we want to
 *   call the Bacula smartalloc routines instead of new.
 */
BSOCK *new_bsock()
{
   BSOCK *bsock = (BSOCK *)malloc(sizeof(BSOCK));
   bsock->init();
   return bsock;
}

void BSOCK::init()
{
   memset(this, 0, sizeof(BSOCK));
   m_master = this;
   set_closed();
   set_terminated();
   m_blocking = 1;
   pout_msg_no = &out_msg_no;
   uninstall_send_hook_cb();
   msg = get_pool_memory(PM_BSOCK);
   cmsg = get_pool_memory(PM_BSOCK);
   errmsg = get_pool_memory(PM_MESSAGE);
   timeout = BSOCK_TIMEOUT;
}

void BSOCK::free_tls()
{
   free_tls_connection(this->tls);
   this->tls = NULL;
}

/*
 * Try to connect to host for max_retry_time at retry_time intervals.
 *   Note, you must have called the constructor prior to calling
 *   this routine.
 */
bool BSOCK::connect(JCR * jcr, int retry_interval, utime_t max_retry_time,
                    utime_t heart_beat,
                    const char *name, char *host, char *service, int port,
                    int verbose)
{
   bool ok = false;
   int i;
   int fatal = 0;
   time_t begin_time = time(NULL);
   time_t now;
   btimer_t *tid = NULL;

   /* Try to trap out of OS call when time expires */
   if (max_retry_time) {
      tid = start_thread_timer(jcr, pthread_self(), (uint32_t)max_retry_time);
   }

   for (i = 0; !open(jcr, name, host, service, port, heart_beat, &fatal);
        i -= retry_interval) {
      berrno be;
      if (fatal || (jcr && job_canceled(jcr))) {
         goto bail_out;
      }
      Dmsg4(50, "Unable to connect to %s on %s:%d. ERR=%s\n",
            name, host, port, be.bstrerror());
      if (i < 0) {
         i = 60 * 5;               /* complain again in 5 minutes */
         if (verbose)
            Qmsg4(jcr, M_WARNING, 0, _(
               "Could not connect to %s on %s:%d. ERR=%s\n"
               "Retrying ...\n"), name, host, port, be.bstrerror());
      }
      bmicrosleep(retry_interval, 0);
      now = time(NULL);
      if (begin_time + max_retry_time <= now) {
         Qmsg4(jcr, M_FATAL, 0, _("Unable to connect to %s on %s:%d. ERR=%s\n"),
               name, host, port, be.bstrerror());
         goto bail_out;
      }
   }
   ok = true;

bail_out:
   if (tid) {
      stop_thread_timer(tid);
   }
   return ok;
}

/*
 * Finish initialization of the packet structure.
 */
void BSOCK::fin_init(JCR * jcr, int sockfd, const char *who, const char *host, int port,
               struct sockaddr *lclient_addr)
{
   Dmsg3(100, "who=%s host=%s port=%d\n", who, host, port);
   m_fd = sockfd;
   if (m_who) {
      free(m_who);
   }
   if (m_host) {
      free(m_host);
   }
   set_who(bstrdup(who));
   set_host(bstrdup(host));
   set_port(port);
   memcpy(&client_addr, lclient_addr, sizeof(client_addr));
   set_jcr(jcr);
}

/*
 * Copy the address from the configuration dlist that gets passed in
 */
void BSOCK::set_source_address(dlist *src_addr_list)
{
   IPADDR *addr = NULL;

   // delete the object we already have, if it's allocated
   if (src_addr) {
     free( src_addr);
     src_addr = NULL;
   }

   if (src_addr_list) {
     addr = (IPADDR*) src_addr_list->first();
     src_addr = New( IPADDR(*addr));
   }
}

/*
 * Open a TCP connection to the server
 * Returns NULL
 * Returns BSOCK * pointer on success
 */
bool BSOCK::open(JCR *jcr, const char *name, char *host, char *service,
               int port, utime_t heart_beat, int *fatal)
{
   int sockfd = -1;
   dlist *addr_list;
   IPADDR *ipaddr;
   bool connected = false;
   int turnon = 1;
   const char *errstr;
   int save_errno = 0;

   /*
    * Fill in the structure serv_addr with the address of
    * the server that we want to connect with.
    */
   if ((addr_list = bnet_host2ipaddrs(host, 0, &errstr)) == NULL) {
      /* Note errstr is not malloc'ed */
      Qmsg2(jcr, M_ERROR, 0, _("gethostbyname() for host \"%s\" failed: ERR=%s\n"),
            host, errstr);
      Dmsg2(100, "bnet_host2ipaddrs() for host %s failed: ERR=%s\n",
            host, errstr);
      *fatal = 1;
      return false;
   }

   remove_duplicate_addresses(addr_list);
   foreach_dlist(ipaddr, addr_list) {
      ipaddr->set_port_net(htons(port));
      char allbuf[256 * 10];
      char curbuf[256];
      Dmsg2(100, "Current %sAll %s\n",
                   ipaddr->build_address_str(curbuf, sizeof(curbuf)),
                   build_addresses_str(addr_list, allbuf, sizeof(allbuf)));
      /* Open a TCP socket */
      if ((sockfd = socket(ipaddr->get_family(), SOCK_STREAM|SOCK_CLOEXEC, 0)) < 0) {
         berrno be;
         save_errno = errno;
         switch (errno) {
#ifdef EAFNOSUPPORT
         case EAFNOSUPPORT:
            /*
             * The name lookup of the host returned an address in a protocol family
             * we don't support. Suppress the error and try the next address.
             */
            break;
#endif
#ifdef EPROTONOSUPPORT
         /* See above comments */
         case EPROTONOSUPPORT:
            break;
#endif
#ifdef EPROTOTYPE
         /* See above comments */
         case EPROTOTYPE:
            break;
#endif
         default:
            *fatal = 1;
            Qmsg3(jcr, M_ERROR, 0,  _("Socket open error. proto=%d port=%d. ERR=%s\n"),
               ipaddr->get_family(), ipaddr->get_port_host_order(), be.bstrerror());
            Pmsg3(300, _("Socket open error. proto=%d port=%d. ERR=%s\n"),
               ipaddr->get_family(), ipaddr->get_port_host_order(), be.bstrerror());
            break;
         }
         continue;
      }

      /* Bind to the source address if it is set */
      if (src_addr) {
         if (bind(sockfd, src_addr->get_sockaddr(), src_addr->get_sockaddr_len()) < 0) {
            berrno be;
            save_errno = errno;
            *fatal = 1;
            Qmsg2(jcr, M_ERROR, 0, _("Source address bind error. proto=%d. ERR=%s\n"),
                  src_addr->get_family(), be.bstrerror() );
            Pmsg2(000, _("Source address bind error. proto=%d. ERR=%s\n"),
                  src_addr->get_family(), be.bstrerror() );
            if (sockfd >= 0) socketClose(sockfd);
            continue;
         }
      }

      /*
       * Keep socket from timing out from inactivity
       */
      if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (sockopt_val_t)&turnon, sizeof(turnon)) < 0) {
         berrno be;
         Qmsg1(jcr, M_WARNING, 0, _("Cannot set SO_KEEPALIVE on socket: %s\n"),
               be.bstrerror());
      }
#if defined(TCP_KEEPIDLE)
      if (heart_beat) {
         int opt = heart_beat;
         if (setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (sockopt_val_t)&opt, sizeof(opt)) < 0) {
            berrno be;
            Qmsg1(jcr, M_WARNING, 0, _("Cannot set TCP_KEEPIDLE on socket: %s\n"),
                  be.bstrerror());
         }
      }
#endif

      /* connect to server */
      if (::connect(sockfd, ipaddr->get_sockaddr(), ipaddr->get_sockaddr_len()) < 0) {
         save_errno = errno;
         if (sockfd >= 0) socketClose(sockfd);
         continue;
      }
      *fatal = 0;
      connected = true;
      break;
   }

   if (!connected) {
      berrno be;
      free_addresses(addr_list);
      errno = save_errno | b_errno_win32;
      Dmsg4(50, "Could not connect to server %s %s:%d. ERR=%s\n",
            name, host, port, be.bstrerror());
      return false;
   }
   /*
    * Keep socket from timing out from inactivity
    *   Do this a second time out of paranoia
    */
   if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (sockopt_val_t)&turnon, sizeof(turnon)) < 0) {
      berrno be;
      Qmsg1(jcr, M_WARNING, 0, _("Cannot set SO_KEEPALIVE on socket: %s\n"),
            be.bstrerror());
   }
   fin_init(jcr, sockfd, name, host, port, ipaddr->get_sockaddr());
   free_addresses(addr_list);

   /* Clean the packet a bit */
   m_closed = false;
   m_duped = false;
   m_spool = false;
   m_use_locking = false;
   m_timed_out = false;
   m_terminated = false;
   m_suppress_error_msgs = false;
   errors = 0;
   m_blocking = 0;

   Dmsg3(50, "OK connected to server  %s %s:%d.\n",
         name, host, port);

   return true;
}

/*
 * Force read/write to use locking
 */
bool BSOCK::set_locking()
{
   int stat;
   if (m_use_locking) {
      return true;                      /* already set */
   }
   pm_rmutex = &m_rmutex;
   pm_wmutex = &m_wmutex;
   if ((stat = pthread_mutex_init(pm_rmutex, NULL)) != 0) {
      berrno be;
      Qmsg(m_jcr, M_FATAL, 0, _("Could not init bsock read mutex. ERR=%s\n"),
         be.bstrerror(stat));
      return false;
   }
   if ((stat = pthread_mutex_init(pm_wmutex, NULL)) != 0) {
      berrno be;
      Qmsg(m_jcr, M_FATAL, 0, _("Could not init bsock write mutex. ERR=%s\n"),
         be.bstrerror(stat));
      return false;
   }
   if ((stat = pthread_mutex_init(&m_mmutex, NULL)) != 0) {
      berrno be;
      Qmsg(m_jcr, M_FATAL, 0, _("Could not init bsock attribute mutex. ERR=%s\n"),
         be.bstrerror(stat));
      return false;
   }
   m_use_locking = true;
   return true;
}

void BSOCK::clear_locking()
{
   if (!m_use_locking || m_duped) {
      return;
   }
   m_use_locking = false;
   pthread_mutex_destroy(pm_rmutex);
   pthread_mutex_destroy(pm_wmutex);
   pthread_mutex_destroy(&m_mmutex);
   pm_rmutex = NULL;
   pm_wmutex = NULL;
   return;
}

/*
 * Do comm line compression (LZ4) of a bsock message.
 * Returns:  true if the compression was done
 *           false if no compression was done
 * The "offset" defines where to start compressing the message.  This
 *   allows passing "header" information uncompressed and the actual
 *   data part compressed.
 *
 * Note, we don't compress lines less than 20 characters because we
 *  want to save at least 10 characters otherwise compression doesn't
 *  help enough to warrant doing the decompression.
 */
bool BSOCK::comm_compress()
{
   bool compress = false;
   bool compressed = false;
   int offset = m_flags & 0xFF;

   /*
    * Enable compress if allowed and not spooling and the
    *  message is long enough (>20) to get some reasonable savings.
    */
   if (msglen > 20) {
      compress = can_compress() && !is_spooling();
   }
   m_CommBytes += msglen;                    /* uncompressed bytes */
   Dmsg4(DT_NETWORK|200, "can_compress=%d compress=%d CommBytes=%lld CommCompresedBytes=%lld\n",
         can_compress(), compress, m_CommBytes, m_CommCompressedBytes);
   if (compress) {
      int clen;
      int need_size;

      ASSERT2(offset <= msglen, "Comm offset bigger than message\n");
      ASSERT2(offset < 255, "Offset greater than 254\n");
      need_size = LZ4_compressBound(msglen);
      if (need_size >= ((int32_t)sizeof_pool_memory(cmsg))) {
         cmsg = realloc_pool_memory(cmsg, need_size + 100);
      }
      msglen -= offset;
      msg += offset;
      cmsg += offset;
      clen = LZ4_compress_default(msg, cmsg, msglen, msglen);
      //Dmsg2(000, "clen=%d msglen=%d\n", clen, msglen);
      /* Compression should save at least 10 characters */
      if (clen > 0 && clen + 10 <= msglen) {

#ifdef xxx_debug
         /* Debug code -- decompress and compare */
         int blen, rlen, olen;
         olen = msglen;
         POOLMEM *rmsg = get_pool_memory(PM_BSOCK);
         blen = sizeof_pool_memory(msg) * 2;
         if (blen >= sizeof_pool_memory(rmsg)) {
            rmsg = realloc_pool_memory(rmsg, blen);
         }
         rlen = LZ4_decompress_safe(cmsg, rmsg, clen, blen);
         //Dmsg4(000, "blen=%d clen=%d olen=%d rlen=%d\n", blen, clen, olen, rlen);
         ASSERT(olen == rlen);
         ASSERT(memcmp(msg, rmsg, olen) == 0);
         free_pool_memory(rmsg);
         /* end Debug code */
#endif

         msg = cmsg;
         msglen = clen;
         compressed = true;
      }
      msglen += offset;
      msg -= offset;
      cmsg -= offset;
   }
   m_CommCompressedBytes += msglen;
   return compressed;
}


/*
 * Send a message over the network. Everything is sent in one
 *   write request, but depending on the mode you are using
 *   there will be either two or three read requests done.
 * Read 1: 32 bits, gets essentially the packet length, but may
 *         have the upper bits set to indicate compression or
 *         an extended header packet.
 * Read 2: 32 bits, this read is done only of BNET_HDR_EXTEND is set.
 *         In this case the top 16 bits of this 32 bit word are reserved
 *         for flags and the lower 16 bits for data. This word will be
 *         stored in the field "flags" in the BSOCK packet.
 * Read 2 or 3: depending on if Read 2 is done. This is the data.
 *
 * For normal comm line compression, the whole data packet is compressed
 *   but not the msglen (first read).
 * To do data compression rather than full comm line compression, prior to
 *   call send(flags) where the lower 32 bits is the offset to the data to
 *   be compressed.  The top 32 bits are reserved for flags that can be
 *   set. The are:
 *     BNET_IS_CMD   We are sending a command
 *     BNET_OFFSET   An offset is specified (this implies data compression)
 *     BNET_NOCOMPRESS Inhibit normal comm line compression
 *     BNET_DATACOMPRESSED The data using the specified offset was
 *                   compressed, and normal comm line compression will
 *                   not be done.
 *   If any of the above bits are set, then BNET_HDR_EXTEND will be set
 *   in the top bits of msglen, and the full set of flags + the offset
 *   will be passed as a 32 bit word just after the msglen, and then
 *   followed by any data that is either compressed or not.
 *
 *   Note, neither comm line nor data compression is not
 *   guaranteed since it may result in more data, in which case, the
 *   record is sent uncompressed and there will be no offset.
 *   On the receive side, if BNET_OFFSET is set, then the data is compressed.
 *
 * Returns: false on failure
 *          true  on success
 */
bool BSOCK::send(int aflags)
{
   int32_t rc;
   int32_t pktsiz;
   int32_t *hdrptr;
   int offset;
   int hdrsiz;
   bool ok = true;
   int32_t save_msglen;
   POOLMEM *save_msg;
   bool compressed;
   bool locked = false;

   if (is_closed()) {
      if (!m_suppress_error_msgs) {
         Qmsg0(m_jcr, M_ERROR, 0,  _("Socket is closed\n"));
      }
      return false;
   }
   if (errors) {
      if (!m_suppress_error_msgs) {
         Qmsg4(m_jcr, M_ERROR, 0,  _("Socket has errors=%d on call to %s:%s:%d\n"),
             errors, m_who, m_host, m_port);
      }
      return false;
   }
   if (is_terminated()) {
      if (!m_suppress_error_msgs) {
         Qmsg4(m_jcr, M_ERROR, 0,  _("Bsock send while terminated=%d on call to %s:%s:%d\n"),
             is_terminated(), m_who, m_host, m_port);
      }
      return false;
   }

   if (msglen > 4000000) {
      if (!m_suppress_error_msgs) {
         Qmsg4(m_jcr, M_ERROR, 0,
            _("Socket has insane msglen=%d on call to %s:%s:%d\n"),
             msglen, m_who, m_host, m_port);
      }
      return false;
   }

   if (send_hook_cb) {
      if (!send_hook_cb->bsock_send_cb()) {
         Dmsg3(1, "Flowcontrol failure on %s:%s:%d\n", m_who, m_host, m_port);
         Qmsg3(m_jcr, M_ERROR, 0,
            _("Flowcontrol failure on %s:%s:%d\n"),
                  m_who, m_host, m_port);
         return false;
      }
   }
   if (m_use_locking) {
      pP(pm_wmutex);
      locked = true;
   }
   save_msglen = msglen;
   save_msg = msg;
   m_flags = aflags;

   offset = aflags & 0xFF;              /* offset is 16 bits */
   if (offset) {
      m_flags |= BNET_OFFSET;
   }
   if (m_flags & BNET_DATACOMPRESSED) {   /* Check if already compressed */
      compressed = true;
   } else if (m_flags & BNET_NOCOMPRESS) {
      compressed = false;
   } else {
      compressed = comm_compress();       /* do requested compression */
   }
   if (offset && compressed) {
      m_flags |= BNET_DATACOMPRESSED;
   }
   if (!compressed) {
      m_flags &= ~BNET_COMPRESSED;
   }

   /* Compute total packet length */
   if (msglen <= 0) {
      hdrsiz = sizeof(pktsiz);
      pktsiz = hdrsiz;                     /* signal, no data */
   } else if (m_flags) {
      hdrsiz = 2 * sizeof(pktsiz);         /* have 64 bit header */
      pktsiz = msglen + hdrsiz;
   } else {
      hdrsiz = sizeof(pktsiz);             /* have 32 bit header */
      pktsiz = msglen + hdrsiz;
   }

   /* Set special bits */
   if (m_flags & BNET_OFFSET) {            /* if data compression on */
      compressed = false;                  /*   no comm compression */
   }
   if (compressed) {
      msglen |= BNET_COMPRESSED;           /* comm line compression */
   }

   if (m_flags) {
      msglen |= BNET_HDR_EXTEND;           /* extended header */
   }

   /*
    * Store packet length at head of message -- note, we
    *  have reserved an int32_t just before msg, so we can
    *  store there
    */
   hdrptr = (int32_t *)(msg - hdrsiz);
   *hdrptr = htonl(msglen);             /* store signal/length */
   if (m_flags) {
      *(hdrptr+1) = htonl(m_flags);     /* store flags */
   }

   (*pout_msg_no)++;        /* increment message number */

   /* send data packet */
   timer_start = watchdog_time;  /* start timer */
   clear_timed_out();
   /* Full I/O done in one write */
   rc = write_nbytes(this, (char *)hdrptr, pktsiz);
   if (chk_dbglvl(DT_NETWORK|1900)) dump_bsock_msg(m_fd, *pout_msg_no, "SEND", rc, msglen, m_flags, save_msg, save_msglen);
   timer_start = 0;         /* clear timer */
   if (rc != pktsiz) {
      errors++;
      if (errno == 0) {
         b_errno = EIO;
      } else {
         b_errno = errno;
      }
      if (rc < 0) {
         if (!m_suppress_error_msgs) {
            Qmsg5(m_jcr, M_ERROR, 0,
                  _("Write error sending %d bytes to %s:%s:%d: ERR=%s\n"),
                  pktsiz, m_who,
                  m_host, m_port, this->bstrerror());
         }
      } else {
         Qmsg5(m_jcr, M_ERROR, 0,
               _("Wrote %d bytes to %s:%s:%d, but only %d accepted.\n"),
               pktsiz, m_who, m_host, m_port, rc);
      }
      ok = false;
   }
//   Dmsg4(000, "cmpr=%d ext=%d cmd=%d m_flags=0x%x\n", msglen&BNET_COMPRESSED?1:0,
//      msglen&BNET_HDR_EXTEND?1:0, msglen&BNET_CMD_BIT?1:0, m_flags);
   msglen = save_msglen;
   msg = save_msg;
   if (locked) pV(pm_wmutex);
   return ok;
}

/*
 * Format and send a message
 *  Returns: false on error
 *           true  on success
 */
bool BSOCK::fsend(const char *fmt, ...)
{
   va_list arg_ptr;
   int maxlen;

   if (is_null(this)) {
      return false;                /* do not seg fault */
   }
   if (errors || is_terminated() || is_closed()) {
      return false;
   }
   /* This probably won't work, but we vsnprintf, then if we
    * get a negative length or a length greater than our buffer
    * (depending on which library is used), the printf was truncated, so
    * get a bigger buffer and try again.
    */
   for (;;) {
      maxlen = sizeof_pool_memory(msg) - 1;
      va_start(arg_ptr, fmt);
      msglen = bvsnprintf(msg, maxlen, fmt, arg_ptr);
      va_end(arg_ptr);
      if (msglen >= 0 && msglen < (maxlen - 5)) {
         break;
      }
      msg = realloc_pool_memory(msg, maxlen + maxlen / 2);
   }
   return send();
}

/*
 * Receive a message from the other end. Each message consists of
 * two packets. The first is a header that contains the size
 * of the data that follows in the second packet.
 * Returns number of bytes read (may return zero)
 * Returns -1 on signal (BNET_SIGNAL)
 * Returns -2 on hard end of file (BNET_HARDEOF)
 * Returns -3 on error  (BNET_ERROR)
 * Returns -4 on COMMAND (BNET_COMMAND)
 *  Unfortunately, it is a bit complicated because we have these
 *    four return types:
 *    1. Normal data
 *    2. Signal including end of data stream
 *    3. Hard end of file
 *    4. Error
 *  Using bsock->is_stop() and bsock->is_error() you can figure this all out.
 */
int32_t BSOCK::recv()
{
   int32_t nbytes;
   int32_t pktsiz;
   int32_t o_pktsiz = 0;
   bool compressed = false;
   bool command = false;
   bool locked = false;

   cmsg[0] = msg[0] = 0;
   msglen = 0;
   m_flags = 0;
   if (errors || is_terminated() || is_closed()) {
      return BNET_HARDEOF;
   }
   if (m_use_locking) {
      pP(pm_rmutex);
      locked = true;
   }

   read_seqno++;            /* bump sequence number */
   timer_start = watchdog_time;  /* set start wait time */
   clear_timed_out();
   /* get data size -- in int32_t */
   if ((nbytes = read_nbytes(this, (char *)&pktsiz, sizeof(int32_t))) <= 0) {
      timer_start = 0;      /* clear timer */
      /* probably pipe broken because client died */
      if (errno == 0) {
         b_errno = ENODATA;
      } else {
         b_errno = errno;
      }
      errors++;
      nbytes = BNET_HARDEOF;        /* assume hard EOF received */
      goto get_out;
   }
   timer_start = 0;         /* clear timer */
   if (nbytes != sizeof(int32_t)) {
      errors++;
      b_errno = EIO;
      Qmsg5(m_jcr, M_ERROR, 0, _("Read expected %d got %d from %s:%s:%d\n"),
            sizeof(int32_t), nbytes, m_who, m_host, m_port);
      nbytes = BNET_ERROR;
      goto get_out;
   }

   pktsiz = ntohl(pktsiz);         /* decode no. of bytes that follow */
   o_pktsiz = pktsiz;
   /* If extension, read it */
   if (pktsiz > 0 && (pktsiz & BNET_HDR_EXTEND)) {
      timer_start = watchdog_time;  /* set start wait time */
      clear_timed_out();
      if ((nbytes = read_nbytes(this, (char *)&m_flags, sizeof(int32_t))) <= 0) {
         timer_start = 0;      /* clear timer */
         /* probably pipe broken because client died */
         if (errno == 0) {
            b_errno = ENODATA;
         } else {
            b_errno = errno;
         }
         errors++;
         nbytes = BNET_HARDEOF;        /* assume hard EOF received */
         goto get_out;
      }
      timer_start = 0;         /* clear timer */
      if (nbytes != sizeof(int32_t)) {
         errors++;
         b_errno = EIO;
         Qmsg5(m_jcr, M_ERROR, 0, _("Read expected %d got %d from %s:%s:%d\n"),
               sizeof(int32_t), nbytes, m_who, m_host, m_port);
         nbytes = BNET_ERROR;
         goto get_out;
      }
      pktsiz &= ~BNET_HDR_EXTEND;
      m_flags = ntohl(m_flags);
   }

   if (pktsiz > 0 && (pktsiz & BNET_COMPRESSED)) {
      compressed = true;
      pktsiz &= ~BNET_COMPRESSED;
   }

   if (m_flags & BNET_IS_CMD) {
       command = true;
   }
   if (m_flags & BNET_OFFSET) {
      compressed = true;
   }

   if (pktsiz == 0) {              /* No data transferred */
      timer_start = 0;             /* clear timer */
      in_msg_no++;
      msglen = 0;
      nbytes = 0;                  /* zero bytes read */
      goto get_out;
   }

   /* If signal or packet size too big */
   if (pktsiz < 0 || pktsiz > 1000000) {
      if (pktsiz > 0) {            /* if packet too big */
         Qmsg4(m_jcr, M_FATAL, 0,
               _("Packet size=%d too big from \"%s:%s:%d\". Maximum permitted 1000000. Terminating connection.\n"),
               pktsiz, m_who, m_host, m_port);
         pktsiz = BNET_TERMINATE;  /* hang up */
      }
      if (pktsiz == BNET_TERMINATE) {
         set_terminated();
      }
      timer_start = 0;                /* clear timer */
      b_errno = ENODATA;
      msglen = pktsiz;                /* signal code */
      nbytes =  BNET_SIGNAL;          /* signal */
      goto get_out;
   }

   /* Make sure the buffer is big enough + one byte for EOS */
   if (pktsiz >= (int32_t) sizeof_pool_memory(msg)) {
      msg = realloc_pool_memory(msg, pktsiz + 100);
   }

   timer_start = watchdog_time;  /* set start wait time */
   clear_timed_out();
   /* now read the actual data */
   if ((nbytes = read_nbytes(this, msg, pktsiz)) <= 0) {
      timer_start = 0;      /* clear timer */
      if (errno == 0) {
         b_errno = ENODATA;
      } else {
         b_errno = errno;
      }
      errors++;
      Qmsg4(m_jcr, M_ERROR, 0, _("Read error from %s:%s:%d: ERR=%s\n"),
            m_who, m_host, m_port, this->bstrerror());
      nbytes = BNET_ERROR;
      goto get_out;
   }
   timer_start = 0;         /* clear timer */
   in_msg_no++;
   msglen = nbytes;
   if (nbytes != pktsiz) {
      b_errno = EIO;
      errors++;
      Qmsg5(m_jcr, M_ERROR, 0, _("Read expected %d got %d from %s:%s:%d\n"),
            pktsiz, nbytes, m_who, m_host, m_port);
      nbytes = BNET_ERROR;
      goto get_out;
   }
   /* If compressed uncompress it */
   if (compressed) {
      int offset = 0;
      int psize = nbytes * 4;
      if (psize >= ((int32_t)sizeof_pool_memory(cmsg))) {
         cmsg = realloc_pool_memory(cmsg, psize);
      }
      psize = sizeof_pool_memory(cmsg);
      if (m_flags & BNET_OFFSET) {
         offset = m_flags & 0xFF;
         msg += offset;
         msglen -= offset;
      }
      /* Grow buffer to max approx 4MB */
      for (int i=0; i < 7; i++) {
         nbytes = LZ4_decompress_safe(msg, cmsg, msglen, psize);
         if (nbytes >=  0) {
            break;
         }
         if (psize < 65536) {
            psize = 65536;
         } else {
            psize = psize * 2;
         }
         if (psize >= ((int32_t)sizeof_pool_memory(cmsg))) {
            cmsg = realloc_pool_memory(cmsg, psize + 100);
         }
      }
      if (m_flags & BNET_OFFSET) {
         msg -= offset;
         msglen += offset;
      }
      if (nbytes < 0) {
         Jmsg1(m_jcr, M_ERROR, 0, "Decompress error!!!! ERR=%d\n", nbytes);
         Pmsg3(000, "Decompress error!! pktsiz=%d cmsgsiz=%d nbytes=%d\n", pktsiz,
           psize, nbytes);
         b_errno = EIO;
         errors++;
          Qmsg5(m_jcr, M_ERROR, 0, _("Read expected %d got %d from %s:%s:%d\n"),
               pktsiz, nbytes, m_who, m_host, m_port);
         nbytes = BNET_ERROR;
         goto get_out;
      }
      msglen = nbytes;
      /* Make sure the buffer is big enough + one byte for EOS */
      if (msglen >= (int32_t)sizeof_pool_memory(msg)) {
         msg = realloc_pool_memory(msg, msglen + 100);
      }
      /* If this is a data decompress, leave msg compressed */
      if (!(m_flags & BNET_OFFSET)) {
         memcpy(msg, cmsg, msglen);
      }
   }

   /* always add a zero by to properly terminate any
    * string that was send to us. Note, we ensured above that the
    * buffer is at least one byte longer than the message length.
    */
   msg[nbytes] = 0; /* terminate in case it is a string */
   /*
    * The following uses *lots* of resources so turn it on only for
    * serious debugging.
    */
   Dsm_check(300);

get_out:
   if ((chk_dbglvl(DT_NETWORK|1900))) dump_bsock_msg(m_fd, read_seqno, "RECV", nbytes, o_pktsiz, m_flags, msg, msglen);
   if (nbytes != BNET_ERROR && command) {
      nbytes = BNET_COMMAND;
   }

   if (locked) pV(pm_rmutex);
   return nbytes;                  /* return actual length of message */
}

/*
 * Send a signal
 */
bool BSOCK::signal(int signal)
{
   msglen = signal;
   if (signal == BNET_TERMINATE) {
      m_suppress_error_msgs = true;
   }
   return send();
}

/*
 * Despool spooled attributes
 */
bool BSOCK::despool(void update_attr_spool_size(ssize_t size), ssize_t tsize)
{
   int32_t pktsiz;
   size_t nbytes;
   ssize_t last = 0, size = 0;
   int count = 0;
   JCR *jcr = get_jcr();

   rewind(m_spool_fd);

#if defined(HAVE_POSIX_FADVISE) && defined(POSIX_FADV_WILLNEED)
   posix_fadvise(fileno(m_spool_fd), 0, 0, POSIX_FADV_WILLNEED);
#endif

   while (fread((char *)&pktsiz, 1, sizeof(int32_t), m_spool_fd) ==
          sizeof(int32_t)) {
      size += sizeof(int32_t);
      msglen = ntohl(pktsiz);
      if (msglen > 0) {
         if (msglen > (int32_t)sizeof_pool_memory(msg)) {
            msg = realloc_pool_memory(msg, msglen + 1);
         }
         nbytes = fread(msg, 1, msglen, m_spool_fd);
         if (nbytes != (size_t)msglen) {
            berrno be;
            Dmsg2(400, "nbytes=%d msglen=%d\n", nbytes, msglen);
            Qmsg2(get_jcr(), M_FATAL, 0, _("fread attr spool error. Wanted=%d got=%d bytes.\n"),
                  msglen, nbytes);
            update_attr_spool_size(tsize - last);
            return false;
         }
         size += nbytes;
         if ((++count & 0x3F) == 0) {
            update_attr_spool_size(size - last);
            last = size;
         }
      }
      send();
      if (jcr && job_canceled(jcr)) {
         return false;
      }
   }
   update_attr_spool_size(tsize - last);
   if (ferror(m_spool_fd)) {
      Qmsg(jcr, M_FATAL, 0, _("fread attr spool I/O error.\n"));
      return false;
   }
   return true;
}

/*
 * Return the string for the error that occurred
 * on the socket. Only the first error is retained.
 */
const char *BSOCK::bstrerror()
{
   berrno be;
   if (errmsg == NULL) {
      errmsg = get_pool_memory(PM_MESSAGE);
   }
   if (b_errno == 0) {
      pm_strcpy(errmsg, "I/O Error");
   } else {
      pm_strcpy(errmsg, be.bstrerror(b_errno));
   }
   return errmsg;
}

int BSOCK::get_peer(char *buf, socklen_t buflen)
{
#if !defined(HAVE_WIN32)
    if (peer_addr.sin_family == 0) {
        socklen_t salen = sizeof(peer_addr);
        int rval = (getpeername)(m_fd, (struct sockaddr *)&peer_addr, &salen);
        if (rval < 0) return rval;
    }
    if (!inet_ntop(peer_addr.sin_family, &peer_addr.sin_addr, buf, buflen))
        return -1;

    return 0;
#else
    return -1;
#endif
}

/*
 * Set the network buffer size, suggested size is in size.
 *  Actual size obtained is returned in bs->msglen
 *
 *  Returns: false on failure
 *           true  on success
 */
bool BSOCK::set_buffer_size(uint32_t size, int rw)
{
   uint32_t dbuf_size, start_size;

#if defined(IP_TOS) && defined(IPTOS_THROUGHPUT)
   int opt;
   opt = IPTOS_THROUGHPUT;
   setsockopt(m_fd, IPPROTO_IP, IP_TOS, (sockopt_val_t)&opt, sizeof(opt));
#endif

   if (size != 0) {
      dbuf_size = size;
   } else {
      dbuf_size = DEFAULT_NETWORK_BUFFER_SIZE;
   }
   start_size = dbuf_size;
   if ((msg = realloc_pool_memory(msg, dbuf_size + 100)) == NULL) {
      Qmsg0(get_jcr(), M_FATAL, 0, _("Could not malloc BSOCK data buffer\n"));
      return false;
   }

   /*
    * If user has not set the size, use the OS default -- i.e. do not
    *   try to set it.  This allows sys admins to set the size they
    *   want in the OS, and Bacula will comply. See bug #1493
    */
   if (size == 0) {
      msglen = dbuf_size;
      return true;
   }

   if (rw & BNET_SETBUF_READ) {
      while ((dbuf_size > TAPE_BSIZE) && (setsockopt(m_fd, SOL_SOCKET,
              SO_RCVBUF, (sockopt_val_t) & dbuf_size, sizeof(dbuf_size)) < 0)) {
         berrno be;
         Qmsg1(get_jcr(), M_ERROR, 0, _("sockopt error: %s\n"), be.bstrerror());
         dbuf_size -= TAPE_BSIZE;
      }
      Dmsg1(200, "set network buffer size=%d\n", dbuf_size);
      if (dbuf_size != start_size) {
         Qmsg1(get_jcr(), M_WARNING, 0,
               _("Warning network buffer = %d bytes not max size.\n"), dbuf_size);
      }
   }
   if (size != 0) {
      dbuf_size = size;
   } else {
      dbuf_size = DEFAULT_NETWORK_BUFFER_SIZE;
   }
   start_size = dbuf_size;
   if (rw & BNET_SETBUF_WRITE) {
      while ((dbuf_size > TAPE_BSIZE) && (setsockopt(m_fd, SOL_SOCKET,
              SO_SNDBUF, (sockopt_val_t) & dbuf_size, sizeof(dbuf_size)) < 0)) {
         berrno be;
         Qmsg1(get_jcr(), M_ERROR, 0, _("sockopt error: %s\n"), be.bstrerror());
         dbuf_size -= TAPE_BSIZE;
      }
      Dmsg1(900, "set network buffer size=%d\n", dbuf_size);
      if (dbuf_size != start_size) {
         Qmsg1(get_jcr(), M_WARNING, 0,
               _("Warning network buffer = %d bytes not max size.\n"), dbuf_size);
      }
   }

   msglen = dbuf_size;
   return true;
}

/*
 * Set socket non-blocking
 * Returns previous socket flag
 */
int BSOCK::set_nonblocking()
{
#ifndef HAVE_WIN32
   int oflags;

   /* Get current flags */
   if ((oflags = fcntl(m_fd, F_GETFL, 0)) < 0) {
      berrno be;
      Qmsg1(get_jcr(), M_ABORT, 0, _("fcntl F_GETFL error. ERR=%s\n"), be.bstrerror());
   }

   /* Set O_NONBLOCK flag */
   if ((fcntl(m_fd, F_SETFL, oflags|O_NONBLOCK)) < 0) {
      berrno be;
      Qmsg1(get_jcr(), M_ABORT, 0, _("fcntl F_SETFL error. ERR=%s\n"), be.bstrerror());
   }

   m_blocking = 0;
   return oflags;
#else
   int flags;
   u_long ioctlArg = 1;

   flags = m_blocking;
   ioctlsocket(m_fd, FIONBIO, &ioctlArg);
   m_blocking = 0;

   return flags;
#endif
}

/*
 * Set socket blocking
 * Returns previous socket flags
 */
int BSOCK::set_blocking()
{
#ifndef HAVE_WIN32
   int oflags;
   /* Get current flags */
   if ((oflags = fcntl(m_fd, F_GETFL, 0)) < 0) {
      berrno be;
      Qmsg1(get_jcr(), M_ABORT, 0, _("fcntl F_GETFL error. ERR=%s\n"), be.bstrerror());
   }

   /* Set O_NONBLOCK flag */
   if ((fcntl(m_fd, F_SETFL, oflags & ~O_NONBLOCK)) < 0) {
      berrno be;
      Qmsg1(get_jcr(), M_ABORT, 0, _("fcntl F_SETFL error. ERR=%s\n"), be.bstrerror());
   }

   m_blocking = 1;
   return oflags;
#else
   int flags;
   u_long ioctlArg = 0;

   flags = m_blocking;
   ioctlsocket(m_fd, FIONBIO, &ioctlArg);
   m_blocking = 1;

   return flags;
#endif
}

void BSOCK::set_killable(bool killable)
{
   if (m_jcr) {
      m_jcr->set_killable(killable);
   }
}

/*
 * Restores socket flags
 */
void BSOCK::restore_blocking (int flags)
{
#ifndef HAVE_WIN32
   if ((fcntl(m_fd, F_SETFL, flags)) < 0) {
      berrno be;
      Qmsg1(get_jcr(), M_ABORT, 0, _("fcntl F_SETFL error. ERR=%s\n"), be.bstrerror());
   }

   m_blocking = (flags & O_NONBLOCK) ? true : false;
#else
   u_long ioctlArg = flags;

   ioctlsocket(m_fd, FIONBIO, &ioctlArg);
   m_blocking = 1;
#endif
}

/*
 * Wait for a specified time for data to appear on
 * the BSOCK connection.
 *
 *   Returns: 1 if data available
 *            0 if timeout
 *           -1 if error
 */
int BSOCK::wait_data(int sec, int msec)
{
   for (;;) {
      switch (fd_wait_data(m_fd, WAIT_READ, sec, msec)) {
      case 0:                      /* timeout */
         b_errno = 0;
         return 0;
      case -1:
         b_errno = errno;
         if (errno == EINTR) {
            continue;
         }
         return -1;                /* error return */
      default:
         b_errno = 0;
#ifdef HAVE_TLS
         if (this->tls && !tls_bsock_probe(this)) {
            continue; /* false alarm, maybe a session key negotiation in progress on the socket */
         }
#endif
         return 1;
      }
   }
}

/*
 * As above, but returns on interrupt
 */
int BSOCK::wait_data_intr(int sec, int msec)
{
   switch (fd_wait_data(m_fd, WAIT_READ, sec, msec)) {
   case 0:                      /* timeout */
      b_errno = 0;
      return 0;
   case -1:
      b_errno = errno;
      return -1;                /* error return */
   default:
      b_errno = 0;
#ifdef HAVE_TLS
      if (this->tls && !tls_bsock_probe(this)) {
         /* maybe a session key negotiation waked up the socket */
         return 0;
      }
#endif
      break;
   }
   return 1;
}

/*
 *  This routine closes the current BSOCK.
 *   It does not delete the socket packet
 *   resources, which are released int
 *   bsock->destroy().
 */
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

/*
 * The JCR is canceled, set terminate for chained BSOCKs starting from master
 */
void BSOCK::cancel()
{
   master_lock();
   for (BSOCK *next = m_master; next != NULL; next = next->m_next) {
      if (!next->m_closed) {
         next->m_terminated = true;
         next->m_timed_out = true;
      }
   }
   master_unlock();
}

/*
 * Note, this routine closes the socket, but leaves the
 *   bsock memory in place.
 *   every thread is responsible of closing and destroying its own duped or not
 *   duped BSOCK
 */
void BSOCK::close()
{
   BSOCK *bsock = this;

   if (bsock->is_closed()) {
      return;
   }
   if (!m_duped) {
      clear_locking();
   }
   bsock->set_closed();
   bsock->set_terminated();
   if (!bsock->m_duped) {
      /* Shutdown tls cleanly. */
      if (bsock->tls) {
         tls_bsock_shutdown(bsock);
         free_tls_connection(bsock->tls);
         bsock->tls = NULL;
      }

#ifdef HAVE_WIN32
      if (!bsock->is_timed_out()) {
         win_close_wait(bsock->m_fd);  /* Ensure that data is not discarded */
      }
#else
      if (bsock->is_timed_out()) {
         shutdown(bsock->m_fd, SHUT_RDWR);   /* discard any pending I/O */
      }
#endif
      /* On Windows this discards data if we did not do a close_wait() */
      socketClose(bsock->m_fd);      /* normal close */
   }
   return;
}

/*
 * Destroy the socket (i.e. release all resources)
 */
void BSOCK::_destroy()
{
   this->close();                  /* Ensure that socket is closed */

   if (msg) {
      free_pool_memory(msg);
      msg = NULL;
   } else {
      ASSERT2(1 == 0, "Two calls to destroy socket");  /* double destroy */
   }
   if (cmsg) {
      free_pool_memory(cmsg);
      cmsg = NULL;
   }
   if (errmsg) {
      free_pool_memory(errmsg);
      errmsg = NULL;
   }
   if (m_who) {
      free(m_who);
      m_who = NULL;
   }
   if (m_host) {
      free(m_host);
      m_host = NULL;
   }
   if (src_addr) {
      free(src_addr);
      src_addr = NULL;
   }
   free(this);
}

/*
 * Destroy the socket (i.e. release all resources)
 * including duped sockets.
 * should not be called from duped BSOCK
 */
void BSOCK::destroy()
{
   ASSERTD(reinterpret_cast<uintptr_t>(m_next) != 0xaaaaaaaaaaaaaaaa, "BSOCK::destroy() already called\n")
   ASSERTD(this == m_master, "BSOCK::destroy() called by a non master BSOCK\n")
   ASSERTD(!m_duped, "BSOCK::destroy() called by a duped BSOCK\n")
   /* I'm the master I must destroy() all the duped BSOCKs */
   master_lock();
   BSOCK *ahead;
   for (BSOCK *next = m_next; next != NULL; next = ahead) {
      ahead = next->m_next;
      next->_destroy();
   }
   master_unlock();
   _destroy();
}

/* Commands sent to Director */
static char hello[]    = "Hello %s calling\n";

/* Response from Director */
static char OKhello[]   = "1000 OK:";

/*
 * Authenticate Director
 */
bool BSOCK::authenticate_director(const char *name, const char *password,
               TLS_CONTEXT *tls_ctx, char *errmsg, int errmsg_len)
{
   int tls_local_need = BNET_TLS_NONE;
   int tls_remote_need = BNET_TLS_NONE;
   int compatible = true;
   char bashed_name[MAX_NAME_LENGTH];
   BSOCK *dir = this;        /* for readability */

   *errmsg = 0;
   /*
    * Send my name to the Director then do authentication
    */

   /* Timeout Hello after 15 secs */
   dir->start_timer(15);
   dir->fsend(hello, bashed_name);

   if (get_tls_enable(tls_ctx)) {
      tls_local_need = get_tls_enable(tls_ctx) ? BNET_TLS_REQUIRED : BNET_TLS_OK;
   }

   /* respond to Dir challenge */
   if (!cram_md5_respond(dir, password, &tls_remote_need, &compatible) ||
       /* Now challenge dir */
       !cram_md5_challenge(dir, password, tls_local_need, compatible)) {
      bsnprintf(errmsg, errmsg_len, _("Director authorization error at \"%s:%d\"\n"),
         dir->host(), dir->port());
      goto bail_out;
   }

   /* Verify that the remote host is willing to meet our TLS requirements */
   if (tls_remote_need < tls_local_need && tls_local_need != BNET_TLS_OK && tls_remote_need != BNET_TLS_OK) {
      bsnprintf(errmsg, errmsg_len, _("Authorization error:"
             " Remote server at \"%s:%d\" did not advertise required TLS support.\n"),
             dir->host(), dir->port());
      goto bail_out;
   }

   /* Verify that we are willing to meet the remote host's requirements */
   if (tls_remote_need > tls_local_need && tls_local_need != BNET_TLS_OK && tls_remote_need != BNET_TLS_OK) {
      bsnprintf(errmsg, errmsg_len, _("Authorization error with Director at \"%s:%d\":"
                     " Remote server requires TLS.\n"),
                     dir->host(), dir->port());

      goto bail_out;
   }

   /* Is TLS Enabled? */
   if (have_tls) {
      if (tls_local_need >= BNET_TLS_OK && tls_remote_need >= BNET_TLS_OK) {
         /* Engage TLS! Full Speed Ahead! */
         if (!bnet_tls_client(tls_ctx, dir, NULL)) {
            bsnprintf(errmsg, errmsg_len, _("TLS negotiation failed with Director at \"%s:%d\"\n"),
               dir->host(), dir->port());
            goto bail_out;
         }
      }
   }

   Dmsg1(6, ">dird: %s", dir->msg);
   if (dir->recv() <= 0) {
      dir->stop_timer();
      bsnprintf(errmsg, errmsg_len, _("Bad errmsg to Hello command: ERR=%s\n"
                      "The Director at \"%s:%d\" may not be running.\n"),
                    dir->bstrerror(), dir->host(), dir->port());
      return false;
   }

   dir->stop_timer();
   Dmsg1(10, "<dird: %s", dir->msg);
   if (strncmp(dir->msg, OKhello, sizeof(OKhello)-1) != 0) {
      bsnprintf(errmsg, errmsg_len, _("Director at \"%s:%d\" rejected Hello command\n"),
         dir->host(), dir->port());
      return false;
   } else {
      bsnprintf(errmsg, errmsg_len, "%s", dir->msg);
   }
   return true;

bail_out:
   dir->stop_timer();
   bsnprintf(errmsg, errmsg_len, _("Authorization error with Director at \"%s:%d\"\n"
             "Most likely the passwords do not agree.\n"
             "If you are using TLS, there may have been a certificate validation error during the TLS handshake.\n"
             "For help, please see: " MANUAL_AUTH_URL "\n"),
             dir->host(), dir->port());
   return false;
}

/* Try to limit the bandwidth of a network connection
 */
void BSOCK::control_bwlimit(int bytes)
{
   btime_t now, temp;
   if (bytes == 0) {
      return;
   }

   now = get_current_btime();          /* microseconds */
   temp = now - m_last_tick;           /* microseconds */

   m_nb_bytes += bytes;

   if (temp < 0 || temp > 10000000) { /* Take care of clock problems (>10s) or back in time */
      m_nb_bytes = bytes;
      m_last_tick = now;
      return;
   }

   /* Less than 0.1ms since the last call, see the next time */
   if (temp < 100) {
      return;
   }

   /* Remove what was authorised to be written in temp us */
   m_nb_bytes -= (int64_t)(temp * ((double)m_bwlimit / 1000000.0));

   if (m_nb_bytes < 0) {
      m_nb_bytes = 0;
   }

   /* What exceed should be converted in sleep time */
   int64_t usec_sleep = (int64_t)(m_nb_bytes /((double)m_bwlimit / 1000000.0));
   if (usec_sleep > 100) {
      bmicrosleep(usec_sleep/1000000, usec_sleep%1000000); /* TODO: Check that bmicrosleep slept enough or sleep again */
      m_last_tick = get_current_btime();
      m_nb_bytes = 0;
   } else {
      m_last_tick = now;
   }
}

#ifdef HAVE_WIN32
/*
 * closesocket is supposed to do a graceful disconnect under Window
 *   but it doesn't. Comments on http://msdn.microsoft.com/en-us/li
 *   confirm this behaviour. DisconnectEx is required instead, but
 *   that function needs to be retrieved via WS IOCTL
 */
static void
win_close_wait(int fd)
{
   int ret;
   GUID disconnectex_guid = WSAID_DISCONNECTEX;
   DWORD bytes_returned;
   LPFN_DISCONNECTEX DisconnectEx;
   ret = WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &disconnectex_guid, sizeof(disconnectex_guid), &DisconnectEx, sizeof(DisconnectEx), &bytes_returned, NULL, NULL);
   Dmsg1(100, "WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, WSAID_DISCONNECTEX) ret = %d\n", ret);
   if (!ret) {
      DisconnectEx(fd, NULL, 0, 0);
   }
}
#endif
