// Vector Arithmetic using SSE, define USESIMD to enable SSE
#include "assert.h"
#include "vect_sse.h"
#include <memory.h>

#define USESIMD 1

inline float *MatrixMulVector4( float *mat, float *vin, float *vout ){
#ifdef	USESIMD
	__asm{
		mov			eax,	[mat]
		mov			ebx,	[vin]
		movups		xmm0,	[eax]
		movss		xmm4,	[ebx]
		movups		xmm1,	[eax+16]
		movss		xmm5,	[ebx+4]
		movups		xmm2,	[eax+32]
		movss		xmm6,	[ebx+8]
		movups		xmm3,	[eax+48]
		movss		xmm7,	[ebx+12]
		unpcklps	xmm4,	xmm4
		unpcklps	xmm5,	xmm5
		unpcklps	xmm6,	xmm6
		unpcklps	xmm7,	xmm7
		movlhps		xmm4,	xmm4
		movlhps		xmm5,	xmm5
		movlhps		xmm6,	xmm6
		movlhps		xmm7,	xmm7
		mulps		xmm0,	xmm4
		mulps		xmm1,	xmm5
		mulps		xmm2,	xmm6
		mulps		xmm3,	xmm7
		addps		xmm0,	xmm1
		addps		xmm0,	xmm2
		addps		xmm0,	xmm3
		mov			eax,	[vout]
		movups		[eax],	xmm0
	}
#else
	for( int x=0; x<4; x++ ){
		vout[x] = 0;
		for( int y=0; y<4; y++ ) vout[x] += mat[(y<<2)+x]*vin[y];
	}
#endif
	return	vout;
}

/*
inline float *MatrixMulMatrix4( float *mat1, float *mat2, float *mout ){
	int	x, y;
	for(x=0; x<4; x++){
		vout[x] = 0;
		for(y=0; y<4; y++)	vout[x] += mat[(y<<2)+x]*vin[y];
	}
	return	vout;
}
*/

inline float VectorLength(float *v){
#ifdef USESIMD
	float	res;
	__asm{
		mov			eax,	[v]
		movups		xmm0,	[eax]
		mulps		xmm0,	xmm0
		movhlps		xmm4,	xmm0
		unpcklps	xmm0,	xmm0
		movhlps		xmm1,	xmm0
		addss		xmm0,	xmm1
		addss		xmm0,	xmm4
		sqrtss		xmm0,	xmm0
		movss		[res],	xmm0
	}
	return	res;
#else
	return  (float)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
#endif
}

inline float VectorDot( float *v1, float *v2 ){
#ifdef USESIMD
	float	res;
	__asm{
		mov			eax,	[v1]
		mov			ebx,	[v2]
		movups		xmm0,	[eax]
		movups		xmm1,	[ebx]
		mulps		xmm0,	xmm1
		movhlps		xmm4,	xmm0
		unpcklps	xmm0,	xmm0
		movhlps		xmm1,	xmm0
		addss		xmm0,	xmm1
		addss		xmm0,	xmm4
		movss		[res],	xmm0
	}
	return	res;
#else
	return	v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2];
#endif
}

inline float Vector4Dot( float *v1, float *v2 ){
#ifdef USESIMD
	float	res;
	__asm{
		mov		eax,	[v1]
		mov		ebx,	[v2]
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		mulps	xmm0,	xmm1
		haddps	xmm0,	xmm0
		haddps	xmm0,	xmm0
		movss	[res],	xmm0
	}
	return	res;
#else
	return	v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2];
#endif
}

inline float *VectorNorm( float *v ){
#ifdef USESIMD
	__asm{
		mov			eax,	[v]
		movups		xmm0,	[eax]
		movaps		xmm2,	xmm0
		mulps		xmm0,	xmm0
		movhlps		xmm4,	xmm0
		unpcklps	xmm0,	xmm0
		movhlps		xmm1,	xmm0
		addss		xmm0,	xmm1
		addss		xmm0,	xmm4
		rsqrtss		xmm0,	xmm0
		unpcklps	xmm0,	xmm0
		movlhps		xmm0,	xmm0
		mulps		xmm0,	xmm2
		movlps		[eax],	xmm0
		movhlps		xmm0,	xmm0
		movss		[eax+8],xmm0
	}
	return	v;
#else
	float	len=1.0f/VectorLength(v);
	v[0] *= len;
	v[1] *= len;
	v[2] *= len;
	return	v;
#endif
}

