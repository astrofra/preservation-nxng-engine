	/*----------------------------------------------------------------------------

		nXv9 by xBaRr/Syndrome 2000
			(nXs3) by xBaRr/Syndrome alias Emmanuel JULIEN
			All rights reserved 2000.

		Part of nXng.
		mailto://xbarr@pingoo.com
		http://xbarr.planet-d.net

		nXng GUI implementation file

	----------------------------------------------------------------------------*/


	#include	<windows.h>
	#include	<windowsx.h>
	#include	<ddraw.h>
	#include	<d3d.h>
	#include	<commctrl.h>
	#include	<winuser.h>
	#include	<stdio.h>

	#include	"resource.h"

	#include	"nXng/nX_Console.h"
	#include	"nXng/cScreenMN.h"
	#include	"nXng/nX_GUI.h"
	#include	"IO/cqString.h"

	static		nX_GUI			*ruler;
	static		char			str[256];
	static		char			lws[8192];
	static		char			buff[2048];
	static		TVINSERTSTRUCT	tvstruct;

	static	    RECT rcItem;			// bounding rectangle of item 
	static	    DWORD dwLevel;			// heading level of item 
	static	    DWORD dwIndent;			// amount that child items are indented  

	#define		GUI_R	150
	#define		GUI_G	177
	#define		GUI_B	206


struct	GUI_TVnode
{
	bool		info;								// infotext?
	HTREEITEM	*hti;
	GUI_TVnode	*parent, *listnext;
};

	static		GUI_TVnode		nodeWork[4096];		// max scene in TV


static bool isLWS(char *s)
{
	char		ext[5];
	qString		qS;

	strcpy(ext, &s[strlen(s) - 4]);
	qS.qstrlwr(ext);
	if (!qS.qstrcmp(ext, ".lws"))
		return true;
	return false;
}

static void InsertSequence(HTREEITEM ins)
{
	tvstruct.hParent = NULL;
	tvstruct.hInsertAfter = ins;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvstruct.item.iSelectedImage = tvstruct.item.iImage = ruler->hISEQ;
	tvstruct.item.pszText = "<SEQUENCE>";
	ruler->tvPar = TreeView_InsertItem(GetDlgItem(ruler->hWnd, IDC_TREE), &tvstruct);

	tvstruct.hParent = ruler->tvPar;
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvstruct.item.iSelectedImage = tvstruct.item.iImage = ruler->hISEQ;
	tvstruct.item.pszText = "<LWS/XM>";
	ruler->tvCur = TreeView_InsertItem(GetDlgItem(ruler->hWnd, IDC_TREE), &tvstruct);

	TreeView_Expand(GetDlgItem(ruler->hWnd, IDC_TREE), ruler->tvPar, TVE_EXPAND);
}

static void InsertItem(HTREEITEM par, HTREEITEM cur, char *txt)
{
	tvstruct.hParent = par;
	tvstruct.hInsertAfter = cur;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvstruct.item.pszText = txt;
	if (isLWS(txt))
			tvstruct.item.iSelectedImage = tvstruct.item.iImage = ruler->hILWS;
	else	tvstruct.item.iSelectedImage = tvstruct.item.iImage = ruler->hIXMP;
	ruler->tvCur = TreeView_InsertItem(GetDlgItem(ruler->hWnd, IDC_TREE), &tvstruct);
}

static void InitTreeBaseItems()
{
	TreeView_DeleteAllItems(GetDlgItem(ruler->hWnd, IDC_TREE));

	// Initialize default tree view
	tvstruct.hParent = TVI_ROOT;
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvstruct.item.iSelectedImage = tvstruct.item.iImage = ruler->hISEQ;
	tvstruct.item.pszText = "<SEQUENCE> Right click to add SEQUENCE to playlist";
	ruler->htRoot = TreeView_InsertItem(GetDlgItem(ruler->hWnd, IDC_TREE), &tvstruct);
	tvstruct.hParent = ruler->htRoot;
	tvstruct.hInsertAfter = TVI_LAST;
	tvstruct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvstruct.item.iSelectedImage = tvstruct.item.iImage = ruler->hISEQ;
	tvstruct.item.pszText = "<LWS/XM> Double click to add LWS/XM to sequence";
	ruler->tvCur = TreeView_InsertItem(GetDlgItem(ruler->hWnd, IDC_TREE), &tvstruct);

	TreeView_Expand(GetDlgItem(ruler->hWnd, IDC_TREE), ruler->htRoot, TVE_EXPAND);
	TreeView_Select(GetDlgItem(ruler->hWnd, IDC_TREE), ruler->tvCur, TVGN_CARET);
}

