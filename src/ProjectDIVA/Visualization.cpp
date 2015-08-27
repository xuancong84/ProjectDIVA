// Draw 3D FFT spectrum
#define		USESIMD	1
#include <map>
#include <winerror.h>
#include "opengl.h"
#include "glext.h"
#include "vect_sse.h"
#include "parameters.h"
#include "Visualization.hpp"
#include "TempoVis.hpp"

const int	BELT_INC  = (int)((FLOAT)TempoPrecision/BELT_FPS+0.5);
const FLOAT BELT_OFF  = (FLOAT)M_PI/BELTCIRCUMSIZE;

int total_added = 0;

GLfloat	space_attn[3]	= { 0.0f, 0.2f, 0 };
float	zero_vector[4]	= { 0, 0, 0, 1 };
float	ones_vector[4]	= { 1, 1, 1, 1 };
float	zero_4vector[4]	= { 0, 0, 0, 0 };

FLOAT	l_ambient[4]={0.4f,0.4f,0.4f,1.0f};
FLOAT	l_diffuse[4]={0.2f,0.2f,0.2f,1.0f};
FLOAT	l_specular[4]={1.5f,1.5f,1.5f,1.0f};
FLOAT	m_ambient[4]={0.4f,0.4f,0.4f,1.0f};
FLOAT	m_diffuse[4]={0.2f,0.2f,0.2f,1.0f};
FLOAT	m_specular[4]={1.0f,1.0f,1.0f,1.0f};
FLOAT	m_emissive[4]={0.01f,0.01f,0.01f,1.0f};

class	Visualization{
public:
	// Special parameters
	GLwindow	*pGLwin;
	__int64		start_stamp;
	LONG		g_sampleRate;
	LONG		g_nChannels;

	// Normal parameters
	int		N_total_frames;
	int		N_draw_frames;
	int		bins_per_bin;
	int		verts_per_line;
	int		*multidraw_first;
	int		*multidraw_count;
	int		belt_lastParity;
	int		belt_prim_count;
	int		fft_prim_count;
	int		sprite_count;
	int		tempo_state;									// number of spin stars to be drawn, 0: don't draw any (tempo not reliable)
	int		last_tempo_index;
	int		last_pri_tempo_index;
	int		last_phase_index;
	int		last_phase_posi;
	GLuint	texture_names[1];
	FLOAT	last_tempo_change_time, last_tempo_change_time2;
	FLOAT	last_tempo_est_time;
	FLOAT	freq_bin_cutoff;								// for extending freq. bins
	FLOAT	preset_tempo;									// >0: valid, =0: variable tempo, <0: unknown
	int		preset_meter;									// to distinguish 4/4 vs 3/3 meters
	int		tempoMeter;										// to distinguish 4/4 vs 3/3 meters
	FLOAT	Data_rate;
	DWORD	Data_cnt;
	DWORD	FPS_cnt, FPS_startTick;
	FLOAT	target_eye[3];									// target eye position coordinate 
	FLOAT	target_up[3];									// target up position coordinate 
	FLOAT	current_eye[3];									// current eye position coordinate 
	FLOAT	current_up[3];									// current up position coordinate 
	FLOAT	center_posi[3];									// current center position coordinate 
	FLOAT	belt_radial_posi;								// head of the belt, [0,2*pi]
	FLOAT	FPS;											// average no. of frames per second
	FLOAT	tempoPeriod, tempoPhase;						// in seconds
	FLOAT	tempo_enhance_factor;
	FLOAT	phase_enhance_factor;
	FLOAT	tempo_period_frac, tempo_period_step;			// fractional part of tempo period
	FLOAT	tempo_period_acc;								// fractional accumulator
	FLOAT	last1FFT[FFTSIZE],last2FFT[FFTSIZE];
	FLOAT	star_scale;
	FLOAT	StarVB[6*4*3*3];								// vertex buffer of spin stars
	FLOAT	BeltVB[BELTCIRCUMSIZE*BELTDEPTHSIZE*3];			// vertex buffer of the belt
	FLOAT	BeltNB[BELTCIRCUMSIZE*BELTDEPTHSIZE*3];			// vertex buffer of the belt
	DWORD	BeltCB[BELTCIRCUMSIZE*BELTDEPTHSIZE];			// color buffer of the belt
	WORD	BeltIB[BELTCIRCUMSIZE*BELTDEPTHSIZE*3];			// index buffer of the belt
	BYTE	BeltAlpha[BELTDEPTHSIZE];						// belt alpha for fast update
	FLOAT	belt_x_off, belt_y_off;
	FLOAT	belt_last_radius;
	DWORD	belt_last_color;
	DWORD	wave_last_color;
	FLOAT	SpriteVB[MAXSPRITES*3];							// geometric coordinates of spin sprite
	DWORD	SpriteCB[MAXSPRITES];							// color buffer of spin sprite
	FLOAT	SpaceVB[SPACEPOINTS*3];
	FLOAT	WaveBuffer[FFTSIZE*3];
	FLOAT	last_time_second;
	FLOAT	last_time_second2;								// 2: for update random target eye
	FLOAT	last_time_second3;								// 3: for update camera path
	FLOAT	power_level;									// Average power level for fft adaptation
	FLOAT	PhaseAccBuf[TempoMaxShift];						// Phase accumulation buffer for phase est.
	FLOAT	PhaseAccBuf2[TempoMaxShift];					// Phase accumulation buffer for phase est.
	FLOAT	CombFilter[TempoMaxShift];						// for phase computation
	FLOAT	TempoWindowFunc[TempoMaxShift];
	FLOAT	ExpWindowFunc[TempoMaxShift];
	FLOAT	TempoSpec[TempoMaxShift];
	FLOAT	TempoSpecP[TempoMaxShift];
	FLOAT	TempoAcorr[TempoMaxShift],
			TempoEcorr[TempoMaxShift],
			TempoEDcorr[TempoMaxShift],
			TempoEDDcorr[TempoMaxShift];

	// Buffers
	FLOAT		*FFTVB, *FFTNB;
	DWORD		*FFTCB;
	WORD		*FFTEB;
	LoopBuffer	*fftBuffer;
	LoopBuffer	*maxBuffer;
	LoopBuffer	*TempoABuffer;		// Amplitude buffer
	LoopBuffer	*TempoEBuffer;		// Energy buffer
	LoopBuffer	*TempoEDBuffer;		// Energy derivative buffer
	LoopBuffer	*TempoEDDBuffer;	// Energy derivative's derivative buffer
	LoopBuffer	*BeltEBuffer;		// Belt Energy buffer
	LoopBuffer	*BeltEDBuffer;		// Belt Energy derivative buffer
	Camera		*camera,*fft_camera;
	BYTE		*PointSpriteData;

	// Functions
	Visualization( GLwindow *GLmain, BYTE *psData ){
		memset(this,0,sizeof(Visualization));
		// default bounding box
		pGLwin			=	GLmain;
		N_total_frames	=	1024;
		verts_per_line	=	128;
		PointSpriteData	=	psData;
		init();
	}
	~Visualization(){ free(); }

	void	free(){
		if( fftBuffer )		delete	fftBuffer;
		if( maxBuffer )		delete	maxBuffer;
		if( camera )		delete	camera;
		if( fft_camera )	delete	fft_camera;
		if( TempoABuffer )	delete	TempoABuffer;
		if( TempoEBuffer )	delete	TempoEBuffer;
		if( TempoEDBuffer )	delete	TempoEDBuffer;
		if( TempoEDDBuffer )delete	TempoEDDBuffer;
		if( BeltEBuffer )	delete	BeltEBuffer;
		if( BeltEDBuffer )	delete	BeltEDBuffer;
		if( FFTVB )	delete [] FFTVB;
		if( FFTNB )	delete [] FFTNB;
		if( FFTCB )	delete [] FFTCB;
		if( FFTEB )	delete [] FFTEB;
		if( multidraw_first )	delete [] multidraw_first;
		if( multidraw_count )	delete [] multidraw_count;
	}

