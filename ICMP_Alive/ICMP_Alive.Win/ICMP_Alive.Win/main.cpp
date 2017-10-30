#include "IcmpDetect.h"
#include "resource.h"
#include "global.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Shlwapi.lib")

#define NETADDR_BUF_LEN	16

HWND g_hListWnd;
IcmpDetect* g_IcmpDetect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dlg_OnInitDialog(HWND hWnd)
{
	HWND hItem = GetDlgItem(hWnd, IDC_NETADDR);
	SetFocus(hItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dlg_OnCommand(HWND hWnd, int id)
{
	g_hListWnd = GetDlgItem(hWnd, IDC_LIST);
	
	switch (id)
	{
	case IDCANCEL:
		EndDialog(hWnd, id);
		break;

	case IDC_BEGIN_BTN:
		u_int net, mask;
		CHAR szNetAddr[NETADDR_BUF_LEN];
		CHAR szNetMask[NETADDR_BUF_LEN];

		GetDlgItemText(hWnd, IDC_NETADDR, szNetAddr, sizeof(szNetAddr));
		GetDlgItemText(hWnd, IDC_NETMASK, szNetMask, sizeof(szNetMask));
		net = inet_addr(szNetAddr);
		mask = inet_addr(szNetMask);

		g_IcmpDetect = new IcmpDetect(net, mask);
		g_IcmpDetect->beginDetect();

		break;

	case IDC_STOP_BTN:
		if (g_IcmpDetect)
		{
			g_IcmpDetect->stopDetect();
			delete g_IcmpDetect;
			g_IcmpDetect = NULL;
		}
		break;

	case IDC_RESET_BTN:
		ResetContent(g_hListWnd);
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Dlg_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		Dlg_OnCommand(hWnd, LOWORD(wParam));
		break;
	case WM_INITDIALOG:
		Dlg_OnInitDialog(hWnd);
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, Dlg_Proc);
	return 0;
}

/////////////////////////////////////////////////// End //////////////////////////////////////////////////