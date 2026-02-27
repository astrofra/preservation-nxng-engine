	/*----------------------------------------------------------------------------

		nXv9 by xBaRr/Syndrome 2000
			(nXs3) by xBaRr/Syndrome alias Emmanuel JULIEN
			All rights reserved 2000.

		Part of nXng.
		mailto://xbarr@pingoo.com
		http://xbarr.planet-d.net

		Window implementation file

	----------------------------------------------------------------------------*/


	#define		NAME		"nXng[2.0] SYNDROME 3D.Engine"
	#define		TITLE		"nXng[2.0] SYNDROME 3D.Engine"
	#define		ALLOWMSG

	#include	<ddraw.h>
	#include	<d3d.h>
	#include	"resource.h"

	#include	"nXng/nX_console.h"
	#include	"nXng/cScreenMN.h"

	static		ScreenMN		*iScreen;
	static		char			str[1024];
	static		unsigned char	msgCode;
	static		CSC_MSG			CSC;


		// --------------------------------------------
		// WINDOW MANAGEMENT implementation source code
		// --------------------------------------------


static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    HRESULT         hRet;

    switch (msg)
    {
	   	case WM_DESTROY:
			iScreen->bReady = false;
			iScreen->AppState = APP_FINISH;
			break;

		case WM_ACTIVATE:
			iScreen->bActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
			break;

        case WM_COMMAND:

            // handle accelerated key commands
        	switch (LOWORD(wParam))
        	{
				case ACC_QUIT:
					iScreen->bReady = false;
					iScreen->AppState = APP_FINISH;
					msgCode = 2;
					break;
				case ACC_SPACE:
					msgCode = 1;			// SPACEBAR pressed!
					iScreen->TempMsg("Sequence <BREAK>", 10, iScreen->height - 15, 75);
					break;
				case ACC_FULLSCREEN:
				case IDM_TOGGLEFULLSCREEN:
                    if (iScreen->bReady)// && iScreen->board[iScreen->cboard].windowed)
                    {
                        iScreen->bWindowed =! iScreen->bWindowed;
						iScreen->bReady = false;

						if (!iScreen->bWindowed)
							GetWindowRect(hWnd, &iScreen->rcWindow);

						CSC = iScreen->ChangeD3DCoopLevel();
						if (CSC == CSC_NOERROR)
						{
	                        iScreen->bReady = true;
							iScreen->Verbose("Change D3D cooperative level OK");
						}
						else
						{
							iScreen->GetErrDesc(CSC, str);
							iScreen->Verbose("    ERROR: Screen::ChangeD3DCoopLevel();");
							iScreen->Verbose(str);
							iScreen->AppState = APP_FINISH;
						}
                    }
					break;

				case IDM_TOGGLESTATS:
					iScreen->bUseFrameReport = !iScreen->bUseFrameReport;
					break;
				case IDM_TOGGLERASTER:
					iScreen->bForceSingleRaster = !iScreen->bForceSingleRaster;
					if (iScreen->bForceSingleRaster)
							iScreen->TempMsg("Forcing use of SINGLE-TEXTURE rasterizer...", 10, iScreen->height - 15, 50);
					else	iScreen->TempMsg("Released SINGLE-TEXTURE rasterizer...", 10, iScreen->height - 15, 50);
					break;
				case IDM_FREERIDE:
					iScreen->bFreeRide = !iScreen->bFreeRide;
					if (iScreen->bFreeRide)
							iScreen->TempMsg("FreeRide mode: ON", 10, iScreen->height - 15, 50);
					else	iScreen->TempMsg("FreeRide mode: OFF", 10, iScreen->height - 15, 50);
					break;
				case IDM_TOGGLEBB:
					iScreen->bBoundingBox = !iScreen->bBoundingBox;
					if (iScreen->bBoundingBox)
							iScreen->TempMsg("Display Bounding Boxes mode: ON", 10, iScreen->height - 15, 50);
					else	iScreen->TempMsg("Display Bounding Boxes mode: OFF", 10, iScreen->height - 15, 50);
					break;
				case IDM_TOGGLEBLOOM:
					iScreen->bBloom = !iScreen->bBloom;
					if (iScreen->bBloom)
							iScreen->TempMsg("Bloom: ON", 10, iScreen->height - 15, 50);
					else	iScreen->TempMsg("Bloom: OFF", 10, iScreen->height - 15, 50);
					break;
				case ACC_ACCELERATE:
					iScreen->RideRatio *= 2.0;
					iScreen->TempMsg("FreeRide speed UP...", 10, iScreen->height - 15, 50);
					break;
				case ACC_DECCELERATE:
					iScreen->RideRatio /= 2.0;
					iScreen->TempMsg("FreeRide speed DOWN...", 10, iScreen->height - 15, 50);
					break;
			}
        	break;

        case WM_MOVE:

            if (iScreen->bReady && iScreen->bWindowed)
            {
                GetWindowRect(hWnd, &iScreen->rcWindow);
            	GetClientRect(hWnd, &iScreen->rcViewport);
            	GetClientRect(hWnd, &iScreen->rcScreen);
            	ClientToScreen(hWnd, (POINT*)&iScreen->rcScreen.left);
            	ClientToScreen(hWnd, (POINT*)&iScreen->rcScreen.right);
            }
            break;

        case WM_PAINT:
            if (iScreen->bWindowed && iScreen->bReady)
            {
                while (true)
                {
                    hRet = iScreen->pDDSPrimary->Blt(&iScreen->rcScreen, iScreen->pDDSBack,
											  &iScreen->rcViewport, DDBLT_WAIT,
                                              NULL);
                    if (hRet == DD_OK)
                        break;
					if (hRet == DDERR_SURFACELOST)
						iScreen->pDDSPrimary->Restore();
                }
            }
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);

}