	inline void	SetCosineArray( int tempoInd ){
		register FLOAT mul = ((FLOAT)M_2PI/tempoInd);
		for( int x=0; x<tempoInd; x++ )	CombFilter[x] = cos(x*mul);
	}

	void	init(){
		free();

		N_draw_frames	=	verts_per_line;
		power_level		=	RMSMIN;
		fftBuffer		=	new LoopBuffer( N_draw_frames*2, N_draw_frames, sizeof(FLOAT)*verts_per_line, (FLOAT)N_draw_frames/FFTDURATION );
		maxBuffer		=	new LoopBuffer( N_draw_frames/2, 0, sizeof(FLOAT), (FLOAT)N_draw_frames/FFTDURATION );
		TempoABuffer	=	new LoopBuffer( TempoBufferLength*TempoPrecision*2, TempoBufferLength*TempoPrecision, sizeof(FLOAT), TempoPrecision );
		TempoEBuffer	=	new LoopBuffer( TempoBufferLength*TempoPrecision*2, TempoBufferLength*TempoPrecision, sizeof(FLOAT), TempoPrecision );
		TempoEDBuffer	=	new LoopBuffer( TempoBufferLength*TempoPrecision*2, TempoBufferLength*TempoPrecision, sizeof(FLOAT), TempoPrecision );
		TempoEDDBuffer	=	new LoopBuffer( TempoBufferLength*TempoPrecision*2, TempoBufferLength*TempoPrecision, sizeof(FLOAT), TempoPrecision );
		BeltEBuffer		=	new LoopBuffer( BELTDEPTHSIZE*2, BELTDEPTHSIZE, sizeof(FLOAT), BELT_FPS );
		BeltEDBuffer	=	new LoopBuffer( BELTDEPTHSIZE*2, BELTDEPTHSIZE, sizeof(FLOAT), BELT_FPS );
		camera			=	new Camera( pGLwin );
		fft_camera		=	new Camera( pGLwin );
		FFTVB			=	new FLOAT [N_draw_frames*verts_per_line*3];
		FFTNB			=	new FLOAT [N_draw_frames*verts_per_line*3];
		FFTCB			=	new DWORD [N_draw_frames*verts_per_line];
		FFTEB			=	new WORD  [N_draw_frames*verts_per_line*2];
		multidraw_first	=	new int [N_draw_frames+verts_per_line];
		multidraw_count	=	new int [N_draw_frames+verts_per_line];

		/*
		{
			float	mat[16]={1,5,8,9,5,2,6,8,8,6,3,7,9,8,7,4},vec[4]={0.1,0.2,0.3,0.4},mat2[16];
			float	v1[4]={1,2,3,4}, v2[4]={1,3,5,6}, v3[4]={11,12,13,14};
			VectorCrossAdd(v1,v2,v3);
			int x=0;
		}*/
		// Load point sprite texture
		{
			glGenTextures( 1, texture_names );
			glBindTexture( GL_TEXTURE_2D, texture_names[0] );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			BYTE *pAlpha = &PointSpriteData[3];
			for( int x=0;x<4096;x++,pAlpha+=4 ) *pAlpha = 0xff;//*(pAlpha-1)?0xff:0;
			glTexImage2D( GL_TEXTURE_2D, 0, 4, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, PointSpriteData );
		}

		// Initialize spining sprites
		{
			FLOAT	*pSprite = SpriteVB;
			for( int x=0; x<MAXSPRITES; x++ ){
				if( rand()>(RAND_MAX/4.0f) ) continue;
				pSprite[0]	= 0;
				pSprite[1]	= 0;
				pSprite[2]	= (FLOAT)x/(MAXSPRITES-1)*BELTLENGTH+BELTTAIL;
				SpriteCB[sprite_count]	= RandomColor( 2 );
				pSprite += 3;
				sprite_count++;
			}
			BYTE	*pAlpha = &((BYTE*)SpriteCB)[3];
			FLOAT	z_min = BELTTAIL, mul = 240.0f/BELTLENGTH;
			pSprite = &SpriteVB[2];
			for( int x=0; x<sprite_count; x++,pAlpha+=4,pSprite+=3 ){
				*pAlpha = (BYTE)((*pSprite-z_min)*mul+15.5);
			}// update sprite alpha values
		}

		// Initialize belt points
		{
			FLOAT	*pBelt	= BeltVB;
			FLOAT	*pNormal= BeltNB;
			DWORD	*pColor	= BeltCB;
			WORD	*pInd	= BeltIB;
			WORD	index	= 0;
			for( int x=0; x<BELTDEPTHSIZE; x++){
				belt_last_color = InterColor( belt_last_color, RandomColor(1), BELT_COLOR_SRC );
				for( int y=0; y<BELTCIRCUMSIZE; y++ ){
					*pInd++ = index+y;
				}// draw co-circular part
				if( x ) for( int y=0; y<BELTCIRCUMSIZE; y++ ){
					*pInd++ = index+y;
					*pInd++ = index+y-BELTCIRCUMSIZE;
				}// draw zig-zag part
				for( int y=0; y<BELTCIRCUMSIZE; y++,pBelt+=3,pColor++ ){
					pBelt[0] = 0;
					pBelt[1] = 0;
					pBelt[2] = (FLOAT)x/(BELTDEPTHSIZE-1)*BELTLENGTH+BELTTAIL;
					pNormal[0] = cos(y*M_2PI/BELTCIRCUMSIZE);
					pNormal[1] = sin(y*M_2PI/BELTCIRCUMSIZE);
					pNormal[2] = 0;
					*pColor	 = belt_last_color;
				}
				BeltAlpha[x] = (BYTE)(x*BELTALPHAMAX/(BELTDEPTHSIZE-1)+0.5f);
				index += BELTCIRCUMSIZE;
			}
			BYTE	*pAlpha = &((BYTE*)BeltCB)[3];
			for( int x=0; x<BELTDEPTHSIZE; x++ ){
				register BYTE alpha = BeltAlpha[x];
				for( int y=0; y<BELTCIRCUMSIZE; y++,pAlpha+=4 ) *pAlpha = alpha;
			}// update belt alpha values
			belt_prim_count = pInd-BeltIB;
		}

		// Initialize space points
		{
			FLOAT	*pSpace = SpaceVB;
			FLOAT	Point[3];
			for( int x=0; x<SPACEPOINTS; x++,pSpace+=3 ){
				do{
					RandomBoundVector( Point );
				}while(abs(Point[1])>0.8f);
				VectorMul( Point, (FLOAT)SPACERADIUS, pSpace );
			}
		}

		// Initialize time domain wave buffer
		{
			FLOAT	*pWave = WaveBuffer;
			for( int x=0; x<FFTSIZE; x++,pWave+=3 ){
				pWave[0] = SPINRADIUS*cos(x*(FLOAT)M_2PI/FFTSIZE);
				pWave[1] = SPINRADIUS*sin(x*(FLOAT)M_2PI/FFTSIZE);
				pWave[2] = 0.0f;
			}
		}

		// Generate star geometry
		{
			FLOAT	vertices[8+6][3]={	{-1,1,1},{1,1,1},{1,-1,1},{-1,-1,1},{-1,1,-1},{1,1,-1},{1,-1,-1},{-1,-1,-1},
										{-STARRADIUS,0,0},{STARRADIUS,0,0},{0,STARRADIUS,0},
										{0,-STARRADIUS,0},{0,0,STARRADIUS},{0,0,-STARRADIUS}};
			int		indices[24][3]	={	{8,3,0},{8,0,4},{8,4,7},{8,7,3},{9,1,2},{9,2,6},{9,6,5},{9,5,1},
										{10,0,1},{10,1,5},{10,5,4},{10,4,0},{11,2,3},{11,6,2},{11,7,6},{11,3,7},
										{12,1,0},{12,2,1},{12,3,2},{12,0,3},{13,4,5},{13,5,6},{13,6,7},{13,7,4}};
			for( int x=0; x<24; x++ )
				for( int y=0; y<3; y++)
					for( int z=0; z<3; z++ )
						StarVB[(x*3+y)*3+z] = SPINSTARSCALE*vertices[indices[x][y]][z];
		}

		// Generate FFT index buffer
		{
			int i=0, primCount=0;
			for( int x=0; x<N_draw_frames; ++x,++primCount ){
				multidraw_first[primCount] = (int)&FFTEB[i];
				multidraw_count[primCount] = verts_per_line;
				i += verts_per_line;
			}// transverse lines
			for( int x=0; x<verts_per_line; ++x,++primCount ){
				multidraw_first[primCount] = (int)&FFTEB[i];
				multidraw_count[primCount] = N_draw_frames;
				i += N_draw_frames;
			}// longitudinal lines
			fft_prim_count = primCount;
		}

		// Initialize FFT vertex buffers
		{
			FLOAT	X_inc  = (FLOAT)FFTSIDELENGTH*2/(verts_per_line-1);
			FLOAT	Z_inc  = (FLOAT)FFTSIDELENGTH*2/(N_draw_frames-1);
			FLOAT	fdist  = sqrt((FLOAT)N_draw_frames*N_draw_frames+verts_per_line*verts_per_line);
			FLOAT	*pVert = FFTVB;
			FLOAT	*pNorm = FFTNB;
			DWORD	*pColor= FFTCB;
			WORD	*pElem = FFTEB;
#define	FFTCOLORMAX	255
			for( int i=0,z=0; z<N_draw_frames; ++z ){
				FLOAT fval = -(FLOAT)FFTSIDELENGTH+Z_inc*z;
				for( int x=0; x<verts_per_line; ++x,pVert+=3,pNorm+=3,pColor++,i++ ){
					pVert[0] = -(FLOAT)FFTSIDELENGTH+X_inc*x;
					pVert[1] = 0;
					pVert[2] = fval;
					pNorm[0] = 0;
					pNorm[1] = 1.0f;
					pNorm[2] = 0;
					*pElem++ = (WORD)i;		//if(!(x&1) && !(z&1))  16-bit index, cannot have more than 65536 distinct vertices
					*pColor	 = (FFTALPHAMAX<<24) | (((z*FFTCOLORMAX/N_draw_frames)&0xff)<<16)
									| (((x*FFTCOLORMAX/verts_per_line)&0xff)<<8) 
									| (FFTCOLORMAX-(((int)(hypot(x,z)*FFTCOLORMAX/fdist))&0xff));
				}// transverse
			}// longitudinal
			Transpose( (char*)pElem, (char*)FFTEB, N_draw_frames, verts_per_line, sizeof(WORD) );
		}

		// Generate the window function
		//FLOAT	factor = (FLOAT)M_E/(CTempoPeriod*TempoPrecision);
		FLOAT	f2 = 2*CTempoPeriod*TempoPrecision;
		for( int x=1; x<TempoMaxShift; x++ ){
			//TempoWindowFunc[x]	= pow( factor*x, (FLOAT)CTempoEnhFactor/(factor*x))*(1-exp(-factor*x) );
			TempoWindowFunc[x]	= pow((FLOAT)M_E*x/f2,(FLOAT)f2/x)*x*(exp(-(FLOAT)M_E*x/f2) );
			ExpWindowFunc[x]	= 1-exp(-M_E*x*2/(CTempoPeriod*TempoPrecision) );
		}

		RandomDirnVector( target_eye );
		memcpy( current_eye, target_eye, sizeof(target_eye) );
		RandomDirnVector( target_up );
		memcpy( current_up, target_up, sizeof(target_up) );
		FPS = (FLOAT)(FPS_cnt = 0);
		FPS_startTick = GetTickCount();
	}