inline float *VectorMul( float *vin, float rhs, float *vout ){
#ifdef USESIMD
	__asm{
		movss		xmm1,	rhs
		mov			eax,	vin
		unpcklps	xmm1,	xmm1
		movups		xmm0,	[eax]
		movlhps		xmm1,	xmm1
		mulps		xmm0,	xmm1
		mov			eax,	vout
		movlps		[eax],	xmm0
		movhlps		xmm0,	xmm0
		movss		[eax+8],xmm0
	}
#else
	vout[0] = vin[0]*rhs;
	vout[1] = vin[1]*rhs;
	vout[2] = vin[2]*rhs;
#endif
	return	vout;
}

inline float *Vector4Mul( float *vin, float rhs, float *vout ){
#ifdef USESIMD
	__asm{
		movss		xmm1,	rhs
		mov			eax,	vin
		unpcklps	xmm1,	xmm1
		movups		xmm0,	[eax]
		movlhps		xmm1,	xmm1
		mulps		xmm0,	xmm1
		mov			eax,	vout
		movups		[eax],	xmm0
	}
#else
	vout[0] = vin[0]*rhs;
	vout[1] = vin[1]*rhs;
	vout[2] = vin[2]*rhs;
	vout[3] = vin[3]*rhs;
#endif
	return	vout;
}

inline float *VectorCross( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	__asm{
		mov		ebx,	[v2]
		mov		eax,	[v1]
		mov		edx,	[vOut]
		movups	xmm3,	[ebx]
		movups	xmm0,	[eax]
		movaps	xmm1,	xmm3
		movaps	xmm2,	xmm0
		psrldq	xmm1,	4
		psrldq	xmm2,	4
		movhps	xmm1,	[ebx]
		movhps	xmm2,	[eax]
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm3
		subps	xmm0,	xmm2
		movss	[edx+8],xmm0
		psrldq	xmm0,	4
		movlps	[edx],	xmm0
	}
#else	
	vOut[0] = v1[1]*v2[2]-v1[2]*v2[1];
	vOut[1] = v1[2]*v2[0]-v1[0]*v2[2];
	vOut[2] = v1[0]*v2[1]-v1[1]*v2[0];
#endif
	return	vOut;
}

inline float *VectorCrossAdd( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	__asm{
		mov		ebx,	[v2]
		mov		eax,	[v1]
		mov		edx,	[vOut]
		movups	xmm3,	[ebx]
		movups	xmm0,	[eax]
		movups	xmm4,	[edx]
		movaps	xmm1,	xmm3
		movaps	xmm2,	xmm0
		psrldq	xmm1,	4
		psrldq	xmm2,	4
		movhps	xmm1,	[ebx]
		movhps	xmm2,	[eax]
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm3
		subps	xmm0,	xmm2
		movss	xmm1,	xmm0
		psrldq	xmm0,	4
		movlhps	xmm0,	xmm1
		addps	xmm0,	xmm4
		movss	[edx],	xmm0
		psrldq	xmm0,	4
		movlps	[edx+4],xmm0
	}
#else	
	vOut[0] = v1[1]*v2[2]-v1[2]*v2[1];
	vOut[1] = v1[2]*v2[0]-v1[0]*v2[2];
	vOut[2] = v1[0]*v2[1]-v1[1]*v2[0];
#endif
	return	vOut;
}

inline float *VectorSub( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	__asm{
		mov		eax,	v1
		mov		ebx,	v2
		mov		edx,	vOut
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		subps	xmm0,	xmm1
		movlps	[edx],	xmm0
		movhlps	xmm0,	xmm0
		movss	[edx+8],xmm0
	}
#else
	vOut[0] = v1[0]-v2[0];
	vOut[1] = v1[1]-v2[1];
	vOut[2] = v1[2]-v2[2];
#endif
	return	vOut;
}

