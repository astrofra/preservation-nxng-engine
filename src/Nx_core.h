	/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		nX CLASS DEFINITION v9
		----------------------

		nX is an execution unit for nX_Scene objects. It keeps track of all
		scenes, objects, texmap used inside itself to avoid memory loss.
		Objects and Texmaps are optimized when they are used in the same nX instance.

	   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  */


		// nX ALLOCATIONS VARS
		#define	nX_ALLOCTYPE	unsigned long
		#define	nX_ALLOCNONE				0
		#define	nX_ALLOCMAX					1
		#define	nX_ALLOCSMALL				2
		#define	nX_ALLOCSMALLEST			3


		#define PRECAREFRESHRATE			40	// # of frames before new preca packet...


struct nX_AllocInfo
{
	unsigned long		totalface;
	unsigned short		nedgemaxobj;
	unsigned long		nglobedgemaxobj;
	unsigned long		ngvtxface;
};


class	nX
{
	public:

	unsigned long	buildedface;				// # of builded face(s)
	unsigned long	statbuildedface;			// # of builded face(s) without clipping
	unsigned long	lastfaceoffset;				// # of last face built
	unsigned short	FPS;						// FPS

	// ENGINE SETTINGS --------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	bool			Draw_Forward;				// Draw scene forward/backward...
	float			yOff;						// Adaptive Sampling offset
	unsigned long	nX_TRANSPARENTRADIXCONST;	// (0)
	unsigned long	nX_OPAQUERADIXCONST;		// (1 << 30)
	void			SetDrawForward();			// Set the engine to draw scene FORWARD
	void			SetDrawBackward();			// Set the engine to draw scene BACKWARD

	void			SetRenderer(long);			// Select renderer (dX/piXel!)
	void			SetAntialias(bool);			// Toggle antialias
	bool			antialias;					// Antialias

	// LOADER -----------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	// Load filename at void * and set size in ulong *
	bool			Load(char *, unsigned char **, unsigned long *, xLinker *);
	bool			FileExist(char *, xLinker *);
	qFileIO			*iIO1, *iIO2;
	void			SetLoader(qFileIO *, qFileIO *);

	// PRECA METHODS ----------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	// Ensure MOA file is not corrupt
	bool			CheckMOACompliance(char *, long);
	void			GetMOAFilename(char *);

	// fast sin/cos
	float			qSin[sincosREV], qCos[sincosREV];

	// RADIX METHODS ----------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////	

	RadixNode		*buffer2sort;				// Face buffer to sort
	RadixNode		*buffersorted;				// Face buffer sorted
	// Pointers to face buffer used to sort polys
	RadixNode		*pbuffer2sort, *pbuffersorted;
	void			RadixSortFaceList();		// Radix sort buffers (n)

	// ENGINE SCRATCH BUFFERS -------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	nX_face			*facelist, *pfacelist;		// Face buffer
	nX_vertex		*vertexlist, *pvertexlist;	// Face buffer vertice

	float			*edgesbuffer1_adress;		// Primary buffer to rotate vertice
	float			*edgesbuffer2_adress;		// Secondary buffer to rotate vertice
	float			*facenormalsA_adress;		// Primary buffer to rotate face normals
	float			*facenormalsB_adress;		// Secondary buffer to rotate face normals
	float			*edgenormalsA_adress;		// Primary buffer to rotate edge normals
	float			*edgenormalsB_adress;		// Secondary buffer to rotate edge normals

	// Associed pointers
	float			*p3dpswap1, *p3dpswap2, *p3dpswapt;
	float			*pfnorm1, *pfnorm2, *pfnormt;
	float			*penorm1, *penorm2, *penormt;

	// nX MATHS METHODS -------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	void			MatMul44(float *, float *, float *);
	void			MatMul33(float *, float *, float *);
	bool			ForceAllNormals;			// Force all normals to be computed at load time
	void			RequireAllNormals(bool);	// Toggle force normal flag

	// TSP MANAGEMENT METHODS -------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	void			FreeTSPNode(nX_TSPnode *);	// Free a TSP node and all its child

	// nX BUFFERS METHODS -----------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	void			ResetAlloc();				// reset allocation structure
	void			AllocBuffers(nX_ALLOCTYPE);	// allocate runtime buffers (SMALLEST, SMALL, FULL)
	void			DeallocBuffers();			// deallocate runtime buffers
	void			ResetBuffers();				// reset runtime buffers
	// compute scene memory requirement (true for multiscene)
	void			ComputeSceneMemReq(nX_Scene *, bool);

	nX_AllocInfo	ComputeAlloc;				// allocation structure

	// nX FX MAP AND SURFACE METHODS ------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	void			InitFXs();					// load an create all engine's FXs surfaces and maps
	void			GetFXsiD3D();				// get all engine's FXs D3D maps
	void			FreeFXs();					// release all engine's FXs surface and maps
	void			UpdateTexMap();				// update all texmaps

	// I/O METHODS ------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	ScreenMN		*screen;					// Screen CLASS that handle this nX
	void			BeginFrame(bool clear);		// true to clear the frame
	void			EndFrame();					// End something but can't remember what...

	nX_Console		*console;					// console used to transmit outputs
	void			SetConsole(nX_Console *);	// Set output console for nX instance
	void			Verbose(char *);			// Send message to nX console if any
	void			VerboseOver(char *);		// Send message to nX console if any
	void			SetProgress(char);			// Display progress percent to nX console if any
	void			DisplayFrameReport();		// Display statistics of elapsed frame

