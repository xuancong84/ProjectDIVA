#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401

// Extra math constants
#define	M_2PI				6.283185307179586476925286766559
#define	M_LOG_2				0.69314718055994530941723212145818

// System Parameters
#define	FFTSIZE			1024
#define	FFTSIZE_2		512
#define	FFTSIZE_4		256
#define	FFTMAX			255
#define	WAVMAX			127
#define	WAVSIZE			1024
#define	WAVSIZE2		2048
#define	WAVSIZE4		4096
#define	WAVSIZE_2		512
#define	FFTHIST			1024
#define	FLOAT			float
#define	RecBufSize		441000				// recording buffer size in BYTES
#define	MINWAVLENGTH	16					// min duration of wave in second for tempo estimation
#define	MAXWAVLENGTH	1024				// max duration of wave in second for tempo estimation
#define	RECORDERSR		44100

// Dynamic tempo-est parameters
#define	TempoPrecision			100																	// ticks per second
#define	TempoBufferLength		8																	// seconds
#define	TempoMinBufferSize		2																	// seconds
#define	TempoStableTime			4.0f																// in seconds
#define	TempoMaxPeriod			6.0f																// in seconds, 10 bpm (exclusive)
#define	TempoMinPeriod			0.2f																// in seconds, 300 bpm(inclusive)
#define	TempoHalfLife			16																	// seconds (tempo pattern memory)
#define	PhaseHalfLife			32																	// seconds (phase pattern memory)
#define	TempoMaxShift			(int)(TempoPrecision*TempoMaxPeriod+0.5)
#define	CTempoPeriod			0.9f																// critical tempo period in second
#define	CTempoEnhFactor			M_E																	// critical tempo enhancement factor
#define	PhaseCombFiltSharp		0.05f																// Pulse width w.r.t 1 period
#define	TempoPeakSharp			0.03125																// Pulse width w.r.t corr spec
#define	TempoEnhPhaseMax		1.4142f																// max tempo strength enhancement factor
#define	PhaseEnhTempoMax		1.4142f																// max tempo strength enhancement factor
#define	TempoEnhPhaseMaxTime	6																	// time for tempo strength enhancement to reach max
#define	PhaseEnhTempoMaxTime	6																	// time for tempo strength enhancement to reach max
#define	TempoEnhPhaseRatio		pow(TempoEnhPhaseMax,1.0f/(TempoEnhPhaseMaxTime*TempoPrecision))	// tempo strength enhancement per phase lock
#define	PhaseEnhTempoRatio		pow(PhaseEnhTempoMax,1.0f/(PhaseEnhTempoMaxTime*TempoPrecision))	// tempo strength enhancement per phase lock
#define	MinTempoPeriodStep		0.015625
#define	MeterAmbiThreshold		0.03125f

// Graphic parameter
#define	DELTA				1.0e-8f
#define	RMSMIN				16
#define	SPACEPOINTS			65536								// number of stars in the space
#define	SPACERADIUS			64									// radius of the space globe
#define	VIEWDISTANCE		(SPACERADIUS*0.8f)					// radius of the space globe
#define	BELTCIRCUMSIZE		12									// no. of line segments on circumference
#define	BELTDEPTHSIZE		64									// no. of layers to store
#define	BELTRADIUS			0.05f								// max radius
#define	BELTPERTURB			BELTRADIUS							// max perturbation
#define	BELTLENGTH			0.7071f								// belt length
#define	BELTHEAD			BELTLENGTH*0.382f					// belt head posi
#define	BELTTAIL			(-BELTLENGTH*0.618f)				// belt tail posi
#define	BELT_FPS			16									// for each belt frame
#define	BELT_RETENTION		0.5f
#define	BELT_COLOR_SRC		0.618f
#define	BELTALPHAMAX		128.0f
#define	MAXSPRITES			32									// max no. of spinning sprites
#define	SPRITESIZE			32.0f								// max size of spinning sprites
#define	SPINRADIUS			(BELTRADIUS*2.1f)					// added radius of spin stars
#define	SPINSTARSCALE		0.0025f								// scale of spin stars
#define	WAVERINGWIDTH		(SPINRADIUS*0.2f)					// width of time waveform ring
#define	STARRADIUS			8.0f								// length of star tips w.r.t cube radius (1.0f)
#define	CameraSpeed			0.03125f							// w.r.t unit sphere per second
#define	SceneRotPeriod		60									// in seconds times average FPS
#define	StarScaleHalfLife	1									// in seconds
//#define	SceneRotPeriod		M_PI*BELTDEPTHSIZE*SPACERADIUS/(BELT_FPS*BELTLENGTH)

#define	FFTDURATION			4									// in seconds
#define	FFTSIDELENGTH		0.3
#define	FFTHEIGHT			0.08
#define	FFTALPHAMAX			192
#define	ELEVATIONANGLE		15.0								// in degrees
#define	ELEVATIONDIST		0.15								// vertical elevation between FFT and Belt
#define	POWERHALFLIFE		4									// in seconds
#define	FFTEXPFACTOR		0.03125f

extern const int	BELT_INC;
extern const FLOAT BELT_OFF;

extern int total_added;

extern GLfloat	space_attn[3];
extern float	zero_vector[4];
extern float	ones_vector[4];
extern float	zero_4vector[4];

// Lighting and material parameters
#define	attn_const_0		1.0f								// attenuation, constant term
#define	attn_const_1		1.5f								// attenuation, linear term
#define	attn_const_2		6.0f								// attenuation, quadratic term
#define	shiny_const			20.0f								// attenuation, quadratic term
extern FLOAT	l_ambient[4];
extern FLOAT	l_diffuse[4];
extern FLOAT	l_specular[4];
extern FLOAT	m_ambient[4];
extern FLOAT	m_diffuse[4];
extern FLOAT	m_specular[4];
extern FLOAT	m_emissive[4];

// For offline estimation
#define	window_size		0.04f			// in seconds
#define	hop_size		0.01f			// in seconds
#define	delta_width		6
#define	nFilterBands	8

#define nTotalParams (nFilterBands+2)
#define nTotalBufs	(4+nFilterBands*3)

/*
typedef struct tagTimedLevel
{
	unsigned char frequency[ 2 ][ 1024 ];
	unsigned char waveform[ 2 ][ 1024 ];
	int state;
	hyper timeStamp;
} 	TimedLevel;

enum PlayerState
{	
	stop_state	= 0,
	pause_state	= 1,
	play_state	= 2
} ;
*/
