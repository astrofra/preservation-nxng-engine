	/*----------------------------------------------------------------------------

		nXv9 by xBaRr/Syndrome 2000
			(nXs3) by xBaRr/Syndrome alias Emmanuel JULIEN
			All rights reserved 2000.

		Part of nXng.
		mailto://xbarr@pingoo.com
		http://xbarr.planet-d.net

		nXng Scene post-processor BLOOM implementation file

	----------------------------------------------------------------------------*/


	#include	<math.h>
	#include	"nX_HeaderList.h"


#ifdef	nX_ENABLEBLOOM
#define	OFF7	6

	#include	"IO/BitConvertIO.h"

void	nX::PostProcessBloom()
{

	BitIO	iBit;

	// Build a quick Bloom polygon list from rendered surface
	DDSURFACEDESC2		ddcaps;
	unsigned char		r[1200], g[1200], b[1200];

	ddcaps.dwSize = sizeof(DDSURFACEDESC2);
	screen->pDDSBack->Lock(NULL, &ddcaps, DDLOCK_WAIT, NULL);

	unsigned long		rshift = iBit.getshiftbit(ddcaps.ddpfPixelFormat.dwRBitMask);
	unsigned long		gshift = iBit.getshiftbit(ddcaps.ddpfPixelFormat.dwGBitMask);
	unsigned long		bshift = iBit.getshiftbit(ddcaps.ddpfPixelFormat.dwBBitMask);
	unsigned long		rmask = ddcaps.ddpfPixelFormat.dwRBitMask;
	unsigned long		gmask = ddcaps.ddpfPixelFormat.dwGBitMask;
	unsigned long		bmask = ddcaps.ddpfPixelFormat.dwBBitMask;

	char				bpp = (char)ddcaps.ddpfPixelFormat.dwRGBBitCount;
	unsigned long		color;

	unsigned long		cx, cy, tileindex = 0;
	char				*pAdr = (char *)ddcaps.lpSurface;

	unsigned long		width = ddcaps.dwWidth, height = ddcaps.dwHeight;

	for (cy = 0; cy < height; cy += 16)
	{
		for (cx = 0; cx < width; cx += 16)
		{

			switch (bpp)
			{
				case	16:
					color = *(unsigned short *)pAdr;
					r[tileindex] = (unsigned char)((color & rmask) >> rshift);
					g[tileindex] = (unsigned char)((color & gmask) >> gshift);
					b[tileindex] = (unsigned char)((color & bmask) >> bshift);
					break;
				case	24:
				case	32:
					color = *(unsigned long *)pAdr;
					r[tileindex] = (unsigned char)((color & rmask) >> rshift);
					g[tileindex] = (unsigned char)((color & gmask) >> gshift);
					b[tileindex] = (unsigned char)((color & bmask) >> bshift);
					break;
			}
			tileindex++;
		}
		pAdr += ddcaps.lPitch;
	}

	screen->pDDSBack->Unlock(NULL);

	// insert sprites...
	tileindex = 0;
	unsigned long	m = 15, z;

	for (cy = 0; cy < height; cy += 16)
	{
		for (cx = 0; cx < width; cx += 16)
		{
			pfacelist->nedge = 4;
			pfacelist->surf = DefaultFlareSurface;
			pfacelist->alpha = 0;
			pfacelist->vertice = pvertexlist;

			pfacelist->diff.r = r[tileindex];
			pfacelist->diff.g = g[tileindex];
			pfacelist->diff.b = b[tileindex];
			tileindex++;

			(pvertexlist + 0)->x = (float)(cx - m);
			(pvertexlist + 0)->y = (float)(cy - m);
		z = (pvertexlist + 0)->z = 1.0f;
			(pvertexlist + 0)->u[nXML_COLOR] = 0.0f;
			(pvertexlist + 0)->v[nXML_COLOR] = 0.0f;

			(pvertexlist + 1)->x = cx + m;
			(pvertexlist + 1)->y = cy - m;
			(pvertexlist + 1)->z = 1;
			(pvertexlist + 1)->u[nXML_COLOR] = 1.0;
			(pvertexlist + 1)->v[nXML_COLOR] = 0.0;

			(pvertexlist + 2)->x = cx + m;
			(pvertexlist + 2)->y = cy + m;
			(pvertexlist + 2)->z = 1;
			(pvertexlist + 2)->u[nXML_COLOR] = 1.0;
			(pvertexlist + 2)->v[nXML_COLOR] = 1.0;

			(pvertexlist + 3)->x = cx - m;
			(pvertexlist + 3)->y = cy + m;
			(pvertexlist + 3)->z = 1;
			(pvertexlist + 3)->u[nXML_COLOR] = 0.0;
			(pvertexlist + 3)->v[nXML_COLOR] = 1.0;

			if (ClipFace2D())
				ValidFace( unsigned long(z * cscene->radixprec), nX_TRANSPARENTRADIXCONST);

		}
	}

	if ( (screen->cd3ddevice->mxnbmap > 1) && !screen->bForceSingleRaster)
			RasterizeMultiTexture();
	else
			RasterizeMonoTexture();



}

#endif