	void reset( TimedLevel *pLevels ){
		last_time_second	= 0;
		last_time_second2	= 0;
		last_tempo_est_time	= 0;
		start_stamp		= 0;//pLevels?pLevels->timeStamp:
		Data_cnt		= 0;
		total_added 	= 0;
		tempo_state 	= 0;
		star_scale		= 1;
		tempoMeter		= 2;
		last_tempo_change_time	= 0;
		last_tempo_change_time2	= 0;
		tempo_enhance_factor	= 1;
		phase_enhance_factor	= 1;
		if(freq_bin_cutoff>0)	freq_bin_cutoff	= 0;
		if(!freq_bin_cutoff)	bins_per_bin	= 1;

		fftBuffer->Reset();
		TempoABuffer->Reset();
		TempoEBuffer->Reset();
		TempoEDBuffer->Reset();
		TempoEDDBuffer->Reset();
		BeltEBuffer->Reset();
		BeltEDBuffer->Reset();
		memset( last1FFT, 0, FFTSIZE );
		memset( last2FFT, 0, FFTSIZE );
		memset( PhaseAccBuf, 0, TempoMaxShift*sizeof(FLOAT) );
		memset( PhaseAccBuf2, 0, TempoMaxShift*sizeof(FLOAT) );
		memset( TempoAcorr, 0, TempoMaxShift*sizeof(FLOAT) );
		memset( TempoEcorr, 0, TempoMaxShift*sizeof(FLOAT) );
		memset( TempoEDcorr, 0, TempoMaxShift*sizeof(FLOAT) );
		memset( TempoEDDcorr, 0, TempoMaxShift*sizeof(FLOAT) );
		//Record( play_state, true );
	}// Upon track position change or new song starts

