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
 * Bacula File daemon Status Dialog box
 *
 * Kern Sibbald, August 2007
 *
 */

#include "bacula.h"
#include "win32.h"
#include "statusDialog.h"
#include "lib/status.h"

static BOOL CALLBACK dialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   /* Get class pointer from user data */
   statusDialog *statDlg = (statusDialog *)GetWindowLong(hDlg, GWL_USERDATA);

   switch (uMsg) {
   case WM_INITDIALOG:
      /* Set class pointer in user data */
      SetWindowLong(hDlg, GWL_USERDATA, lParam);
      statDlg = (statusDialog *)lParam;
      statDlg->m_textWin = GetDlgItem(hDlg, IDC_TEXTDISPLAY);

      /* show the dialog */
      SetForegroundWindow(hDlg);

      /* Update every 5 seconds */
      SetTimer(hDlg, 1, 5000, NULL); 
      statDlg->m_visible = true;
      statDlg->display();
      return true;

   case WM_TIMER:
      statDlg->display();
      return true;

   case WM_SIZE:
      statDlg->resize(hDlg, LOWORD(lParam), HIWORD(lParam));
      return true;

   case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDCANCEL:
      case IDOK:
         statDlg->m_visible = false;
         KillTimer(hDlg, 1);
         EndDialog(hDlg, true);
         return true;
      }
      break;

   case WM_DESTROY:
      statDlg->m_textWin = NULL;
      statDlg->m_visible = false;
      KillTimer(hDlg, 1);
      EndDialog(hDlg, false);
      return true;
   }
   return false;
}


static void displayString(const char *msg, int len, void *context)
{
   /* Get class pointer from user data */
   statusDialog *statDlg = (statusDialog *)context;
   const char *start = msg;
   const char *p;
   char *str;

   for (p=start; *p; p++) {
      if (*p == '\n') {
         int len = p - start;
         if (len > 0) {
            str = (char *)alloca(len + 1);
            bstrncpy(str, start, len + 1);

            SendMessage(statDlg->m_textWin, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(statDlg->m_textWin, EM_REPLACESEL, 0, (LPARAM)str);
         }
         
         if (*p == '\n') {
            SendMessage(statDlg->m_textWin, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
            SendMessage(statDlg->m_textWin, EM_REPLACESEL, 0, (LPARAM)"\r\n");
         }

         if (*p == '\0'){
            break;
         }
         start = p + 1;
      }
   }
}

void statusDialog::display()
{
   if (m_textWin != NULL) {
      STATUS_PKT sp;
      long hPos = GetScrollPos(m_textWin, SB_HORZ);
      long vPos = GetScrollPos(m_textWin, SB_VERT);
      long selStart;
      long selEnd;

      SendMessage(m_textWin, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

      SetWindowText(m_textWin, "");
      sp.bs = NULL;
      sp.context = this;
      sp.callback = displayString;
      output_status(&sp);

      SendMessage(m_textWin, EM_SETSEL, selStart, selEnd);
      SendMessage(m_textWin, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, hPos), 0);
      SendMessage(m_textWin, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, vPos), 0);
   }
}

/* Dialog box handling functions */
void statusDialog::show(bool show)
{
   if (show && !m_visible) {
      DialogBoxParam(appInstance, MAKEINTRESOURCE(IDD_STATUS), NULL,
          (DLGPROC)dialogProc, (LPARAM)this);
   }
}

/*
 * Make sure OK button is positioned in the right place
 */
void statusDialog::resize(HWND dWin, int dWidth, int dHeight)
{
   int bWidth, bHeight;   
   RECT bRect;
   HWND bWin;

   if (m_textWin != NULL) {
      bWin = GetDlgItem(dWin, IDOK);  /* get size of OK button */

      GetWindowRect(bWin, &bRect);
      bWidth = bRect.right - bRect.left;
      bHeight = bRect.bottom - bRect.top;

      MoveWindow(m_textWin, 8, 8, dWidth-bWidth-24, dHeight-16, true);
      MoveWindow(bWin, dWidth - bWidth-8, 8, bWidth, bHeight, true);
   }
}
