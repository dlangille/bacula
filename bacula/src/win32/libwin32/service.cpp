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
 * 
 *  Kern Sibbald, August 2007
 *
 * This is a generic service routine, which is used by all three
 *  of the daemons. Each one compiles it with slightly different
 *  #defines.
 *
 */

#include "bacula.h"
#include "win32.h"

/* Forward reference */
static void set_service_description(SC_HANDLE hSCManager, 
                                    SC_HANDLE hService, LPSTR lpDesc);
  
/* Other Window component dependencies */
#define BAC_DEPENDENCIES __TEXT("tcpip\0afd\0") 

/* Service globals */
SERVICE_STATUS_HANDLE  service_handle;
SERVICE_STATUS service_status;
DWORD service_error = 0;
static bool is_service = false;

/* Forward references */
void WINAPI serviceControlCallback(DWORD ctrlcode);
BOOL ReportStatus(DWORD state, DWORD exitcode, DWORD waithint);
DWORD WINAPI baculaWorkerThread(LPVOID lpwThreadParam);


/*
 * Post a message to a running instance of the app
 */
bool postToBacula(UINT message, WPARAM wParam, LPARAM lParam)
{
   /* Locate the Bacula menu window */
   HWND hservwnd = FindWindow(APP_NAME, NULL);
   if (hservwnd == NULL) {
      return false;
   }

   /* Post the message to Bacula */
   PostMessage(hservwnd, message, wParam, lParam);
   return true;
}


/*
 * Running as a service?
 */
bool isAService()
{
   return is_service;
}

/* 
 * terminate any running Bacula
 */
int stopRunningBacula()
{
   postToBacula(WM_CLOSE, 0, 0);
   sleep(5);
   return 0;
}

/*
 * New style service start callback handler for the OS.
 *   the OS returns control here immediately after starting
 *   the service.
 */
void WINAPI serviceStartCallback(DWORD argc, char **argv)
{
   DWORD dwThreadID;
  
   /* Register our service */
   service_handle = RegisterServiceCtrlHandler(APP_NAME, serviceControlCallback);
   if (!service_handle) {
      log_error_message(_("RegisterServiceCtlHandler failed")); 
      MessageBox(NULL, _("Failure contacting the Service Handler"),
         APP_DESC, MB_OK);
      return;
   }

   service_status.dwServiceType = SERVICE_WIN32;
   service_status.dwServiceSpecificExitCode = 0;

   /* Report status */
   if (!ReportStatus(SERVICE_START_PENDING, NO_ERROR, 45000)) {
       ReportStatus(SERVICE_STOPPED, service_error,  0);
       log_error_message(_("Service start report failed")); 
       return;
   }

   /* Now create the Bacula worker thread */
   (void)CreateThread(NULL, 0, baculaWorkerThread, NULL, 0, &dwThreadID);
   return;
}

/*
 *  Stop our service 
 */
static void serviceStop()
{
   /* Post a quit message our service thread */
   if (service_thread_id != 0) {
      PostThreadMessage(service_thread_id, WM_QUIT, 0, 0);
   }
}

/*
 * Service Control callback handler.  The OS can call us here
 *   at any time, most often to stop the service.
 */
void WINAPI serviceControlCallback(DWORD ctrlcode)
{
   switch(ctrlcode) {
   case SERVICE_CONTROL_STOP:
      service_status.dwCurrentState = SERVICE_STOP_PENDING;
      serviceStop();  /* our stop service routine */
      break;
   }

   /* Report our status */
   ReportStatus(service_status.dwCurrentState, NO_ERROR, 0);
}


/*
 * Run Bacula as a service 
 */
int baculaServiceMain()
{
   is_service = true;                 /* indicate we are running as a service */

   if (have_service_api) {            /* New style service API */
      /* Tell OS where to dispatch service calls to us */
      SERVICE_TABLE_ENTRY dispatchTable[] = {
         {(char *)APP_NAME, (LPSERVICE_MAIN_FUNCTION)serviceStartCallback},
         {NULL, NULL}};

      /* Start the service control dispatcher */
      if (!StartServiceCtrlDispatcher(dispatchTable)) {
         log_error_message(_("StartServiceCtrlDispatcher failed."));
      }
      /* Note, this thread continues in the ServiceCallback routine */

   } else {                           /* old style Win95/98/Me */
      HINSTANCE kerneldll = LoadLibrary("KERNEL32.DLL");
      if (kerneldll == NULL) {
         MessageBox(NULL, _("KERNEL32.DLL not found: Bacula service not started"), 
             APP_DESC, MB_OK);
         return 1;
      }

      /* Get entry point for RegisterServiceProcess function */
      DWORD (WINAPI *RegisterService)(DWORD, DWORD);
      RegisterService = (DWORD (WINAPI *)(DWORD, DWORD))
              GetProcAddress(kerneldll, "RegisterServiceProcess");
      if (RegisterService == NULL) {
         MessageBox(NULL, _("Registry service not found: Bacula service not started"),
            APP_DESC, MB_OK);
         log_error_message(_("Registry service entry point not found")); 
         FreeLibrary(kerneldll);         /* free up kernel dll */
         return 1;
      }
      
      RegisterService(0, 1);             /* register us as a service */
      BaculaAppMain();                   /* call the main Bacula code */
      RegisterService(0, 0);             /* terminate the service */
      FreeLibrary(kerneldll);            /* free up kernel dll */
   } 
   return 0;
}


