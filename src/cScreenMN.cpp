	/*----------------------------------------------------------------------------

		nXv9 by xBaRr/Syndrome 2000
			(nXng) by xBaRr/Syndrome alias Emmanuel JULIEN
			All rights reserved 2000.

		Part of nXng.
		mailto://xbarr@pingoo.com
		http://xbarr.planet-d.net

		cScreen implementation file

	----------------------------------------------------------------------------*/


	#include	<ddraw.h>
	#include	<d3d.h>
	#include	"nXng/nX_console.h"
	#include	"nXng/cScreenMN.h"
	#include	"resource.h"
	static		char	str[256];


/*
	ScreenMN::ScreenMN()
	--------------------
	Setup default options for ScreenMN
*/


ScreenMN::ScreenMN(HINSTANCE hInst)
{

	memset(this, 0, sizeof(ScreenMN));

	// system
	hInstance = hInst;
	hAccel = LoadAccelerators(hInstance, "ACCEL");

	// setup default D3D options
	bilinear = true;
	dither = true;

	// setup default rendering options
//	renderer = nX_RSOFT;
	bWindowed = true;
	bReady = false;

	referencewidth = 640;
	referenceheight = 480;
	referencebpp = 32;

	// fill default texture flag rating
	TF_rating[0] = 0;
	TF_rating[1] = 1;
	TF_rating[2] = 2;
	TF_rating[3] = 3;
	TF_rating[4] = 4;
	TF_rating[5] = 5;
	TF_rating[6] = 6;
	TF_rating[7] = 7;

	ATF_rating[0] = 0;
	ATF_rating[1] = 1;
	ATF_rating[2] = 2;
	ATF_rating[3] = 3;
	ATF_rating[4] = 4;

	ZB_rating[ZB_16] = 1;
	ZB_rating[ZB_24] = 2;
	ZB_rating[ZB_32] = 3;
	ZB_rating[ZB_X] = 4;

	// Default Free Ride Speed
	RideRatio = 1;

}


/*
	ScreenMN::~ScreenMN()
	---------------------
	Free ScreenMN
*/


ScreenMN::~ScreenMN()
{

	long	c, c2;

	// Free all boards...

	for (c = 0; c < nbboard; c++)
	{
		SelectBoard((unsigned char)c);
		if (cddboard->name)
			delete (cddboard->name);
		cddboard->name = NULL;

		// Video modes are static table...

		// Free devices

		for (c2 = 0; c2 < cddboard->nbdevice; c2++)
		{
			if (cddboard->device[c2].name)
				delete (cddboard->device[c2].name);
			cddboard->device[c2].name = NULL;
		}

	}

	if (this->tmpMsg)
		delete (tmpMsg);

}


/*
	ScreenMN::SetConsole(nX_Console *)
	----------------------------------
	Set nX_Console for Screen
*/


void ScreenMN::SetConsole(nX_Console *c)
{
	console = c;
}


/*
	ScreenMN::Verbose(char *)
	-------------------------
	Display message in nX_Console if any
*/


void ScreenMN::Verbose(char *s)
{
	if (console)
		console->Print(s);
}


/*
	ScreenMN::SetPhysicResolution()
	-------------------------------
	Set physic resolution and calculate rendering ratio
*/


void ScreenMN::SetPhysicResolution(unsigned short uw, unsigned short uh, unsigned char ub)
{

	width = uw;
	height = uh;
	vmbpp = ub;

	wcoef = float ((float)width / referencewidth);
	hcoef = float ((float)height / referenceheight);

	prcViewportRect.lX1 = 0;
	prcViewportRect.lY1 = 0;
	prcViewportRect.lX2 = width;
	prcViewportRect.lY2 = height;

}


/*
	static DDEnumCallback()
	-----------------------
	Retrieve all installed DD board
*/