static int CALLBACK GUIProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	long			c, c2, s;
	NMHDR			*v;
	unsigned long	cnt;
	OPENFILENAME	ofn;
	char			tmp[256];

    switch (uMsg)
    {

		case WM_INITDIALOG:
			CheckDlgButton(hDlg, IDC_FULLSCREEN, BST_UNCHECKED);
			s = 0;

			// fill boards
			for (c = 0; c < ruler->Screen->nbboard; c++)
			{
				wsprintf(str, "%d: %s", c, ruler->Screen->board[c].name);
				SendDlgItemMessage(hDlg, IDC_BOARD, CB_ADDSTRING, 0, (LPARAM)str);
			}
			SendDlgItemMessage(hDlg, IDC_BOARD, CB_SETCURSEL, 0, 0);

			// fill VMS
			for (c = 0; c < ruler->Screen->cddboard->nbvm; c++)
			{
				SendDlgItemMessage(hDlg, IDC_VMODE, CB_ADDSTRING, 0, (LPARAM)ruler->Screen->cddboard->vm[c].desc);
				if (640 == ruler->Screen->cddboard->vm[c].width &&
					480 == ruler->Screen->cddboard->vm[c].height &&
					16 == ruler->Screen->cddboard->vm[c].bpp)
					s = c;
			}
			SendDlgItemMessage(hDlg, IDC_VMODE, CB_SETCURSEL, s, 0);
			break;

		case WM_CTLCOLORLISTBOX:
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, 0xffeedd);
			return long(ruler->lbbgBrush);

		case WM_CTLCOLORBTN:
			SetBkColor((HDC)wParam, GUI_B * 65536 + GUI_G * 256 + GUI_R);
			return long(ruler->bgBrush);

		case WM_CTLCOLORDLG:
			SetBkColor((HDC)wParam, GUI_B * 65536 + GUI_G * 256 + GUI_R);
			return long(ruler->bgBrush);

		case WM_CTLCOLORSTATIC:
			SetBkColor((HDC)wParam, GUI_B * 65536 + GUI_G * 256 + GUI_R);
			return long(ruler->bgBrush);

		case WM_NOTIFY:

			v = (NMHDR *)lParam;
			if (v->idFrom == IDC_TREE)
			{
				switch (v->code)
				{
					case NM_RCLICK:
						ruler->tvCur = TreeView_GetSelection(GetDlgItem(hDlg, IDC_TREE));
						if (!TreeView_GetParent(GetDlgItem(ruler->hWnd, IDC_TREE), ruler->tvCur))
								InsertSequence(ruler->tvCur);
						else	InsertSequence(TVI_ROOT);
						break;

					case NM_DBLCLK:

						// perform insertion
						lws[0] = 0;
						cnt = 0;

						// If we have parent here then add file mode
						ruler->tvCur = TreeView_GetSelection(GetDlgItem(hDlg, IDC_TREE));

						if (ruler->tvPar = TreeView_GetParent(GetDlgItem(ruler->hWnd, IDC_TREE), ruler->tvCur))
						{
							strcpy(buff, "All supported Items (LWS & BASS Audio)");
							cnt += strlen("All supported Items (LWS & BASS Audio)") + 1;
							strcpy(&buff[cnt], "*.LWS;*.WAV;*.MP?;*.XM;*.MOD;*.IT;*.S3M");
							cnt += strlen("*.LWS;*.WAV;*.MP?;*.XM;*.MOD;*.IT;*.S3M") + 1;
							strcpy(&buff[cnt], "LIGHTWAVE v5.6c Ascii scene");
							cnt += strlen("LIGHTWAVE v5.6c Ascii scene") + 1;
							strcpy(&buff[cnt], "*.LWS");
							cnt += strlen("*.LWS") + 1;
							strcpy(&buff[cnt], "BASS Audio filetype");
							cnt += strlen("BASS Audio filetype") + 1;
							strcpy(&buff[cnt], "*.WAV;*.MP?;*.XM;*.MOD;*.IT;*.S3M");
							cnt += strlen("*.WAV;*.MP?;*.XM;*.MOD;*.IT;*.S3M") + 1;
							strcpy(&buff[cnt], "All files");
							cnt += strlen("All files") + 1;
							strcpy(&buff[cnt], "*.*");
							cnt += strlen("*.*") + 1;
							buff[cnt] = 0;

							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hwndOwner = hDlg;
							ofn.hInstance = ruler->hInstance;
							ofn.lpstrFilter = buff;
							ofn.lpstrCustomFilter = (LPSTR)NULL;
							ofn.nMaxCustFilter = 0L;
							ofn.nFilterIndex = 1L;
							ofn.lpstrFile = lws;
							ofn.nMaxFile = 8192;
							ofn.lpstrFileTitle = NULL;
							ofn.nMaxFileTitle = NULL;
							ofn.lpstrInitialDir = NULL;
							ofn.lpstrTitle = "Select item to add";
							ofn.nFileOffset = 0;
							ofn.nFileExtension = 0;
							ofn.lpstrDefExt = NULL;
							ofn.lCustData = 0;
							ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;// | OFN_ALLOWMULTISELECT;

							if (GetOpenFileName(&ofn))
								InsertItem(ruler->tvPar, ruler->tvCur, lws);
						}
						break;
				}
			}
			break;

        case WM_COMMAND:
        	switch (LOWORD(wParam))
        	{
				case ACC_FULLSCREEN:
					if (IsDlgButtonChecked(hDlg, IDC_FULLSCREEN) == BST_UNCHECKED)
							CheckDlgButton(hDlg, IDC_FULLSCREEN, BST_CHECKED);
					else	CheckDlgButton(hDlg, IDC_FULLSCREEN, BST_UNCHECKED);
					break;
				case ACC_SHADE:
					if (IsDlgButtonChecked(hDlg, IDC_SHADE) == BST_UNCHECKED)
							CheckDlgButton(hDlg, IDC_SHADE, BST_CHECKED);
					else	CheckDlgButton(hDlg, IDC_SHADE, BST_UNCHECKED);
					break;

				case ACC_O:	
					PostMessage(hDlg, WM_COMMAND, IDC_LOADPL, NULL);
					break;
				case ACC_S:
					PostMessage(hDlg, WM_COMMAND, IDC_SAVEPL, NULL);
					break;
				case ACC_C:
					InitTreeBaseItems();
					break;
				case ACC_QUIT:
					ruler->command = GUI_QUIT;
					break;

				case ACC_SUPP:
					ruler->tvCur = TreeView_GetSelection(GetDlgItem(hDlg, IDC_TREE));
					if (ruler->tvCur == ruler->htRoot)
						break;
					if (ruler->tvCur == TreeView_GetChild(GetDlgItem(hDlg, IDC_TREE), ruler->htRoot))
						break;

					ruler->tvPar = TreeView_GetParent(GetDlgItem(hDlg, IDC_TREE), ruler->tvCur);
					TreeView_DeleteItem(GetDlgItem(hDlg, IDC_TREE), ruler->tvCur);

					if (!TreeView_GetChild(GetDlgItem(hDlg, IDC_TREE), ruler->tvPar))
						TreeView_DeleteItem(GetDlgItem(hDlg, IDC_TREE), ruler->tvPar);
					break;

				case ACC_REPLAY:
					PostMessage(hDlg, WM_COMMAND, IDC_REPLAY, NULL);
					break;
			}
			switch (wParam)
			{
				case IDC_CLPL:
					InitTreeBaseItems();
					break;

				case IDC_LOADPL:
					lws[0] = 0;
					cnt = 0;

					strcpy(buff, "nXng[2.0] Sequence playlist");
					cnt += strlen("nXng[2.0] Sequence playlist") + 1;
					strcpy(&buff[cnt], "*.SPL");
					cnt += strlen("*.SPL") + 1;
					strcpy(&buff[cnt], "All files");
					cnt += strlen("All files") + 1;
					strcpy(&buff[cnt], "*.*");
					cnt += strlen("*.*") + 1;
					buff[cnt] = 0;

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.hInstance = ruler->hInstance;
					ofn.lpstrFilter = buff;
					ofn.lpstrCustomFilter = (LPSTR)NULL;
					ofn.nMaxCustFilter = 0L;
					ofn.nFilterIndex = 1L;
					ofn.lpstrFile = lws;
					ofn.nMaxFile = 8192;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = NULL;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = "Load SEQUENCE playlist";
					ofn.nFileOffset = 0;
					ofn.nFileExtension = 0;
					ofn.lpstrDefExt = NULL;
					ofn.lCustData = 0;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;// | OFN_ALLOWMULTISELECT;

					if (GetOpenFileName(&ofn))
						ruler->LoadTree(ofn.lpstrFile);
					break;

				case IDC_SAVEPL:
					strcpy(lws, "default.spl");
					cnt = 0;

					strcpy(buff, "nXng[2.0] Sequence playlist");
					cnt += strlen("nXng[2.0] Sequence playlist") + 1;
					strcpy(&buff[cnt], "*.SPL");
					cnt += strlen("*.SPL") + 1;
					strcpy(&buff[cnt], "All files");
					cnt += strlen("All files") + 1;
					strcpy(&buff[cnt], "*.*");
					cnt += strlen("*.*") + 1;
					buff[cnt] = 0;

					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.hInstance = ruler->hInstance;
					ofn.lpstrFilter = buff;
					ofn.lpstrCustomFilter = (LPSTR)NULL;
					ofn.nMaxCustFilter = 0L;
					ofn.nFilterIndex = 1L;
					ofn.lpstrFile = lws;
					ofn.nMaxFile = 8192;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = NULL;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = "Save SEQUENCE playlist";
					ofn.nFileOffset = 0;
					ofn.nFileExtension = 0;
					ofn.lpstrDefExt = NULL;
					ofn.lCustData = 0;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER;// | OFN_ALLOWMULTISELECT;

					if (GetSaveFileName(&ofn))
						ruler->SaveTree(ofn.lpstrFile);
					break;

				case IDC_QUIT:
					ruler->command = GUI_QUIT;
					break;

				case IDC_REPLAY:
					if (!ruler->Screen->cddboard->nbvm)
					{
						ruler->console->Print("    ERROR: Invalid video mode!");
						break;
					}

					ruler->command = GUI_REPLAY;
					SendMessage(GetDlgItem(hDlg, IDC_VMODE), CB_GETLBTEXT, SendMessage(GetDlgItem(hDlg, IDC_VMODE), CB_GETCURSEL, NULL, NULL), (LPARAM)str);

					// nbvm
					c = 0;
					while (str[c] != ':')
						{	tmp[c] = str[c]; c++;	}
					tmp[c] = 0;
					c2 = (long)atof(tmp);

					if ( (c2 < 0) || (c2 >= ruler->Screen->cddboard->nbvm) )
					{
						ruler->console->Print("    ERROR: Invalid video mode!");
						break;
					}

					ruler->Screen->SetPhysicResolution(ruler->Screen->cddboard->vm[c2].width, ruler->Screen->cddboard->vm[c2].height, ruler->Screen->cddboard->vm[c2].bpp);
					break;
			}
			break;
	}

	return false;

}

