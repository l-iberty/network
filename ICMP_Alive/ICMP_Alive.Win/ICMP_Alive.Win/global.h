#ifndef GLOBAL_H
#define GLOBAL_H

#define AddString(hListWnd, szString) SendMessage(hListWnd, LB_ADDSTRING, NULL, (LPARAM)szString)
#define ResetContent(hListWnd) SendMessage(hListWnd, LB_RESETCONTENT, 0, 0)

#endif // GLOBAL_H