static int WINAPI DDEnumCallback(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{

	ScreenMN	*calling = (ScreenMN *)lpContext;

	if (lpGUID)
		memcpy(&calling->board[calling->nbboard].GUID, lpGUID, sizeof(GUID));
	calling->nbboard++;

	if (calling->nbboard < DDRAW_Maxboard)
			return DDENUMRET_OK;
	else	{	calling->Verbose("WHAT'S THE FAAK?!: DirectDraw board max limit reached!"); return DDENUMRET_CANCEL;	}

}


/*
	static DeviceEnumCallback()
	---------------------------
	Enumerate all available devices in a given board
*/


static HRESULT WINAPI DeviceEnumCallback(	LPSTR strDesc,
											LPSTR strName,
											D3DDEVICEDESC7 *pDesc,
											VOID *pCalling	)
{

	ScreenMN	*calling = (ScreenMN *)pCalling;
	calling->cd3ddevice = &calling->cddboard->device[calling->cddboard->nbdevice];
	D3DDevice	*ldevice = calling->cd3ddevice;

	if ( pDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION )
	{
		// we don't need D3D software devices
		calling->cddboard->cdevice = calling->cddboard->nbdevice;	
		ldevice->software = false;
	}
	else
		return D3DENUMRET_OK;

	// save the GUID
	memcpy(&ldevice->dGUID, &pDesc->deviceGUID, sizeof(GUID));

	// get the name
	ldevice->name = new char[strlen(strName) + 1];
	strcpy(ldevice->name, strName);

	// the rendering bpp the device support
	
	ldevice->renderbpp[0] = (pDesc->dwDeviceRenderBitDepth & DDBD_8) ? true : false;
	ldevice->renderbpp[1] = (pDesc->dwDeviceRenderBitDepth & DDBD_16) ? true : false;
	ldevice->renderbpp[2] = (pDesc->dwDeviceRenderBitDepth & DDBD_24) ? true : false;
	ldevice->renderbpp[3] = (pDesc->dwDeviceRenderBitDepth & DDBD_32) ? true : false;

	// supported texture size
	ldevice->minmapw = (unsigned short)pDesc->dwMinTextureWidth;
	ldevice->minmaph = (unsigned short)pDesc->dwMinTextureHeight;
	ldevice->maxmapw = (unsigned short)pDesc->dwMaxTextureWidth;
	ldevice->maxmaph = (unsigned short)pDesc->dwMaxTextureHeight;
	if (!ldevice->minmapw || !ldevice->minmaph)
		calling->Verbose("      - D3D drivers for this device seems very old, UPGRADE!,");

	if (!ldevice->minmapw) ldevice->minmapw = 1;
	if (!ldevice->minmaph) ldevice->minmaph = 1;
	if (!ldevice->maxmapw) ldevice->maxmapw = 256;
	if (!ldevice->maxmaph) ldevice->maxmaph = 256;

	// multipass device?
	ldevice->nbstage = (unsigned char)pDesc->wMaxTextureBlendStages;
	ldevice->mxnbmap = (unsigned char)pDesc->wMaxSimultaneousTextures;
	if (ldevice->mxnbmap > 1)
			calling->Verbose("    - hardware device supports multitexturing,");
	else	calling->Verbose("    - INFO: switched to single-texture rasterizer,");
	if (ldevice->nbstage > 1)
		calling->Verbose("    - hardware device can blend over multiple stages,");

	// do fog table support range?
	if (pDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGRANGE)
		ldevice->fogrange = true; else ldevice->fogrange = false;
	if (ldevice->fogrange == 1)
		calling->Verbose("    - hardware device can perform range based fog,");

	// device added
	calling->cddboard->nbdevice++;

	// make sure not to accept too much devices!
	if (calling->cddboard->nbdevice < DDRAW_MaxDevice)
			return D3DENUMRET_OK;
	else	return D3DENUMRET_CANCEL;

}

/*
	static EnumModesCallback()
	--------------------------
	Enumerate all available display modes for current board
*/


static HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC2 lpddsd, LPVOID lpContext)
{

	ScreenMN	*calling = (ScreenMN *)lpContext;
	VideoMode	*vm = &calling->cddboard->vm[calling->cddboard->nbvm];

	if ( calling->cddboard->nbvm < DDRAW_Maxvmode)
	{
		vm->width = (unsigned short)lpddsd->dwWidth;
		vm->height = (unsigned short)lpddsd->dwHeight;
		vm->pitch = lpddsd->lPitch;
		vm->bpp = (unsigned char)lpddsd->ddpfPixelFormat.dwRGBBitCount;

		if (vm->bpp != 8)
		{
			wsprintf(vm->desc, "%d: %dx%dx%d", calling->cddboard->nbvm, vm->width, vm->height, vm->bpp);
			calling->cddboard->nbvm++;
		}
	}

	// make sure we do not accept too much video modes!
	if (calling->cddboard->nbvm < DDRAW_Maxvmode)
		return DDENUMRET_OK;
	else
		return DDENUMRET_CANCEL;

}


/*
	ScreenMN::BPP2fBPP()
	--------------------
	Returns flag corresponding to selected BPP
*/


char ScreenMN::BPP2fBPP(char bpp)
{
	if (bpp == 8)
		return	0;
	else if (bpp == 16)
		return	1;
	else if (bpp == 24)
		return 2;
	else
		return 3;
};