bool nX_GUI::LoadTree(char *file)
{

	qString		qS;

	wsprintf(str, "Loading tree from '%s'", file);
	console->Print(str);

	FILE	*h = fopen(file, "r");
	if (!h)
	{
		console->Print("    ERROR: Cannot open file!");
		return false;
	}

	fscanf(h, "%s\n", str);
	if (strcmp(str, "nXSQPLH"))
	{
		console->Print("    ERROR: Incorrect file format!");
		return false;
	}

	InitTreeBaseItems();
	HTREEITEM	current = htRoot;
	bool	baseseq = false;

	while (true)
	{
		if (fscanf(h, "%s\n", str) == EOF)
			break;
		qS.qstrrpl(str, '|', ' ');

		if (!strcmp(str, "[SEQUENCE]"))
		{
			if (baseseq)
			{		InsertSequence(TVI_LAST); current = tvPar;	}
			else	baseseq = true;
		}

		else	InsertItem(current, TVI_LAST, str);
	}

	fclose(h);

	console->Print("Done...");
	console->Print(" ");
	return true;

}

bool nX_GUI::SaveTree(char *file)
{

	char		tmp[256];
	qString		qS;

	wsprintf(str, "Saving tree to '%s'", file);
	console->Print(str);

	FILE	*h = fopen(file, "w");
	if (!h)
	{
		console->Print("    ERROR: Cannot create file!");
		return false;
	}
	nX_TVitem		item;

	fprintf(h, "nXSQPLH\n");
	InitItemRetrieval();
	while (PoolItemRetrieval(&item))
	{
		if (item.sequence)
			fprintf(h, "[SEQUENCE]\n");
		strcpy(tmp, item.txt);

		qS.qstrrpl(tmp, ' ', '|');
		fprintf(h,"%s\n", tmp);
	}
	fclose(h);

	console->Print("Done...");
	console->Print(" ");
	return true;

}