inline float *Vector4Inter( float *v1, float *v2, float s_factor, float *vout ){
#ifdef USESIMD
	float	one = 1;
	__asm{
		movss		xmm2,	s_factor
		movss		xmm3,	one
		unpcklps	xmm2,	xmm2
		unpcklps	xmm3,	xmm3
		movlhps		xmm2,	xmm2
		movlhps		xmm3,	xmm3
		mov			eax,	v1
		mov			ebx,	v2
		movups		xmm0,	[eax]
		movups		xmm1,	[ebx]
		subps		xmm3,	xmm2
		mulps		xmm0,	xmm2
		mulps		xmm1,	xmm3
		addps		xmm0,	xmm1
		mov			eax,	vout
		movups		[eax],	xmm0
	}
#else
	vout[0] = v1[0]*s_factor+v2[0]*(1-s_factor);
	vout[1] = v1[1]*s_factor+v2[1]*(1-s_factor);
	vout[2] = v1[2]*s_factor+v2[2]*(1-s_factor);
	vout[3] = v1[3]*s_factor+v2[3]*(1-s_factor);
#endif
	return	vout;
}

inline float *VectorAdd( float *v1, float *v2, float *vOut ){
#ifdef USESIMD
	__asm{
		mov		eax,	v1
		mov		ebx,	v2
		mov		edx,	vOut
		movups	xmm0,	[eax]
		movups	xmm1,	[ebx]
		addps	xmm0,	xmm1
		movlps	[edx],	xmm0
		movhlps	xmm0,	xmm0
		movss	[edx+8],xmm0
	}
#else
	vOut[0] = v1[0]+v2[0];
	vOut[1] = v1[1]+v2[1];
	vOut[2] = v1[2]+v2[2];
#endif
	return	vOut;
}

inline float *Vector4Add( float *v1, float v, float *vOut ){
#ifdef USESIMD
	__asm{
		mov			eax,	v1
		movss		xmm1,	v
		movups		xmm0,	[eax]
		unpcklps	xmm1,	xmm1
		movlhps		xmm1,	xmm1
		mov			eax,	vOut
		addps		xmm0,	xmm1
		movups		[eax],	xmm0
	}
#else
	vOut[0] = v1[0]+v;
	vOut[1] = v1[1]+v;
	vOut[2] = v1[2]+v;
	vOut[3] = v1[3]+v;
#endif
	return	vOut;
}

// Adapted from MESA implementation of the GLU library
bool InvertMatrix( float *m, float *invOut) 
{ 
	float inv[16], det; 
	int i; 

	inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] 
	+ m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10]; 
	inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] 
	- m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10]; 
	inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] 
	+ m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9]; 
	inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] 
	- m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9]; 
	inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] 
	- m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10]; 
	inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] 
	+ m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10]; 
	inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] 
	- m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9]; 
	inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] 
	+ m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9]; 
	inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] 
	+ m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6]; 
	inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] 
	- m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6]; 
	inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] 
	+ m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5]; 
	inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] 
	- m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5]; 
	inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] 
	- m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6]; 
	inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] 
	+ m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6]; 
	inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] 
	- m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5]; 
	inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] 
	+ m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5]; 

	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12]; 
	if (det == 0) return false; 
	det = (float)1.0 / det; 
	for (i = 0; i < 16; i++) invOut[i] = inv[i] * det; 
 
	return true; 
}

inline float* autoCorr( float *data_out, float *data_in, int max_shift, int Corr_window_size ){
	float	*dataEnd = &data_in[Corr_window_size];
	if( max_shift>Corr_window_size ){
		memset( data_out, 0, sizeof(float)*max_shift );
		max_shift = Corr_window_size;
	}
	for(int x=0; x<max_shift; x++ ){
		register float	*data1	=	data_in;
		register float	*data2	=	data1+x;
		register float	sum		=	0;
		if( data2+4<=data_in ) __asm{
			movss	xmm4,	[sum]
			mov		eax,	[data1]
			unpcklps xmm4,	xmm4
			mov		ebx,	[data2]
			movlhps	xmm4,	xmm4
			mov		ecx,	[dataEnd]
			sub		ecx,	ebx
			shr		ecx,	4
re:
			movups	xmm0,	[eax]
			movups	xmm1,	[ebx]
			add		eax,	16
			mulps	xmm0,	xmm1
			add		ebx,	16
			addps	xmm4,	xmm0
			loop	re

			movaps	xmm0,	xmm4
			unpcklps xmm0,	xmm0
			unpckhps xmm4,	xmm4
			movhlps	xmm1,	xmm0
			mov		[data1],eax
			movhlps	xmm5,	xmm4
			mov		[data2],ebx
			addss	xmm0,	xmm1
			addss	xmm0,	xmm4
			addss	xmm0,	xmm5
			movss	[sum],	xmm0
		}
		assert(data2<=dataEnd);
		for( ; data2<dataEnd; data1++,data2++ )
			sum	+=	(*data1)*(*data2);

		/*/			for( ; data2>=data_in; data1--,data2-- )
		sum	+=	(*data1)*(*data2);
		*/
		data_out[x] = (float)(sum/(Corr_window_size-x));
	}
	return	data_out;
	//CheckFloat( data_out, max_shift );
}// Compute autocorrelation


