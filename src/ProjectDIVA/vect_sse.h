// Vector Arithmetic using SSE, define USESIMD to enable SSE
#include "assert.h"

#define FLOAT float

inline FLOAT *MatrixMulVector4( FLOAT *mat, FLOAT *vin, FLOAT *vout );

/*
inline FLOAT *MatrixMulMatrix4( FLOAT *mat1, FLOAT *mat2, FLOAT *mout ){
	int	x, y;
	for(x=0; x<4; x++){
		vout[x] = 0;
		for(y=0; y<4; y++)	vout[x] += mat[(y<<2)+x]*vin[y];
	}
	return	vout;
}
*/

inline FLOAT VectorLength(float *v);
inline FLOAT VectorDot( float *v1, float *v2 );
inline FLOAT Vector4Dot( float *v1, float *v2 );
inline FLOAT *VectorNorm( float *v );
inline FLOAT *VectorMul( float *vin, float rhs, float *vout );
inline FLOAT *Vector4Mul( float *vin, float rhs, float *vout );
inline float *VectorCross( float *v1, float *v2, float *vOut );
inline float *VectorCrossAdd( float *v1, float *v2, float *vOut );
inline float *VectorSub( float *v1, float *v2, float *vOut );
inline float *Vector4Inter( float *v1, float *v2, float s_factor, float *vout );
inline float *VectorAdd( float *v1, float *v2, float *vOut );
inline float *Vector4Add( float *v1, float v, float *vOut );

// Adapted from MESA implementation of the GLU library
bool InvertMatrix( FLOAT *m, FLOAT *invOut);
inline float* autoCorr( float *data_out, float *data_in, int max_shift, int Corr_window_size );
float* ComputeNormalA( float* pVertex, float* pNormal, int width, int height );
float* ComputeNormalB( float* pVertex, float* pNormal, int width, int height );

#define ComputeNormal ComputeNormalB

//float* (*ComputeNormal)( float* pVertex, float* pNormal, int width, int height );
