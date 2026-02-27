	/*----------------------------------------------------------------------------

		nXv9 by xBaRr/Syndrome 2000
			(nXs3) by xBaRr/Syndrome alias Emmanuel JULIEN
			All rights reserved 2000.

		Part of nXng.
		mailto://xbarr@pingoo.com
		http://xbarr.planet-d.net

		nXng Console implementation file

	----------------------------------------------------------------------------*/


	#include	<windows.h>
	#include	<commctrl.h>
	#include	<stdio.h>
	#include	"resource.h"

	#include	"IMG.h"
	#include	"nXng/nX_Version.h"
	#include	"nXng/nX_Console.h"

	static		HBRUSH			bgBrush;
	static		HBRUSH			lbbgBrush;
//	static		nX_Console		*ruler;
	static		char			str[2048];

	// define to output to CONSOLEFILE
//	#define		CONSOLEFILE		"c:\\nXconsole.txt"
//	#define		CONSOLETOFILE

	// Console color
	#define		CONSOLE_R		150
	#define		CONSOLE_G		177
	#define		CONSOLE_B		206

	// BG Console color
	#define		CONSOLEBG_R		20
	#define		CONSOLEBG_G		30
	#define		CONSOLEBG_B		60


static int CALLBACK ConsoleProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
		case WM_CTLCOLORLISTBOX:
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, 0xffeedd);
			return long(lbbgBrush);
		case WM_CTLCOLORDLG:
			SetBkColor((HDC)wParam, CONSOLE_B * 65536 + CONSOLE_G * 256 + CONSOLE_R);
			return long(bgBrush);
		case WM_CTLCOLORSTATIC:
			SetBkColor((HDC)wParam, CONSOLE_B * 65536 + CONSOLE_G * 256 + CONSOLE_R);
			return long(bgBrush);

		case WM_INITDIALOG:
			return true;

        case WM_COMMAND:
			switch (wParam)
			{
				case IDC_CLEARCON:
//					ruler->Clear();
					break;
			}
			switch (HIWORD(wParam))
            {
				case LBN_SETFOCUS:
					SetFocus(GetDlgItem(hDlg, IDC_PROGRESS));
					return false;
			}
			break;

	}
	return false;
}

void nX_Console::CreateConsole(HINSTANCE hInst, HWND ephWnd)
{
	hInstance = hInst;
	phWnd = ephWnd;
    bgBrush = CreateSolidBrush(CONSOLE_B * 65536 + CONSOLE_G * 256 + CONSOLE_R);
	lbbgBrush = CreateSolidBrush(CONSOLEBG_B * 65536 + CONSOLEBG_G * 256 + CONSOLEBG_R);
	hWnd = CreateDialog(hInstance, "CONS", phWnd, ConsoleProc);
	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);
	active = true;
}

void nX_Console::Message()
{
	MSG		msg;
	while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
        DispatchMessage(&msg); 
}

void nX_Console::DestroyConsole()
{
	if (active)
	{
		DestroyWindow(hWnd);
		active = false;
	}
}

void nX_Console::PrintOver(char *str)
{
	long count;

	if (active)
	{
		Message();
		count = SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_GETCOUNT, 0, 0);
		SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_DELETESTRING, count - 1, 0);
		SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_ADDSTRING, 0, (LPARAM)str);
		SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_SETCURSEL, count - 1, 0);
		UpdateWindow(hWnd);
		ShowWindow(hWnd, SW_SHOWNORMAL);
		#ifdef	CONSOLETOFILE
			FILE		*file;
			file = fopen(CONSOLEFILE, "a+");
			fwrite(str, 1, strlen(str), file);
			fwrite("\n", 1, 2, file);
			fclose(file);
		#endif
	}
}

void nX_Console::Print(char *str)
{
	long count;

	if (active)
	{
		Message();
		count = SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_GETCOUNT, 0, 0);
		SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_ADDSTRING, 0, (LPARAM)str);
		SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_SETCURSEL, count, 0);
		UpdateWindow(hWnd);
		ShowWindow(hWnd, SW_SHOWNORMAL);

		#ifdef	CONSOLETOFILE
			FILE		*file;
			file = fopen(CONSOLEFILE, "a+");
			fwrite(str, 1, strlen(str), file);
			fwrite("\n", 1, 2, file);
			fclose(file);
		#endif
	}
}

void nX_Console::SetProgress(char p)
{
	if (p == -1)
			SendMessage(GetDlgItem(hWnd, IDC_PROGRESS), PBM_SETPOS, (WPARAM)0, 0);
	else	SendMessage(GetDlgItem(hWnd, IDC_PROGRESS), PBM_SETPOS, (WPARAM)p, 0);
}

void nX_Console::Clear()
{
	SendDlgItemMessage(hWnd, IDC_CONSOLEOUTPUT, LB_RESETCONTENT, 0, 0);
}

nX_Console::nX_Console()
{
//	ruler = this;
	SendMessage(GetDlgItem(hWnd, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100));

	#ifdef	CONSOLETOFILE
		FILE		*file;
		file = fopen(CONSOLEFILE, "w");
		strcpy(str, "nXng CONSOLE Log to file...\n\n");
		fwrite(str, 1, strlen(str), file);
		fclose(file);
	#endif
}

nX_Console::~nX_Console()
{
	DeleteObject(bgBrush);
	DeleteObject(lbbgBrush);
	if (active)
		DestroyConsole();
}