void nX_GUI::SetConsole(nX_Console *c)
{
	console = c;
}

void nX_GUI::CreateGUI(HINSTANCE hInst, HWND ephWnd)
{

	hInstance = hInst;
	phWnd = ephWnd;
    bgBrush = CreateSolidBrush(GUI_B * 65536 + GUI_G * 256 + GUI_R);
	lbbgBrush = CreateSolidBrush(GUI_B * 65536 + GUI_G * 256 + GUI_R);

	hWnd = CreateDialog(hInstance, "nX_GUI", phWnd, GUIProc);
	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);

	command = GUI_NULL;

	// Create image list
	HIMAGELIST hIL = ImageList_Create(16, 16, ILC_COLOR, 3, 0);

	HBITMAP	tmp = LoadBitmap(hInstance, "TVLWS"); 
    hILWS = ImageList_Add(hIL, tmp, NULL);  
	DeleteObject(tmp);
	tmp = LoadBitmap(hInstance, "TVSEQ"); 
    hISEQ = ImageList_Add(hIL, tmp, NULL);  
	DeleteObject(tmp);
	tmp = LoadBitmap(hInstance, "TVXMP"); 
    hIXMP = ImageList_Add(hIL, tmp, NULL);  
	DeleteObject(tmp);
	TreeView_SetImageList(GetDlgItem(hWnd, IDC_TREE), hIL, TVSIL_NORMAL); 

	// Load previous playlist if any
	char	file[256];

	wsprintf(file, "%s%s", basedir, "\\gui.spl");
	if (!LoadTree(file))
		InitTreeBaseItems();
}