	int	addData( TimedLevel *pLevels ){
		int		b_added=0, n_added, f_added=0;
		bool	bNeed=( g_mode&1 || (!(g_mode&1)&&preset_tempo<=0));
		FLOAT	current_time_second, phase_energy;
		FLOAT	curFFT[FFTSIZE];
		BYTE	*fft_data = pLevels->frequency[0], *wav_data = pLevels->waveform[0];

		// Update FPS which is needed for rendering and tempo estimation
		{
			FLOAT	fval=0;
			n_added = GetTickCount();
			if( n_added != FPS_startTick ){
				fval = (++FPS_cnt)*1000.0f/(n_added-FPS_startTick);
				if( FPS_cnt>20 && fval>2 ){
					FPS = (FPS+fval)*0.5f;
					if( FPS_cnt>= 200 ){
						FPS_cnt = 0;
						FPS_startTick = GetTickCount();
					}
				}
			}
			if(!FPS) FPS = fval?fval:20;
		}

		// Get time stamp
		current_time_second	= (FLOAT)(pLevels->timeStamp-start_stamp)*1e-7f;
		if( current_time_second<last_time_second ){
			reset( pLevels );
			current_time_second = 0;
		}
		n_added = (current_time_second!=last_time_second);

		// Add FFT spectrum data
		if(n_added){
			FLOAT drawFFT[FFTSIZE];
			register FLOAT	fft_max = -FLT_MAX;
			register FLOAT	power_factor = (FLOAT)(FFTHEIGHT/power_level);
			register FLOAT	mul_factor = (FLOAT)(FFTEXPFACTOR/bins_per_bin);
			if( g_nChannels == 2 ){
				BYTE *p1=pLevels->frequency[1];
				for( int x=0; x<FFTSIZE; x++ ) curFFT[x] = (p1[x]+fft_data[x])*0.5f;
			}else for(int x=0; x<FFTSIZE; x++) curFFT[x] = (FLOAT)fft_data[x];

			for( int x=0, z=0; x<verts_per_line; x++ ){
				FLOAT	fval = sin( x*bins_per_bin*(float)M_PI_2/FFTSIZE );
				FLOAT	sum	 = 0;
				for( int y=0;y<bins_per_bin;y++,z++ ) sum += curFFT[z];
				fval = (exp(sum*mul_factor)-1)*(fval+0.0625f);
				if( fval>fft_max ) fft_max = fval;
				drawFFT[x]=	fval*power_factor;
			}// sum displayed freq. bins

			if( fft_max*power_factor > FFTHEIGHT ){
				//__asm int 3
				mul_factor	 = (FLOAT)(FFTHEIGHT/fft_max/power_factor);
				power_factor = (FLOAT)(FFTHEIGHT/fft_max);
				power_level	 = fft_max;
				for(int x=0; x<verts_per_line; x++) drawFFT[x] *= mul_factor;
			}

			if( freq_bin_cutoff >= 0 ){
				// Adapt FFT bin width: bins_per_bin
				FLOAT	sum=0, mean=0;
				for( int x=0; x<FFTSIZE; x++ ) mean += exp(curFFT[x]*FFTEXPFACTOR);
				mean /= FFTSIZE;
				for( int x=0; x<FFTSIZE; x++ ) sum += pow(exp(curFFT[x]*FFTEXPFACTOR)-mean,2);
				if( sum > RMSMIN ){
					sum *= 0.9f;
					FLOAT	acc=0;
					int		x;
					for( x=0; x<FFTSIZE && acc<sum; x++ ) acc += pow(exp(curFFT[x]*FFTEXPFACTOR)-mean,2);
					sum = (FLOAT)x;
					expUpdate( &freq_bin_cutoff, &sum, 1, last_time_second, current_time_second, 4 );
					if( freq_bin_cutoff > bins_per_bin*192 ){
						if( bins_per_bin<8 ){
							bins_per_bin++;
							freq_bin_cutoff = (float)bins_per_bin*verts_per_line;
						}
					}else if( freq_bin_cutoff < bins_per_bin*96 ){
						if( bins_per_bin>1 ){
							bins_per_bin--;
							freq_bin_cutoff = (float)bins_per_bin*verts_per_line;
						}
					}
				}// extend bins if spectrum is wide
			}

			// Add draw FFT
			f_added = fftBuffer->AddFrame( current_time_second, drawFFT );
			maxBuffer->AddFrame( current_time_second, fft_max );
			fft_max = getMax( (FLOAT*)maxBuffer->data, maxBuffer->N_total_frames );

			// Adapt FFT magnitude: power_factor
			expUpdate( &power_level, &fft_max, 1, last_time_second, current_time_second, POWERHALFLIFE );
			if( power_level < RMSMIN ) power_level = RMSMIN;
		}


		// Add TempoEstimation data
		FLOAT	corr_spec[TempoMaxShift];

		// Add Amplitude data and do autocorrelation
		if( n_added ){
			Data_cnt ++;
			Data_rate = (FLOAT)Data_cnt*1e7f/(pLevels->timeStamp-start_stamp);

			int Amax=-1000, Amin=1000;
			for( int x=0 ;x<FFTSIZE; x++ ){
				register int A = (int)wav_data[x]-128;
				if( A>Amax ) Amax = A;
				else if( A<Amin ) Amin = A;
			}
			phase_energy = (FLOAT)(Amax-Amin);
			{
				FLOAT	new_scale = exp(phase_energy/362.0352f);
				expUpdate( &star_scale, &new_scale, 1, last_time_second, current_time_second, StarScaleHalfLife );
			}
			n_added = TempoABuffer->AddFrame( current_time_second, pow(phase_energy/16.0f,4.0f) );
			total_added +=  n_added;
			if( bNeed ){
				normCorr(autoCorr( corr_spec, TempoABuffer->getCurrentPtrFront(), TempoMaxShift, TempoABuffer->N_draw_frames ),TempoMaxShift);
				expUpdate( TempoAcorr, corr_spec, TempoMaxShift, current_time_second>TempoBufferLength?last_time_second:0,
					current_time_second, TempoHalfLife );
			}
		}

		// Add Energy data
		if( n_added ){
			register FLOAT	lin_energy = 0;
			register FLOAT	log_energy = 0;
			for( int x=0; x<FFTSIZE; x++ ){
				log_energy += curFFT[x];
				lin_energy += exp(curFFT[x]*0.1f);
			}
			TempoEBuffer->AddFrame( current_time_second, lin_energy/FFTSIZE );
			b_added = BeltEBuffer->AddFrame( current_time_second, log_energy/FFTSIZE );
			if( bNeed ){
				normCorr(autoCorr( corr_spec, TempoEBuffer->getCurrentPtrFront(), TempoMaxShift, TempoPrecision*TempoBufferLength ),TempoMaxShift);
				expUpdate( TempoEcorr, corr_spec, TempoMaxShift, current_time_second>TempoBufferLength?last_time_second:0,
					current_time_second, TempoHalfLife );
			}
		}
		// Add Energy Derivative data and do autocorrelation
		if( n_added ){
			register FLOAT	rms_energy = 0;

			for(int x=0;x<FFTSIZE;x++){
				register FLOAT fval = (curFFT[x]-last1FFT[x]);
				if(fval>0) 
					rms_energy += fval*fval;
			}
			int x=BeltEDBuffer->AddFrame( current_time_second, (FLOAT)(rms_energy*0.00390625/FFTSIZE) );
			if( x<b_added ) b_added=x;
			TempoEDBuffer->AddFrame( current_time_second, (FLOAT)(rms_energy*0.00390625) );
			if( bNeed ){
				normCorr(autoCorr( corr_spec, TempoEDBuffer->getCurrentPtrFront(), TempoMaxShift, TempoPrecision*TempoBufferLength ),TempoMaxShift);
				expUpdate( TempoEDcorr, corr_spec, TempoMaxShift, current_time_second>TempoBufferLength?last_time_second:0,
					current_time_second, TempoHalfLife );
			}
		}

		// Add Energy Derivative's Derivative data and do autocorrelation
		if( n_added && bNeed ){
			FLOAT	rms_energy = 0;
			for(int x=0;x<FFTSIZE;x++){
				register FLOAT fval = (last1FFT[x]*2-curFFT[x]-last2FFT[x]);
				if(fval>0)
					rms_energy += fval*fval;
			}
			TempoEDDBuffer->AddFrame( current_time_second, rms_energy*0.00390625f/FFTSIZE );
			normCorr(autoCorr( corr_spec, TempoEDDBuffer->getCurrentPtrFront(), TempoMaxShift, TempoPrecision*TempoBufferLength ),TempoMaxShift);
			expUpdate( TempoEDDcorr, corr_spec, TempoMaxShift, current_time_second>TempoBufferLength?last_time_second:0,
				current_time_second, TempoHalfLife );
		}

		if( n_added ){
			memcpy( last2FFT, last1FFT, sizeof(last2FFT) );
			memcpy( last1FFT, curFFT, sizeof(last1FFT) );
		}

		// Estimate tempo
		int		tempoInd = 0, tempoChanged = 0, pri_tempoInd;

		// estimate tempo period
		if( n_added ){
			if( bNeed && current_time_second-last_tempo_est_time>CTempoPeriod ){
				FLOAT	f1,f2,f3,f4;
				last_tempo_est_time = current_time_second;

				memset( TempoSpec, 0, sizeof(TempoSpec) );
				addCorr( TempoSpec, TempoAcorr, TempoMaxShift, f1=calcSpecWeightByMaxPeakHeight(TempoAcorr,TempoMaxShift) );
				addCorr( TempoSpec, TempoEcorr,	TempoMaxShift, f2=calcSpecWeightByMaxPeakHeight(TempoEcorr,TempoMaxShift) );
				addCorr( TempoSpec, TempoEDcorr,TempoMaxShift, f3=calcSpecWeightByMaxPeakHeight(TempoEDcorr,TempoMaxShift) );
				addCorr( TempoSpec, TempoEDDcorr,TempoMaxShift,f4=calcSpecWeightByMaxPeakHeight(TempoEDDcorr,TempoMaxShift) );

				getPeakSpectrum( TempoSpecP, TempoSpec, TempoMaxShift );

				for( int x=1; x<TempoMaxShift; x++ ){
					if( !TempoSpecP[x] ) continue;
					FLOAT fval = sqrt(TempoSpecP[x]);
					fval = (FLOAT)hypot( fval, f1*calcPeakHeight( TempoAcorr, TempoMaxShift, (FLOAT)x ) );
					fval = (FLOAT)hypot( fval, f2*calcPeakHeight( TempoEcorr, TempoMaxShift, (FLOAT)x ) );
					fval = (FLOAT)hypot( fval, f3*calcPeakHeight( TempoEDcorr, TempoMaxShift, (FLOAT)x ));
					fval = (FLOAT)hypot( fval, f4*calcPeakHeight( TempoEDDcorr, TempoMaxShift, (FLOAT)x ));
					TempoSpecP[x] = (float)(fval*ExpWindowFunc[x]*ExpWindowFunc[TempoMaxShift-x]);
				}

				{// use offset, avoid scanning through all tempo values
					int	offset  = (int)(TempoMinPeriod*TempoPrecision+0.5);
					pri_tempoInd= findMax(&TempoSpecP[offset],TempoMaxShift-offset)+offset;
					tempoInd	= adjustTempo( pri_tempoInd+interPeakPosi(&TempoSpec[pri_tempoInd]),
								(int)(TempoMinPeriod*TempoPrecision+0.5),(int)(TempoMaxPeriod*TempoPrecision+0.5),
								TempoSpec, TempoSpecP, TempoWindowFunc, TempoWindowFunc );
				}

				if( abs(tempoInd-last_tempo_index)>1 ){				// change tempo
					if( isHarmonicMultiple(pri_tempoInd,last_pri_tempo_index) ){
						if( testLower((FLOAT)max(pri_tempoInd,last_pri_tempo_index),
							(FLOAT)min(pri_tempoInd,last_pri_tempo_index),TempoSpec,TempoMaxShift) )
							if( TempoSpecP[last_tempo_index]*TempoWindowFunc[last_tempo_index]*PhaseEnhTempoMax
								> TempoSpecP[tempoInd]*TempoWindowFunc[tempoInd] ){
								tempoInd = findPeakPosi( TempoSpec, TempoMaxShift, last_tempo_index );
								goto pass;
							}
					}
					if( TempoSpecP[last_pri_tempo_index] > TempoSpecP[pri_tempoInd] ){
						tempoInd = findPeakPosi( TempoSpec, TempoMaxShift, last_tempo_index );
						pri_tempoInd = findPeakPosi( TempoSpec, TempoMaxShift, last_pri_tempo_index );
						goto pass;
					}
					tempo_enhance_factor = 1;
					phase_enhance_factor = 1;
					last_tempo_change_time2 = last_tempo_change_time;
					last_tempo_change_time = current_time_second;
					tempoChanged = 2;
				}else{
pass:
					tempoChanged = last_tempo_index - tempoInd;
					if( (phase_enhance_factor*=pow(TempoEnhPhaseRatio,n_added)) > TempoEnhPhaseMax )
						phase_enhance_factor = TempoEnhPhaseMax;
				}
				last_pri_tempo_index = pri_tempoInd;
				tempoPeriod = (tempoInd+interPeakPosi(&TempoSpec[tempoInd]))/TempoPrecision;
				//tempoMeter	= 2;//getMeter( tempoPeriod*TempoPrecision, TempoSpec, TempoMaxShift );
			}else{
				tempoInd = last_tempo_index;
			}
		}

		if( preset_tempo>0 ){
			tempo_state		= 1;
			tempoInd		= (int)(preset_tempo*TempoPrecision+0.5f);
			tempoPeriod		= preset_tempo;
			tempoMeter		= preset_meter;
			if( tempoInd != last_tempo_index ){
				tempo_period_frac	= (tempoInd-preset_tempo*TempoPrecision);
				tempo_period_step	= (FLOAT)MinTempoPeriodStep*16;
				//assert(abs(tempo_period_frac)<=1);
			}
			phase_enhance_factor = 2.0f;
			if( n_added ) tempoChanged	= (tempoInd==last_tempo_index?0:2);
		}else if( (current_time_second-last_tempo_change_time<TempoStableTime
			&& last_tempo_change_time-last_tempo_change_time2<TempoStableTime)
			|| tempoPeriod<TempoMinPeriod )	tempo_state = 0;
		else tempo_state = 1;

		if( n_added ) last_tempo_index = tempoInd;

		// update tempo phase
		if( n_added && tempoInd>0 ){

			{// update phase buffer
				FLOAT	*pA, *pB;
				int		n_toadd;
				if( tempoChanged ){	
					{// set cosine array
						register FLOAT mul1 = (FLOAT)M_2PI/tempoInd;
						register FLOAT mul2 = (FLOAT)1/PhaseCombFiltSharp;
						for( int x=0; x<tempoInd; x++ )
							CombFilter[x] = (cos(x*mul1)+1)*0.5f*exp(-sin(0.5f*x*mul1)*mul2);
					}
					tempo_period_step	= 1;
					tempo_period_frac	= interPeakPosi(&TempoSpec[tempoInd]);
					tempo_period_acc	= 0;
					if( abs(tempoChanged)==1 ){ // tempo slides, readjust buffer
						FLOAT	tempAccBuf[TempoMaxShift];
						resizeData( tempoInd+tempoChanged, 1, PhaseAccBuf, tempoInd, 1, tempAccBuf );
						memcpy( PhaseAccBuf, tempAccBuf, tempoInd );
						last_phase_posi = (int)((FLOAT)(last_phase_posi%(tempoInd+tempoChanged))*tempoInd/(tempoInd+tempoChanged)+0.5f);
						goto tempo_no_change;
					}else{						// tempo changed significantly, need to recompute phase from begining
						memset( PhaseAccBuf, 0, sizeof(PhaseAccBuf) );
						memset( PhaseAccBuf2, 0, sizeof(PhaseAccBuf2) );
						pA = TempoEDBuffer->getCurrentPtrFront();
						pB = TempoABuffer->getCurrentPtrFront();
						n_toadd = TempoABuffer->N_draw_frames;
						last_phase_posi = 0;
					}
				}else{
tempo_no_change:
					pA = TempoEDBuffer->getCurrentPtrBack()-n_added+1;
					pB = TempoABuffer->getCurrentPtrBack()-n_added+1;
					n_toadd = n_added;
				}

				for( int x=0,y=0; x<n_toadd; x++,last_phase_posi++ ){
					if( !(last_phase_posi%tempoInd) ){								// do exponential decay
						if( y ) { y=0; continue; }
						while( tempo_period_acc>1 ){
							tempo_period_acc --;
							if( tempo_period_frac>0 ){
								last_phase_posi--;
								y = 1;
							}
							else last_phase_posi++;
						}
						tempo_period_acc += abs(tempo_period_frac);
					//	expUpdate( PhaseAccBuf, PhaseAccBuf2, tempoInd, 
					//		current_time_second-(FLOAT)tempoInd/TempoPrecision, current_time_second, PhaseHalfLife );
						expUpdate( PhaseAccBuf, PhaseAccBuf2, tempoInd, 
							current_time_second-(FLOAT)tempoInd/TempoPrecision, current_time_second, (FLOAT)tempoInd*16/TempoPrecision );
						memset( PhaseAccBuf2, 0, sizeof(PhaseAccBuf2) );
					}

					register FLOAT fval = (FLOAT) hypot(pA[x],pB[x]);
					//assert( tempoInd<TempoMaxShift );
					for( int y=0; y<tempoInd; y++ )
						PhaseAccBuf2[y] += fval*CombFilter[(last_phase_posi+y)%tempoInd];
				}// add to phase accumulation buffer
				last_phase_posi %= tempoInd;
			}

			FLOAT	PhaseAccBufP[TempoMaxShift];
			getPeakSpectrum( PhaseAccBufP, PhaseAccBuf, tempoInd );
			int	phaseInd	= findMax( PhaseAccBufP, tempoInd );

			if( PhaseAccBufP[last_phase_index] )
				PhaseAccBufP[last_phase_index] *= phase_enhance_factor;
			else if(last_phase_index){
				PhaseAccBufP[last_phase_index+1] *= phase_enhance_factor;
				PhaseAccBufP[last_phase_index-1] *= phase_enhance_factor;
			}// apply phase_enhance_factor

			int	phaseInd2	= findMax( PhaseAccBufP,tempoInd );

			if( phaseInd == phaseInd2 ){
				int	phaseDiff = PhaseDiff( last_phase_index, phaseInd, tempoInd );
				if( abs(phaseDiff)<2 ){			// allow slow phase sliding due to discrete tempo period
					phase_slide( phaseDiff );
					if( (tempo_enhance_factor*=pow(PhaseEnhTempoRatio,n_added)) > PhaseEnhTempoMax )	// enhance tempo strength
						tempo_enhance_factor = PhaseEnhTempoMax;
				}else	phase_change( phaseInd, n_added );
			}else{
				int	phaseDiff	= PhaseDiff( last_phase_index, phaseInd, tempoInd );
				int	phaseDiff2	= PhaseDiff( last_phase_index, phaseInd2, tempoInd );
				if( abs(phaseDiff)<2 ) phase_slide( phaseDiff );
				else if( abs(phaseDiff2)<2 ){
					phase_slide( phaseDiff2 );
					phaseInd = phaseInd2;
				}else phase_change( phaseInd, n_added );
			}
			tempoPhase = (FLOAT)M_2PI*(last_phase_posi+phaseInd)/tempoInd;
			last_phase_index = phaseInd;
		}else if( pLevels->state==play_state && tempoPeriod>0 )
			tempoPhase += (FLOAT)(M_2PI/FPS/tempoPeriod);

		// Update random camera
		if( n_added ){
			FLOAT timeElapse = abs(last_time_second2-current_time_second);
			if( (abs(tempoPhase)<0.1f && timeElapse>0.9f) || timeElapse>5.0f ){
				while( VectorDot(target_eye,current_eye)>0 )RandomDirnVector( target_eye );
				while( VectorDot(target_up,current_up)>0 )	RandomDirnVector( target_up );
				last_time_second2 = current_time_second;
			}
		}

		// Update belt and sprite
		if( b_added ){
			if( b_added>=BELTDEPTHSIZE ) b_added = BELTDEPTHSIZE;
			FLOAT	*pE	= (FLOAT*)	BeltEBuffer->getCurrentPtrBack();
			FLOAT	*pED= (FLOAT*)	BeltEDBuffer->getCurrentPtrBack();
			FLOAT	slice_thick = BELTLENGTH/(BELTDEPTHSIZE-1);
			{// Delete sprites at the back
				FLOAT	*pSprite	= &SpriteVB[2];
				FLOAT	shift_dist  = (FLOAT)b_added*slice_thick;
				FLOAT	min_z		= BELTTAIL;
				int		n_delete	= 0;
				for( int n=0; n<sprite_count; n++,pSprite+=3 ){
					if( (*pSprite-=shift_dist) < min_z ) n_delete++;
				}
				// Shift buffer upward
				if( n_delete ){
					sprite_count -= n_delete;
					memmove( SpriteVB, &SpriteVB[n_delete*3], sprite_count*sizeof(FLOAT)*3 );
					memmove( SpriteCB, &SpriteCB[n_delete],	sprite_count*sizeof(DWORD) );
				}
			}
			{// For belt
				FLOAT	beltTemp[BELTCIRCUMSIZE*3];
				FLOAT	*pBelt = &BeltVB[(BELTDEPTHSIZE-b_added)*BELTCIRCUMSIZE*3];
				DWORD	*pColor= &BeltCB[(BELTDEPTHSIZE-b_added)*BELTCIRCUMSIZE];
				if( b_added < BELTDEPTHSIZE ){
					int	diff = b_added*BELTCIRCUMSIZE*3;
					for( FLOAT	*pSrc=BeltVB; pSrc<pBelt; pSrc+=3 )
						*(double*)pSrc = *(double*)(pSrc+diff);
					memmove( BeltCB, &BeltCB[b_added*BELTCIRCUMSIZE], (BELTDEPTHSIZE-b_added)*BELTCIRCUMSIZE*sizeof(DWORD) );
					memcpy( beltTemp, &BeltNB[(BELTDEPTHSIZE-1)*BELTCIRCUMSIZE*3], BELTCIRCUMSIZE*3*sizeof(FLOAT) );
					memmove( BeltNB, &BeltNB[b_added*BELTCIRCUMSIZE*3], (BELTDEPTHSIZE-b_added)*BELTCIRCUMSIZE*sizeof(FLOAT)*3 );
				}// Shift buffer when neccessary
				for( int n=0; n<b_added ; n++,belt_lastParity^=1,pE--,pED-- ){
					FLOAT	perturb= BELTPERTURB*(1-exp(-(*pED)*0.5f));
					belt_last_radius = belt_last_radius*BELT_RETENTION + BELTRADIUS*(1-exp(-(*pE)*0.1f))*(1-BELT_RETENTION);
					belt_x_off	= belt_x_off*BELT_RETENTION + perturb*(1-BELT_RETENTION)*cos(tempoPhase);
					belt_y_off	= belt_y_off*BELT_RETENTION + perturb*(1-BELT_RETENTION)*sin(tempoPhase);
					belt_last_color = InterColor( belt_last_color, RandomColor(1), BELT_COLOR_SRC );
					for( int z=0; z<BELTCIRCUMSIZE; z++,pBelt+=3,pColor++ ){
						pBelt[0] = belt_x_off + belt_last_radius*cos(BELT_OFF*((z<<1)+belt_lastParity));
						pBelt[1] = belt_y_off + belt_last_radius*sin(-BELT_OFF*((z<<1)+belt_lastParity));
						*pColor	 = belt_last_color;
					}
					if( sprite_count<MAXSPRITES ){
						if( *(pED-1)>*(pED-2) && *(pED-1)>*pED ){
							SpriteVB[sprite_count*3]	= belt_x_off + belt_last_radius*(FLOAT)M_SQRT1_2*cos(tempoPhase);
							SpriteVB[sprite_count*3+1]	= belt_x_off + belt_last_radius*(FLOAT)M_SQRT1_2*sin(tempoPhase);
							SpriteVB[sprite_count*3+2]	= BELTHEAD-(b_added-n-1)*slice_thick;
							SpriteCB[sprite_count++]	= RandomColor( 2 );
						}
					}// Add a sprite
				}// Add belt rings
				
				if( b_added < BELTDEPTHSIZE ){
					int	offset = (BELTDEPTHSIZE-b_added-1)*BELTCIRCUMSIZE*3;
					ComputeNormal(&BeltVB[offset],&BeltNB[offset],-BELTCIRCUMSIZE,b_added+1);
					FLOAT	*n1=&BeltNB[offset], *n2=beltTemp;
					for(int x=0; x<BELTCIRCUMSIZE; x++,n1+=3,n2+=3){
						VectorAdd(n1,n2,n1);
						VectorNorm(n1);
					}
				}else 
				ComputeNormal(BeltVB,BeltNB,-BELTCIRCUMSIZE,BELTDEPTHSIZE);

				BYTE	*pAlpha = &((BYTE*)BeltCB)[3];
				for( int x=0; x<BELTDEPTHSIZE; x++ ){
					register BYTE alpha = BeltAlpha[x];
					for( int y=0; y<BELTCIRCUMSIZE; y++,pAlpha+=4 ) *pAlpha = alpha;
				}// update belt alpha values
			}
			{// update sprite alpha values
				BYTE	*pAlpha = &((BYTE*)SpriteCB)[3];
				FLOAT	*pSprite = &SpriteVB[2];
				FLOAT	z_min = BELTTAIL, mul = 240.0f/BELTLENGTH;
				for( int x=0; x<sprite_count; x++,pAlpha+=4,pSprite+=3 ){
					*pAlpha = (BYTE)((*pSprite-z_min)*mul+15.5);
				}
			}
		}

		// Update time stamp
		last_time_second	=	current_time_second;
		return	f_added;
	}