/*
	ScreenMN::SelectBoard()
	-----------------------
	Set current DD board adress in cddboard
*/


void	ScreenMN::SelectBoard(unsigned char i)
{

	if (i >= nbboard)
	{
		cboard = -1;
		cddboard = NULL;
	}
	else
	{
		cboard = i;
		cddboard = &board[i];
	}
}


/*
	ScreenMN::SelectDevice()
	------------------------
	Set current D3D device adress for current board in cd3ddevice
*/


void	ScreenMN::SelectDevice(unsigned char i)
{
	if (cddboard)
		if (i < cddboard->nbdevice)
			cd3ddevice = &cddboard->device[i];
	else	cd3ddevice = NULL;
}


/*
	ScreenMN::QueryBoards()
	-----------------------
	Enumerate all installed DD boards in computer
*/


CSC_MSG ScreenMN::QueryBoards()
{
	unsigned char	c;
	CSC_MSG			errmsg;

	// Enumerate DirectDraw boards
	nbboard = 0;
	if (DirectDrawEnumerate(&DDEnumCallback, this) != DD_OK)
		return	CSC_WRONGDX;
	wsprintf(str, "Enumeration found %d DirectDraw adapter(s):", nbboard);
	this->Verbose(str);

	// Retrieve caps for each installed boards
	DDDEVICEIDENTIFIER2	dddi;
	DDSURFACEDESC2		ddsd;
	DDCAPS				driverCaps;

	for (c = 0; c < nbboard; c++)
	{
		// set current board
		SelectBoard(c);

		// initialize DirectDraw
		errmsg = DD_Init();
		if (errmsg != CSC_NOERROR)
			return errmsg;

		// get desktop BPP
		if (c == NULL)
		{
			if (pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL) != DD_OK)
				return CSC_ERRCOOPLVL;

			memset(&ddsd, 0, sizeof(ddsd));
			ddsd.dwSize				=	sizeof(ddsd);
			ddsd.dwFlags			=	DDSD_CAPS;
			ddsd.ddsCaps.dwCaps		=	DDSCAPS_PRIMARYSURFACE;

			if (pDD->CreateSurface(&ddsd, &pDDSPrimary, NULL) != DD_OK)
				return CSC_ERRCREATESURF;

			pDDSPrimary->GetSurfaceDesc(&ddsd);
			desktopbpp = (unsigned char)ddsd.ddpfPixelFormat.dwRGBBitCount;
			pDDSPrimary->Release();
		}

		// device infos
		if (pDD->GetDeviceIdentifier(&dddi, NULL) != DD_OK)
			return CSC_ERRGETDEVID;
		cddboard->name = new char[strlen(dddi.szDescription) + 1];
		strcpy(cddboard->name, dddi.szDescription);

		wsprintf(str, "    DEVICE n°%d is '%s'", c, cddboard->name);
		Verbose(str);

		// get board caps
		memset(&driverCaps, 0, sizeof(DDCAPS));
		driverCaps.dwSize = sizeof(DDCAPS);

		if (pDD->GetCaps(&driverCaps, NULL) != DD_OK)
			return CSC_ERRGETCAPS;
		if (driverCaps.dwCaps & DDCAPS_3D)						// is the board D3D?
			cddboard->d3d = true;
		if (driverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
			cddboard->windowed = true;							// can it render windowed?

		if (cddboard->d3d)
				Verbose("    - supports D3D,");
		else	Verbose("    - has no D3D support,");
		if (cddboard->windowed)
				Verbose("    - can render windowed,");
		else	Verbose("    - cannot render windowed,");

		// get D3D caps
		cddboard->nbdevice = 0;
		cddboard->cdevice = 0;

		if (cddboard->d3d)
		{
			if (pDD->QueryInterface(IID_IDirect3D7, (void **)&pD3D) != DD_OK)
				return CSC_ERRQUERYD3D;

			if (pD3D->EnumDevices(DeviceEnumCallback, this) != DD_OK)
				return CSC_ERRENUMDEV;

			// get display modes
			memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);

			if (pDD->EnumDisplayModes(DDEDM_REFRESHRATES, &ddsd, this, EnumModesCallback) != DD_OK)
				return CSC_ERRENUMMODES;

			// we're done with this one...
			pD3D->Release();
			pD3D = NULL;
		}

		DD_Free();
		Verbose(" ");
	}

	SelectBoard(0);
	SelectDevice(0);

	// that's all folks
	return	CSC_NOERROR;

}