float* ComputeNormalA( float* pVertex, float* pNormal, int width, int height ){
	// Negative width or height indicate wrap around
	if( height < 2 ) return pNormal;

	bool wrapX = false, wrapY = false;
	if( width<0 ){
		width = -width;
		wrapX = true;
	}
	if( height<0 ){
		height = -height;
		wrapY = true;
	}

	const int	line_size = width*3;
	const int	full_size = line_size*height;
	float	*pNorm = pNormal, *pVert = pVertex;
	float	v1[3], v2[3], v3[3], v4[3], v[3];

	// First line: begin
	VectorSub(pVert+line_size,pVert,v4);
	VectorSub(pVert+3,pVert,v1);
	VectorCross(v4,v1,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert+line_size-3,pVert,v3);
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCross(v1,v2,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// First line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v3,v4,pNorm);
		VectorNorm(pNorm);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapY ){
			VectorSub(pVert-line_size+full_size,pVert,v2);
			VectorCross(v1,v2,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v2,v3,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
	}

	// First line: end
	VectorSub(pVert-3,pVert,v3);
	VectorSub(pVert+line_size,pVert,v4);
	VectorCross(v3,v4,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v1,v2,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;


	// Middle lines: 4 cross products
	for( int y=2; y<height; y++ ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v1,v2,pNorm);
		VectorNorm(pNorm);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapX ){
			VectorSub(pVert-3+line_size,pVert,v3);
			VectorCross(v2,v3,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v3,v4,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;

		for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
			VectorSub(pVert+3,pVert,v1);
			VectorSub(pVert-line_size,pVert,v2);
			VectorSub(pVert-3,pVert,v3);
			VectorSub(pVert+line_size,pVert,v4);
			VectorCross(v1,v2,pNorm);
			VectorNorm(pNorm);

			VectorCross(v2,v3,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);

			VectorCross(v3,v4,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);

			VectorCross(v4,v1,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);

			VectorNorm(pNorm);
		}

		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v2,v3,pNorm);
		VectorNorm(pNorm);
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapX ){
			VectorSub(pVert+3-line_size,pVert,v1);
			VectorCross(v1,v2,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v4,v1,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;
	}


	// Last line: begin
	VectorSub(pVert+3,pVert,v1);
	VectorSub(pVert-line_size,pVert,v2);
	VectorCross(v1,v2,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert-3+line_size,pVert,v3);
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// Last line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorCross(v1,v2,pNorm);
		VectorNorm(pNorm);
		VectorCross(v2,v3,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
		if( wrapY ){
			VectorSub(pVert+line_size-full_size,pVert,v4);
			VectorCross(v3,v4,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
			VectorCross(v4,v1,v);
			VectorNorm(v);
			VectorAdd(pNorm,v,pNorm);
		}
		VectorNorm(pNorm);
	}

	// Last line: end
	VectorSub(pVert-line_size,pVert,v2);
	VectorSub(pVert-3,pVert,v3);
	VectorCross(v2,v3,pNorm);
	VectorNorm(pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCross(v1,v2,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCross(v3,v4,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCross(v4,v1,v);
		VectorNorm(v);
		VectorAdd(pNorm,v,pNorm);
	}
	VectorNorm(pNorm);

	return	pNormal;
}

float* ComputeNormalB( float* pVertex, float* pNormal, int width, int height ){
	// Negative width or height indicate wrap around
	if( height < 2 ) return pNormal;

	bool wrapX = false, wrapY = false;
	if( width<0 ){
		width = -width;
		wrapX = true;
	}
	if( height<0 ){
		height = -height;
		wrapY = true;
	}

	const int	line_size = width*3;
	const int	full_size = line_size*height;
	float	*pNorm = pNormal, *pVert = pVertex;
	float	v1[3], v2[3], v3[3], v4[3];

	// First line: begin
	VectorSub(pVert+line_size,pVert,v4);
	VectorSub(pVert+3,pVert,v1);
	VectorCross(v4,v1,pNorm);
	if( wrapX ){
		VectorSub(pVert+line_size-3,pVert,v3);
		VectorCrossAdd(v3,v4,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCrossAdd(v1,v2,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v2,v3,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// First line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v3,v4,pNorm);
		VectorCrossAdd(v4,v1,pNorm);
		if( wrapY ){
			VectorSub(pVert-line_size+full_size,pVert,v2);
			VectorCrossAdd(v1,v2,pNorm);
			VectorCrossAdd(v2,v3,pNorm);
		}
		VectorNorm(pNorm);
	}

	// First line: end
	VectorSub(pVert-3,pVert,v3);
	VectorSub(pVert+line_size,pVert,v4);
	VectorCross(v3,v4,pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCrossAdd(v4,v1,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert-line_size+full_size,pVert,v2);
		VectorCrossAdd(v2,v3,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v1,v2,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;


	// Middle lines: 4 cross products
	for( int y=2; y<height; y++ ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v1,v2,pNorm);
		VectorCrossAdd(v4,v1,pNorm);
		if( wrapX ){
			VectorSub(pVert-3+line_size,pVert,v3);
			VectorCrossAdd(v2,v3,pNorm);
			VectorCrossAdd(v3,v4,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;

		for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
			VectorSub(pVert+3,pVert,v1);
			VectorSub(pVert-line_size,pVert,v2);
			VectorSub(pVert-3,pVert,v3);
			VectorSub(pVert+line_size,pVert,v4);
			VectorCross(v1,v2,pNorm);
			VectorCrossAdd(v2,v3,pNorm);
			VectorCrossAdd(v3,v4,pNorm);
			VectorCrossAdd(v4,v1,pNorm);
			VectorNorm(pNorm);
		}

		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorSub(pVert+line_size,pVert,v4);
		VectorCross(v2,v3,pNorm);
		VectorCrossAdd(v3,v4,pNorm);
		if( wrapX ){
			VectorSub(pVert+3-line_size,pVert,v1);
			VectorCrossAdd(v1,v2,pNorm);
			VectorCrossAdd(v4,v1,pNorm);
		}
		VectorNorm(pNorm);
		pNorm += 3;
		pVert += 3;
	}


	// Last line: begin
	VectorSub(pVert+3,pVert,v1);
	VectorSub(pVert-line_size,pVert,v2);
	VectorCross(v1,v2,pNorm);
	if( wrapX ){
		VectorSub(pVert-3+line_size,pVert,v3);
		VectorCrossAdd(v2,v3,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCrossAdd(v4,v1,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v3,v4,pNorm);
	}
	VectorNorm(pNorm);
	pNorm += 3;
	pVert += 3;

	// Last line: middle part, 2 cross products
	for( int x=2; x<width; x++,pNorm+=3,pVert+=3 ){
		VectorSub(pVert+3,pVert,v1);
		VectorSub(pVert-line_size,pVert,v2);
		VectorSub(pVert-3,pVert,v3);
		VectorCross(v1,v2,pNorm);
		VectorCrossAdd(v2,v3,pNorm);
		if( wrapY ){
			VectorSub(pVert+line_size-full_size,pVert,v4);
			VectorCrossAdd(v3,v4,pNorm);
			VectorCrossAdd(v4,v1,pNorm);
		}
		VectorNorm(pNorm);
	}

	// Last line: end
	VectorSub(pVert-line_size,pVert,v2);
	VectorSub(pVert-3,pVert,v3);
	VectorCross(v2,v3,pNorm);
	if( wrapX ){
		VectorSub(pVert+3-line_size,pVert,v1);
		VectorCrossAdd(v1,v2,pNorm);
	}
	if( wrapY ){
		VectorSub(pVert+line_size-full_size,pVert,v4);
		VectorCrossAdd(v3,v4,pNorm);
	}
	if( wrapX && wrapY ){
		VectorCrossAdd(v4,v1,pNorm);
	}
	VectorNorm(pNorm);

	return	pNormal;
}