//	Print()::display an on screen message -------------------------------------
///////////////////////////////////////////////////////////////////////////////

void ScreenMN::PrintInit(unsigned short iY)
{
#ifdef	ALLOWMSG
	if (!hWnd || bPrintInitDone)
		return;
	pDDSBack->GetDC(&dc);
	SelectObject(dc, font);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, 0x00ffffff);
	yPos = iY;
	bPrintInitDone = true;
#else
	return;
#endif
}

void ScreenMN::qPrint(char *str, unsigned short x)
{
#ifdef	ALLOWMSG
	if (bPrintInitDone)
	{
		TextOut(dc, x, yPos, str, strlen(str));
		yPos += 10;
	}
#else
	return;
#endif
}

void ScreenMN::PrintFree()
{
	if (!bPrintInitDone)
		return;
	pDDSBack->ReleaseDC(dc);
	bPrintInitDone = false;
}

void ScreenMN::Print(char *str, unsigned short x, unsigned short y)
{
#ifdef	ALLOWMSG
	PrintInit(0);
	if (bPrintInitDone)
		TextOut(dc, x, y, str, strlen(str));
	PrintFree();
#else
	return;
#endif
}

void ScreenMN::TempMsg(char *str, unsigned short x, unsigned short y, unsigned short delay)
{
#ifdef	ALLOWMSG
	if (tmpMsg)	delete tmpMsg;
	tmpMsg = new char[strlen(str) + 1];
	strcpy(tmpMsg, str);
	tmpMsgCounter = delay;
	tmpX = x; tmpY = y;
#else
	return;
#endif
}

void ScreenMN::DisplayTempMsg()
{
#ifdef	ALLOWMSG
	if (!tmpMsg)
		return;

	tmpMsgCounter--;
	if (tmpMsgCounter == 0)
	{	delete(tmpMsg);
		tmpMsg = NULL;	}
	else
		Print(tmpMsg, tmpX, tmpY);
#else
	return;
#endif
}

//	Message()::a custom pick message procedure for linear RUNTIME -------------
///////////////////////////////////////////////////////////////////////////////

unsigned long ScreenMN::Message()
{
	MSG		msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!TranslateAccelerator(hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}

	unsigned long	ret = msgCode;
	msgCode = 0;
	return ret;
}

// CREATE RENDER WINDOW
///////////////////////

CSC_MSG ScreenMN::OpenRenderWindow()
{

	iScreen = this;
	WNDCLASS wc;

	// register the Window Class
	wc.lpszClassName = NAME;
	wc.lpfnWndProc = MainWndProc;
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_BYTEALIGNCLIENT | CS_OWNDC;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE("IDI_ICON1"));
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	RegisterClass(&wc);

    // create and Show the Main Window
	hWnd = CreateWindowEx(0,//WS_EX_TOPMOST,
			NAME,
			TITLE,
			WS_OVERLAPPED,
			GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2 - GetSystemMetrics(SM_CXSIZEFRAME),
			GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2 - GetSystemMetrics(SM_CYSIZEFRAME) - GetSystemMetrics(SM_CYMENU) / 2,
			width + GetSystemMetrics(SM_CXSIZEFRAME),
			height + GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYMENU),
			NULL,
			NULL,
			hInstance,
			NULL);
	if (!hWnd)
		return CSC_ERRCREAWIN;

	ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
    GetWindowRect(hWnd, &rcWindow);
	ShowCursor(false);

	// Some things we might need...
	font = CreateFont(12, 7, 0, 0, FW_LIGHT, true, false, false, ANSI_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Arial");
	AppState = APP_RUNNING;

	// well, the callback will need that!
	return CSC_NOERROR;

}

// CLOSE RENDER WINDOW
//////////////////////

void ScreenMN::CloseRenderWindow()
{
	ShowCursor(true);
	Message();
	DestroyWindow(hWnd);
	DeleteObject(font);
	if (tmpMsg)	delete(tmpMsg);
	tmpMsg = NULL;
	hWnd = NULL;
}