	// DIRECTINPUT METHODS ----------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	bool			dInputAvail;				// Is DirectInput ready?
	void			SetcInput(cInput *);		// Set cInput instance to be used in this nX
	cInput			*cIn;						// nX's cInput instance...

	// 2D EFFECTS METHODS -----------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	#ifdef _2D_EFFECT_
	/*
		nX::Tilemap()
		-------------
		Fills screen with tiles of dimension 8x8.
		Mapping them with coordinates set in v (x, y members of v are ignored!).

		Note: 8x8 is NOT relative to the referencewidth and referenceheight
		----  screen members.
	*/
	void			TileMap(nX_vertex *, nX_Surface *);

	// compute TA matrix
	void			_2D_Distort_TA(nX_vertex *);
	// compute TB matrix
	void			_2D_Distort_TB(nX_vertex *);
	// blend two matrix effect [A, coef A, B, coef B] A = A * coefA + B * coefB
	void			_2D_BlendMatrix(nX_vertex *, float, nX_vertex *, float);

	void			_2D_Reset();				// Reset synchro vars
	void			_2D_Init();					// Allocate vertex buffer
	void			_2D_Free();					// Free vertex buffer

	unsigned long	_2D_Tick, _2D_dTick;		// Synchro vars
	nX_vertex		*_2D_BufferA, *_2D_BufferB;	// Vertex buffers
	#endif

	// FACELIST METHODS -------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	bool			ClipFace();					// clip all vertice in a face (x, y, z)
	bool			ClipFace2D();				// clip all vertice in a face (x, y)
	bool			ClipSprite();				// clip all vertice in a face (x, y)
	void			Rasterize();				// Auto select single/multi texture rasterizer
	void			RasterizeMultiTexture();	// process all faces in facelist down to screen via registered output (MULTITEXTURE accelerator optimised)
	void			RasterizeMonoTexture();		// process all faces in facelist down to screen via registered output

	/*

		ValidFace(unsigned long):

		This function add face to the sorting list
		and keep face counters up to date...

		Should be called any time a ClipFace() call
		returned true or prepare for a crash!
		You can call it manually after having added
		a face manually to pfacelist (it MUST be
		clipped or must not need one! no further
		clipping will be done beyond this call)

	*/
	void			ValidFace(unsigned long, unsigned long);


#ifdef	nX_ENABLEBLOOM
	void			ProcessBloom();				// Bloom
	void			PostProcessBloom();			// Post-Process Bloom
#endif

	// SOFTWARE RASTERIZERS ---------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	#include		"nXng/piXel.h"				// RIP old friend ):

	// LIGHTNING THINGS -------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	nXcolor			diff, spec;					// Ugly 'global-like' stuff

	// MISC. SHIT -------------------------------------------------------------
	///////////////////////////////////////////////////////////////////////////

	nX(ScreenMN *);
	~nX();

					// TexMap list (texmaps are shared between all scenes in one engine instance)
	nX_LLTexMap		*LLtexmap;

	nX_TexMap		*FindTexMap(char *, unsigned long);			// seek for a texmap matching id&caps
	void			RegisterTexMap(nX_TexMap *);				// add a texmap to nX linked list
	void			UnregisterTexMap(nX_LLTexMap *);			// remove a texmap from nX linked list
	void			FlushTexMap();								// free all registered texmaps

					// FPU state
	unsigned long	FPUcw;
	void			FPUsetup();
	void			FPUforceOK();
	void			FPUrestore();

					// Loaders methods (two loaders are provided... if loading fails on 1st loader then loading is aptemted on 2nd loader if any)
	void			SetLoaders(qFileIO *, qFileIO *);

					// The largest Z in world after transformation
	float			largestZ;				// the maximal Z-value reached AFTER world tranformation
	
					// Simple w-filler
	bool			childusingzbuffer;

					// Tells Rasterizer if a scene requested fog...
	bool			childusingfog;

					// Reset all FX set...
	void			ResetFX();

					// Replaying behavior
	bool			Synch;					// time-synched replay
	void			SetSynch(bool);			// timesynch replay?

					// Blank surface
	nX_Surface		*BlankSurface;			// default surface
	nX_Surface		*DefaultFlareSurface;
	nX_Surface		*DefaultBLOOMSurface;
	nX_Surface		*DefaultSparkSurface;
	nX_Surface		*DefaultPolySizeSurface;
	nX_Surface		*ReportSurf;
	nX_TexMap		*DefaultEVMAP;

					// Default nX shader
	unsigned long	SurfDefaultShading;		// default shading for a surface
	void			SetDefaultShading(unsigned long);

					// Lightwave path for shortened filenames
	char			*LWpath;
	void			SetLWpath(char *);

					// Scene linked list
	unsigned short	nbscene;
	nX_Scene		*cscene;				// the currently registered scene instance
	nX_Scene		*LLscene;				// scene Linked List

	void			ComputeRGBshades();		// fill the mulshader table

					// Add a trigone to nX_facelist
	void			Trigone(float, float, float, float, float, float, float, float, float, nX_Surface *, float);
					// Add 64 planes to nX_facelist to form one unique Gaussian plane
	void			GaussianPlane(float, float, float, float, float, float, nX_Surface *);
					// Add a simple square to nX_facelist
	void			Square(float, float, float, float, float, long, nX_Surface *, float, nXcolor *);
					// Add a simple plane to nX_facelist
	void			Plane(float, float, float, float, float, float, float, float, float, long, nX_Surface *, float);
					// Add an entity as a spark set
	void			EdgeAsSparks(nX_Entity *, float *);
					// Model light upon a vertice set
	void			ModelLight(nX_Entity *, nX_Surface *, float *, float *, long);

};