void nX_GUI::InitItemRetrieval()
{
	xpRoot = htRoot;
	xpChild = NULL;
	seqb = true;
}

bool nX_GUI::PoolItemRetrieval(nX_TVitem *it)
{

	TVITEM	itdesc;
	itdesc.mask = TVIF_TEXT;
	itdesc.pszText = str;
	itdesc.cchTextMax = 256;
	it->txt = str;

	// Next item in sequence
	if (xpChild)
	{
		xpChild = TreeView_GetNextSibling(GetDlgItem(hWnd, IDC_TREE), xpChild);
		if (!xpChild)
		{
			xpRoot = TreeView_GetNextSibling(GetDlgItem(hWnd, IDC_TREE), xpRoot);
			if (!xpRoot)
				return false;			// end of list
			seqb = true;
		}
	}

	// Find first item in new sequence
	while (!xpChild)
	{
		// Get the <CHILD> add position
		xpChild = TreeView_GetChild(GetDlgItem(hWnd, IDC_TREE), xpRoot);

		// First item in sequence
		if (xpChild = TreeView_GetNextSibling(GetDlgItem(hWnd, IDC_TREE), xpChild))
			break;

		// We met an empty sequence... or got to the end of a previously begun one
		xpRoot = TreeView_GetNextSibling(GetDlgItem(hWnd, IDC_TREE), xpRoot);
		if (!xpRoot)
			return false;			// end of list
		seqb = true;
	}

	// Return infos about current item
	itdesc.hItem = xpChild;
	TreeView_GetItem(GetDlgItem(hWnd, IDC_TREE), &itdesc);
	it->sequence = seqb;
	it->lws = isLWS(str);
	seqb = false;
	return true;

}

unsigned char	nX_GUI::Pool()
{
	MSG	msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!TranslateAccelerator(hWnd, Accel, &msg))
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}

	return command;
}

void nX_GUI::DestroyGUI()
{
	char	file[256];
	if (hWnd)
	{
		wsprintf(file, "%s%s", basedir, "\\gui.spl");
		SaveTree(file);
		TreeView_DeleteAllItems(GetDlgItem(hWnd, IDC_TREE));
		DestroyWindow(hWnd);
	}
	hWnd = NULL;
}

nX_GUI::nX_GUI()
{
	ruler = this;
}

nX_GUI::~nX_GUI()
{
	DestroyGUI();
	DeleteObject(bgBrush);
	DeleteObject(lbbgBrush);
}