	inline void phase_slide( int phaseDiff ){
		if( phaseDiff ){
			if( (tempo_period_step*=(FLOAT)0.5)<MinTempoPeriodStep )
				tempo_period_step = (FLOAT)MinTempoPeriodStep;
			tempo_period_frac -= phaseDiff*tempo_period_step;
			tempo_period_acc = 0;
			last_phase_posi	+= phaseDiff;
		}
	}// phase slide by 1

	inline void phase_change( int phaseInd, int n_added ){
		if( isIntegerMultiple(abs(phaseInd-last_phase_index), last_tempo_index) ){
			if( (tempo_enhance_factor*=pow(PhaseEnhTempoRatio,-n_added)) < (FLOAT)1/PhaseEnhTempoMax )	// suppress tempo strength
				tempo_enhance_factor = (FLOAT)1/PhaseEnhTempoMax;
		}else	tempo_enhance_factor= 1;
	}// phase changed

	void	DrawAll( TimedLevel *pLevels, int n_added ){

		if( n_added>0 ){
			// Shift FFT vertex and normal buffer values
			if( n_added>=N_draw_frames ) n_added=N_draw_frames;
			else{
				int		diff = n_added*verts_per_line*3;
				int		cnt = (N_draw_frames-n_added)*verts_per_line;
				FLOAT	*pData = &FFTVB[1];
				for( int x=0; x<cnt; x++,pData+=3 )	*pData = *(pData+diff);
				memmove( FFTNB, &FFTNB[verts_per_line*n_added*3],
					(N_draw_frames-n_added)*verts_per_line*sizeof(FLOAT)*3 );
			}// move Y values and normal vectors

			{
				// Fill in vertex buffer
				FLOAT	*pSrc = (FLOAT*)((char*)fftBuffer->getCurrentPtrBack()+fftBuffer->frame_size)-1;
				FLOAT	*pDst = &FFTVB[N_draw_frames*verts_per_line*3-2];
				for( int z=0,Z=n_added*verts_per_line; z<Z; ++z,pDst-=3,pSrc-- ){
					*pDst = *pSrc;
				}
			}

			FLOAT	normal_tmp[FFTSIZE*3];
			FLOAT	*norm_start = &FFTNB[(N_draw_frames-n_added-1)*verts_per_line*3];
			FLOAT	*vert_start = &FFTVB[(N_draw_frames-n_added-1)*verts_per_line*3];
			memcpy( normal_tmp, norm_start, verts_per_line*sizeof(FLOAT)*3 );
			ComputeNormal( vert_start, norm_start, verts_per_line, n_added+1 );
			vert_start = normal_tmp;
			for( int x=0; x<verts_per_line; x++,norm_start+=3,vert_start+=3 ){
				VectorAdd(norm_start,vert_start,norm_start);
				VectorNorm(norm_start);
			}// Average the first line normal with the previous frame
		}

		// Start Drawing Graphics
		if( pLevels->state != pause_state ){
			belt_radial_posi += (FLOAT)(M_2PI/(FPS*SceneRotPeriod));
			if( belt_radial_posi >= M_2PI ) belt_radial_posi = fmod(belt_radial_posi,(FLOAT)M_2PI);
			FLOAT	timeElapse = abs((FLOAT)pLevels->timeStamp*1e-7f-last_time_second3);
			center_posi[0] = SPACERADIUS*0.5f*cos(belt_radial_posi);
			center_posi[1] = 0;
			center_posi[2] = SPACERADIUS*0.5f*sin(belt_radial_posi);
			{// update eye and up
				FLOAT	delta_v[3];
				VectorMul( target_eye, CameraSpeed*timeElapse, delta_v );
				VectorAdd( current_eye, delta_v, current_eye );
				VectorNorm( current_eye );
				VectorMul( target_up, CameraSpeed*timeElapse, delta_v );
				VectorAdd( current_up, delta_v, current_up );
				VectorNorm( current_up );
			}
			{// obtain up vector
				FLOAT	eye_posi[3], top_posi[3];
				VectorAdd( center_posi, current_eye, eye_posi );
				VectorAdd( center_posi, current_up, top_posi );
				camera->updateCamera( eye_posi, center_posi, top_posi );
				camera->shiftUp( -(FLOAT)ELEVATIONDIST );
			}
			last_time_second3 = (FLOAT)pLevels->timeStamp*1e-7f;
		}

		// Draw space points
		camera->setCamera();

		glDisable( GL_DEPTH_TEST );
		glEnable( GL_FOG );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		glBindTexture( GL_TEXTURE_2D, texture_names[0] );
		glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, space_attn );
		glPointParameterf( GL_POINT_FADE_THRESHOLD_SIZE, 0.0f );
		glPointParameterf( GL_POINT_SIZE_MAX, SPRITESIZE );
		glPointParameterf( GL_POINT_SIZE_MIN, 0.0f );
		glFogfv( GL_FOG_COLOR, zero_vector );
		glFogi( GL_FOG_MODE, GL_LINEAR );
		glFogf( GL_FOG_DENSITY, log(8.0f)/VIEWDISTANCE );
		glFogf( GL_FOG_START, 0 );
		glFogf( GL_FOG_END, VIEWDISTANCE );
		glColor3ub( 0xff, 0xff, 0xff );
		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 3, GL_FLOAT, 0, SpaceVB );
		glDrawArrays( GL_POINTS, 0, SPACEPOINTS );
		glClear( GL_DEPTH_BUFFER_BIT );
		glDisable( GL_FOG );

		// Draw Sprites
		glMatrixMode( GL_MODELVIEW );
		glTranslatef( center_posi[0], center_posi[1], center_posi[2] );
		glRotatef( belt_radial_posi*180/(FLOAT)M_PI, 0, -1, 0 );

		glEnable( GL_POINT_SPRITE );
		glPointSize( SPRITESIZE );
		glEnable( GL_TEXTURE_2D );
		glTexEnvi( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
		glEnableClientState( GL_COLOR_ARRAY );
		glVertexPointer( 3, GL_FLOAT, 0, SpriteVB );
		glColorPointer( 4, GL_UNSIGNED_BYTE, 0, SpriteCB );
		glDrawArrays( GL_POINTS, 0, sprite_count );
		glClear( GL_DEPTH_BUFFER_BIT );

		glDisable( GL_TEXTURE_2D );
		glDisable( GL_POINT_SPRITE );
		glEnable( GL_DEPTH_TEST );
		//glDisable( GL_BLEND );

		// Setup star light positions
		float	mats[3][16];
		if( tempo_state || g_mode&1 ){
			FLOAT	star_posi[3] = { SPINRADIUS*cos(tempoPhase), SPINRADIUS*sin(tempoPhase), 0 };
			for( int x=0; x<tempoMeter; x++ ){
				glPushMatrix();
				glRotatef( x*360.0f/tempoMeter, 0, 0, 1 );
				glTranslatef( star_posi[0], star_posi[1], star_posi[2] );
				glLightfv( GL_LIGHT0+x, GL_POSITION, zero_vector );
				glGetFloatv( GL_MODELVIEW_MATRIX, mats[x] );
				glPopMatrix();
			}
		}

		// Draw Belt first, requires blending
		setupLight( tempoMeter );
		glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
		glEnableClientState( GL_COLOR_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		glVertexPointer( 3, GL_FLOAT, 0, BeltVB );
		glNormalPointer( GL_FLOAT, 0, BeltNB );
		glColorPointer( 4, GL_UNSIGNED_BYTE, 0, BeltCB );
		glDepthFunc( GL_ALWAYS );
		glDrawElements( GL_LINE_STRIP, belt_prim_count, GL_UNSIGNED_SHORT, BeltIB );
		glDepthFunc( GL_LESS );
		glPopClientAttrib();

		{// Copy wave buffer
			FLOAT	*pDst = &WaveBuffer[2], ratio=(FLOAT)WAVERINGWIDTH/128;
			BYTE	*pSrc = pLevels->waveform[0];
			for( int x=0; x<FFTSIZE; x++, pDst+=3 ) *pDst = ((FLOAT)pSrc[x]-128)*ratio;
		}

		// Draw waveform
		glDisableClientState( GL_COLOR_ARRAY );
		glDisable(GL_LIGHTING);
		glDisable( GL_BLEND );
		if( n_added > 0 )
			wave_last_color = InterColor( wave_last_color, RandomColor(2), BELT_COLOR_SRC );
		glColor3ubv( (BYTE*)&wave_last_color );
		glVertexPointer( 3, GL_FLOAT, 0, WaveBuffer );
		glDrawArrays( GL_LINE_LOOP, 0, FFTSIZE );

		// Draw Spin Stars
		if( tempo_state || g_mode&1 ){
			// max intensity at beat position
			BYTE color =(tempoMeter==2)?(BYTE)(pow(cos(tempoPhase),4.0f)*240+15.5f):
										(BYTE)(pow(cos(tempoPhase*1.5f),2.0f)*240+15.5f);
			glColor3ub( color, color, color );
			glVertexPointer( 3, GL_FLOAT, 0, StarVB );
			glPushMatrix();
			for( int x=0; x<tempoMeter; x++ ){
				glLoadMatrixf(mats[x]);
				glScalef( star_scale, star_scale, star_scale );
				glDrawArrays( GL_TRIANGLES, 0, 72 );
			}
			glPopMatrix();
		}

		// Setup FFT camera
		{
			FLOAT	eyev[3] = { cos((FLOAT)(ELEVATIONANGLE*M_PI/180)), sin((FLOAT)(ELEVATIONANGLE*M_PI/180)), 0 };
			FLOAT	topv[3] = { 0, 10, 0 };
			fft_camera->updateCamera( eyev, zero_vector, topv );
			fft_camera->shiftUp( (FLOAT)ELEVATIONDIST );
			fft_camera->setCamera();
		}
		// Draw FFT
		glEnable(GL_BLEND);
		glEnable(GL_LIGHTING);
		glEnableClientState( GL_COLOR_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		glRotatef( belt_radial_posi*360/(FLOAT)M_2PI, 0, 1, 0 );
		glNormalPointer( GL_FLOAT, 0, FFTNB );
		glColorPointer( 4, GL_UNSIGNED_BYTE, 0, FFTCB );
		glVertexPointer( 3, GL_FLOAT, 0, FFTVB );
		glDisable( GL_DEPTH_TEST );
		glMultiDrawElements( GL_LINE_STRIP, multidraw_count, GL_UNSIGNED_SHORT,
							 (const void**)multidraw_first, fft_prim_count );

		// Draw FFT front when playing
		glDisableClientState( GL_COLOR_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisable( GL_LIGHTING );
		glColor3ub( 0xff, 0xff, 0xff );
		glLineWidth( 2.0f );
		glVertexPointer( 3, GL_FLOAT, 0, &FFTVB[(N_draw_frames-1)*verts_per_line*3] );
		glDrawArrays( GL_LINE_STRIP, 0, verts_per_line );
		glLineWidth( 1.0f );
		//glEnable( GL_DEPTH_TEST );

		glPopClientAttrib();
		glPopAttrib();

		if( g_mode&1 ){

			int	tempo_index = 0;
			if( n_est ){
				tempo_index = (int)(tempoPeriod/hop_size+0.5);
				if( last_tempo_index>0 && last_tempo_index<TempoMaxPeriod*TempoPrecision ){
					DrawSpikeArray( PhaseAccBuf, last_tempo_index, 1.0f/6.0f, 1.5f/6.0f, 0xffff0000, true );
					DrawSpikeArray( CombFilter, last_tempo_index, 2.0f/6.0f, 2.5f/6.0f, 0xffff0000, true );
				}
				for(int x=0; x<n_est;x++){
					DrawSpikeArray( &est_spec[x][1], *(int*)est_spec[x], (float)x/n_est, (float)(x+1)/n_est, 0xff00ff00 );
					pGLwin->glXYPrintf(pGLwin->Width>>1,pGLwin->Height*x/n_est,0x0102,"%f, %f",est_fact[x],est_fact2[x]);
				}
			}else{
				tempo_index = last_tempo_index;
				if( last_tempo_index>0 && last_tempo_index<TempoMaxPeriod*TempoPrecision ){
					DrawSpikeArray( PhaseAccBuf, last_tempo_index, 0.2f, 0.3f, 0xffff0000, true );
					DrawSpikeArray( CombFilter, last_tempo_index, 0.4f, 0.5f, 0xffff0000, true );
				}
				DrawSpikeArray( TempoAcorr, TempoMaxShift, 0.2f, 0.4f, 0xff0000ff );
				DrawSpikeArray( TempoEcorr, TempoMaxShift, 0.4f, 0.6f, 0xff00ff00 );
				DrawSpikeArray( TempoEDcorr, TempoMaxShift, 0.6f, 0.8f, 0xffff0000 );
				DrawSpikeArray( TempoEDDcorr, TempoMaxShift, 0.8f, 1.0f, 0xffffff00 );
				DrawSpikeArray( TempoSpec, TempoMaxShift, 0.0f, 0.2f, 0xff00ffff );
				DrawSpikeArray( TempoSpecP, TempoMaxShift, 0.0f, 0.2f, 0xffffff00, true );
				DrawPointArray( TempoWindowFunc, TempoMaxShift, 0.0f, 0.2f, 0xffffffff );
			}

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);

			if( tempo_index>0 ){
				glLoadIdentity();
				glOrtho(-1,TempoMaxShift,0,1,-1,1);
				glColor3ub(0xff,0,0);
				glBegin(GL_LINES);
				glVertex2i(tempo_index,0);
				glVertex2i(tempo_index,1);
				glEnd();
			}
			if( last_phase_index>0 ){
				glLoadIdentity();
				glOrtho(-1,tempo_index,0,1,-1,1);
				glColor3ub(0,0,0xff);
				glBegin(GL_LINES);
				glVertex2i(last_phase_index,0);
				glVertex2i(last_phase_index,1);
				glEnd();
			}

		}// Draw all buffers
		
	}

	void	setupLight( int n_light ){
		float	_ambient[4], _diffuse[4], _specular[4];
		Vector4Mul( l_ambient,	1.0f/n_light, _ambient );
		Vector4Mul( l_diffuse,	1.0f/n_light, _diffuse );
		Vector4Mul( l_specular,	1.0f/n_light, _specular );

		// Set light properties
		for( int x=0; x<n_light; x++ ){
			GLenum	LIGHT = GL_LIGHT0+x;
			glLightfv(LIGHT,GL_AMBIENT,	_ambient);
			glLightfv(LIGHT,GL_DIFFUSE,	_diffuse);
			glLightfv(LIGHT,GL_SPECULAR,_specular);
			glLightf(LIGHT, GL_CONSTANT_ATTENUATION, attn_const_0);
			glLightf(LIGHT, GL_LINEAR_ATTENUATION, attn_const_1);
			glLightf(LIGHT, GL_QUADRATIC_ATTENUATION, attn_const_2);
			glLightf(LIGHT, GL_SPOT_CUTOFF, 180.0f);
			glEnable( LIGHT );
		}

		// Material properties
		glMaterialfv(GL_FRONT, GL_AMBIENT,	m_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE,	m_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, m_emissive);
		glMaterialf (GL_FRONT, GL_SHININESS, shiny_const);
		glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
		glEnable( GL_COLOR_MATERIAL );
		glEnable( GL_LIGHTING );
	}
};