/* 
 * New style service bacula worker thread
 */
DWORD WINAPI baculaWorkerThread(LPVOID lpwThreadParam)
{
   service_thread_id = GetCurrentThreadId();

   if (!ReportStatus(SERVICE_RUNNING, NO_ERROR, 0)) {
      MessageBox(NULL, _("Report Service failure"), APP_DESC, MB_OK);
      log_error_message("ReportStatus RUNNING failed"); 
      return 0;
   }

   /* Call Bacula main code */
   BaculaAppMain();

   /* Mark that we're no longer running */
   service_thread_id = 0;

   /* Tell the service manager that we've stopped */
   ReportStatus(SERVICE_STOPPED, service_error, 0);
   return 0;
}



/*
 * Install the Bacula service on the OS -- very complicated
 */
int installService(const char *cmdOpts)
{
   const int maxlen = 2048;
   char path[maxlen];
   char svcmd[maxlen];

   bsnprintf(svcmd, sizeof(svcmd), "service: install: %s", cmdOpts, APP_DESC, MB_OK);

   /* Get our filename */
   if (GetModuleFileName(NULL, path, maxlen-11) == 0) {
      MessageBox(NULL, _("Unable to install the service"), APP_DESC, MB_ICONEXCLAMATION | MB_OK);
      return 0;
   }

   /* Create a valid command for starting the service */
   if ((int)strlen(path) + (int)strlen(cmdOpts) + 30  < maxlen) {
      bsnprintf(svcmd, sizeof(svcmd), "\"%s\" /service %s", path, cmdOpts);
   } else {
      log_error_message(_("Service command length too long")); 
      MessageBox(NULL, _("Service command length too long. Service not registered."),
          APP_DESC, MB_ICONEXCLAMATION | MB_OK);
      return 0;
   }

   if (have_service_api) {
      SC_HANDLE baculaService, serviceManager;

      /* Open the service control manager */
      serviceManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
      if (!serviceManager) {
         log_error_message("Open Service Manager failed"); 
         MessageBox(NULL,
            _("The Service Control Manager could not be contacted - the service was not installed"),
            APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         return 0;
      }

      /* Now actually create the Bacula service entry */
      baculaService = CreateService(
              serviceManager, 
              APP_NAME,                       /* Our service name */
              APP_DESC,                       /* Display name */
              SERVICE_ALL_ACCESS,
              SERVICE_WIN32_OWN_PROCESS,      /* | SERVICE_INTERACTIVE_PROCESS, */
              SERVICE_AUTO_START,
              SERVICE_ERROR_NORMAL,
              svcmd,                          /* Command string to start the service */
              NULL,
              NULL,
              BAC_DEPENDENCIES,               /* Services to start before us */
              NULL,                           /* Use default SYSTEM account */
              NULL);
      if (!baculaService) {
         CloseServiceHandle(serviceManager);
         log_error_message("CreateService failed for " APP_DESC); 
         MessageBox(NULL, _("The Bacula service: " APP_NAME " could not be installed"),
              APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         return 0;
      }

      /* Set a text description in the service manager's control panel */
      set_service_description(serviceManager, baculaService,
(char *)_("Provides file backup and restore services. Bacula -- the network backup solution."));

      CloseServiceHandle(serviceManager);
      CloseServiceHandle(baculaService);

   } else {
      /* Old style service -- create appropriate registry key path */
      HKEY runservices;
      if (RegCreateKey(HKEY_LOCAL_MACHINE, 
              "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices",
              &runservices) != ERROR_SUCCESS) {
         log_error_message(_("Cannot write System Registry for " APP_DESC)); 
         MessageBox(NULL, _("The System Registry could not be updated - the Bacula service was not installed"), 
             APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         return 0;
      }

      /* Add the Bacula values */
      if (RegSetValueEx(runservices, APP_NAME, 0, REG_SZ, 
                        (unsigned char *)svcmd, strlen(svcmd)+1) != ERROR_SUCCESS) {
         RegCloseKey(runservices);
         log_error_message(_("Cannot add Bacula key to System Registry")); 
         MessageBox(NULL, _("The Bacula service: " APP_NAME " could not be installed"), 
             APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         return 0;
      }
      RegCloseKey(runservices);
   }

   /* At this point the service is installed */
   if (opt_debug) {
      MessageBox(NULL,
           _("The " APP_DESC "was successfully installed.\n"
             "The service may be started by double clicking on the\n"
             "Bacula \"Start\" icon and will be automatically\n"
             "be run the next time this machine is rebooted. "),
         APP_DESC, MB_ICONINFORMATION | MB_OK);
   }
   return 0;
}


/*
 * Remove a service from the OS (normally done when we are installing
 *   a new version).
 */
int removeService()
{
   SC_HANDLE serviceManager, baculaService;
   int stat = 0;

   if (have_service_api) {      /* Newer Windows platforms (NT, Win2K, ...) */
      serviceManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
      if (serviceManager) {
         /* Now get the Bacula service entry */
         baculaService = OpenService(serviceManager, APP_NAME, SERVICE_ALL_ACCESS);
         if (baculaService) {
            SERVICE_STATUS status;
            /* If the service is running, stop it */
            if (ControlService(baculaService, SERVICE_CONTROL_STOP, &status)) {
               while(QueryServiceStatus(baculaService, &status)) {
                  if (status.dwCurrentState != SERVICE_STOP_PENDING) {
                     break; 
                  }
                  sleep(1);
               }
               if (status.dwCurrentState != SERVICE_STOPPED) {
                  if (opt_debug) {
                     MessageBox(NULL, _("The Bacula service: " APP_NAME " could not be stopped"), 
                        APP_DESC, MB_ICONEXCLAMATION | MB_OK);
                  }
               }
            }
            if (DeleteService(baculaService)) {
               if (opt_debug) {
                  MessageBox(NULL, _("The Bacula service: " APP_NAME " has been removed"), 
                     APP_DESC, MB_ICONINFORMATION | MB_OK);
               }
            } else {
               MessageBox(NULL, _("The Bacula service: " APP_NAME " could not be removed"), 
                  APP_DESC, MB_ICONEXCLAMATION | MB_OK);
               stat = 1; /* error */
            }
            CloseServiceHandle(baculaService);
         } else {
            if (opt_debug) {
               MessageBox(NULL, _("An existing Bacula service: " APP_NAME " could not be found for "
                   "removal. This is not normally an error."),
                   APP_DESC, MB_ICONEXCLAMATION | MB_OK);
            }
         }
         CloseServiceHandle(serviceManager);
         return stat;
      } else {
         MessageBox(NULL, _("The service Manager could not be contacted - the Bacula service was not removed"), 
            APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         return 1; /* error */
      }

   } else {                     /* Old Win95/98/Me OS */
      /* Open the registry path key */
      HKEY runservices;
      if (RegOpenKey(HKEY_LOCAL_MACHINE, 
            "Software\\Microsoft\\Windows\\CurrentVersion\\RunServices",
            &runservices) != ERROR_SUCCESS) {
         if (opt_debug) {
            MessageBox(NULL, 
               _("Could not find registry entry.\nService probably not registerd - the Bacula service was not removed"), 
                  APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         }
      } else {
         /* Now delete the Bacula entry */
         if (RegDeleteValue(runservices, APP_NAME) != ERROR_SUCCESS) {
            RegCloseKey(runservices);
            MessageBox(NULL, _("Could not delete Registry key for " APP_NAME ".\n"
               "The Bacula service could not be removed"), APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         }
         RegCloseKey(runservices);
         return 1;
      }
      /* Stop any running Bacula */
      if (!stopRunningBacula()) {
         if (opt_debug) {
            MessageBox(NULL,
                _("Bacula could not be contacted, probably not running"),
                APP_DESC, MB_ICONEXCLAMATION | MB_OK);
         }
         return 0;   /* not really an error */
      }
      /* At this point, the service has been removed */
      if (opt_debug) {
         MessageBox(NULL, _("The Bacula service has been removed"), APP_DESC, MB_ICONINFORMATION | MB_OK);
      }
   }
   return 0;
}


/*
 * This subroutine is called to report our current status to the
 *  new style service manager
 */
BOOL ReportStatus(DWORD state, DWORD exitcode, DWORD waithint)
{
   static DWORD checkpoint = 1;
   BOOL result = TRUE;

   /* No callbacks until we are started */
   if (state == SERVICE_START_PENDING) {
      service_status.dwControlsAccepted = 0;
   } else {
      service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
   }

   /* Save global service_status state */
   service_status.dwCurrentState = state;
   service_status.dwWin32ExitCode = exitcode;
   service_status.dwWaitHint = waithint;

   /*
    * Update the checkpoint variable so the service manager knows
    *   we are alive.
    */
   if (state == SERVICE_RUNNING || state == SERVICE_STOPPED) {
      service_status.dwCheckPoint = 0;
   } else {
      service_status.dwCheckPoint = checkpoint++;
   }

   /* Send our new status */
   result = SetServiceStatus(service_handle, &service_status);
   if (!result) {
      log_error_message(_("SetServiceStatus failed"));
   }
   return result;
}

/* Log an error message for the last Windows error */
void LogLastErrorMsg(const char *message, const char *fname, int lineno)
{
   char msgbuf[500];
   HANDLE eventHandler;
   const char *strings[3];
   LPTSTR msg;

   service_error = GetLastError();
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 service_error,
                 0,
                 (LPTSTR)&msg,
                 0,
                 NULL);

   /* Use the OS event logging to log the error */
   eventHandler = RegisterEventSource(NULL, APP_NAME);

   bsnprintf(msgbuf, sizeof(msgbuf), _("\n\n%s error: %ld at %s:%d"), 
      APP_NAME, service_error, fname, lineno);

   strings[0] = msgbuf;
   strings[1] = message;
   strings[2] = msg;

   if (eventHandler) {
      ReportEvent(eventHandler, EVENTLOG_ERROR_TYPE,
              0,                      /* category */
              0,                      /* ID */
              NULL,                   /* SID */
              3,                      /* Number of strings */
              0,                      /* raw data size */
              (const char **)strings, /* error strings */
              NULL);                  /* raw data */
      DeregisterEventSource(eventHandler);
   }
   LocalFree(msg);
}

typedef BOOL  (WINAPI * WinAPI)(SC_HANDLE, DWORD, LPVOID);

/*
 * This is amazingly complicated just to get a bit of English explanation
 *  in the service manager's dialog box.
 */
static void set_service_description(SC_HANDLE hSCManager, SC_HANDLE hService,
                             LPSTR lpDesc) 
{ 
    SC_LOCK sclLock; 
    LPQUERY_SERVICE_LOCK_STATUS lpqslsBuf; 
    SERVICE_DESCRIPTION sdBuf;
    DWORD dwBytesNeeded;
    WinAPI ChangeServiceDescription;
 
    HINSTANCE hLib = LoadLibrary("ADVAPI32.DLL");
    if (!hLib) {
       return;
    }
    ChangeServiceDescription = (WinAPI)GetProcAddress(hLib,
       "ChangeServiceConfig2A");
    FreeLibrary(hLib);
    if (!ChangeServiceDescription) {
       return;
    }
    
    // Need to acquire database lock before reconfiguring. 
    sclLock = LockServiceDatabase(hSCManager); 
 
    // If the database cannot be locked, report the details. 
    if (sclLock == NULL) {
       // Exit if the database is not locked by another process. 
       if (GetLastError() != ERROR_SERVICE_DATABASE_LOCKED) {
          log_error_message("LockServiceDatabase"); 
          return;
       }
 
       // Allocate a buffer to get details about the lock. 
       lpqslsBuf = (LPQUERY_SERVICE_LOCK_STATUS)LocalAlloc( 
            LPTR, sizeof(QUERY_SERVICE_LOCK_STATUS)+256); 
       if (lpqslsBuf == NULL) {
          log_error_message("LocalAlloc"); 
          return;
       }
 
       // Get and print the lock status information. 
       if (!QueryServiceLockStatus( 
              hSCManager, 
              lpqslsBuf, 
              sizeof(QUERY_SERVICE_LOCK_STATUS)+256, 
              &dwBytesNeeded)) {
          log_error_message("QueryServiceLockStatus"); 
       }
 
       if (lpqslsBuf->fIsLocked) {
          printf(_("Locked by: %s, duration: %ld seconds\n"), 
                lpqslsBuf->lpLockOwner, 
                lpqslsBuf->dwLockDuration); 
       } else {
          printf(_("No longer locked\n")); 
       }
 
       LocalFree(lpqslsBuf); 
       log_error_message(_("Could not lock database")); 
       return;
    } 
 
    // The database is locked, so it is safe to make changes. 
 
    sdBuf.lpDescription = lpDesc;

    if (!ChangeServiceDescription(
         hService,                   // handle to service
         SERVICE_CONFIG_DESCRIPTION, // change: description
         &sdBuf) ) {                 // value: new description
       log_error_message("ChangeServiceConfig2");
    }

    // Release the database lock. 
    UnlockServiceDatabase(sclLock); 
}
