#include <windows.h>
#include <iostream>
#include <gl/glu.h>
#include <gl/gl.h>

using namespace std;


inline FLOAT *VectorCopy( FLOAT *vDst, FLOAT *vSrc ){
	memcpy( vDst, vSrc, sizeof(FLOAT)*3 );
	return	vDst;
}

void	Transpose( char *pDst, char *pSrc, int rows, int cols, int eachSize ){
	int	rowSize = rows*eachSize;
	for( int x=0; x<rows; x++ ){
		char *p = &pDst[eachSize*x];
		for( int y=0; y<cols; y++,pSrc+=eachSize,p+=rowSize ){
			memcpy( p, pSrc, eachSize);
		}
	}
}

void	expUpdate( FLOAT *data, FLOAT *newdata, int size, FLOAT prev_time, FLOAT curr_time, FLOAT half_life ){
	if( curr_time==prev_time ) return;
	if( !curr_time ){
		memcpy( data, newdata, sizeof(FLOAT)*size );
	}else{
		FLOAT	f1 = (FLOAT)exp(M_LOG_2*(prev_time-curr_time)/half_life);
		FLOAT	f2 = (FLOAT)exp(-M_LOG_2*curr_time/half_life);
		FLOAT	w1=f1-f2, w2=1-f1, w3=1.0f/(w1+w2);
		for(int x=0; x<size; x++){
			data[x] = (data[x]*w1 + newdata[x]*w2)*w3;
		}// weighted sum
	}
}// Update data array using negative exponential decay law

inline void	RandomBoundVector( FLOAT *vec ){
	do{
		vec[0] = (FLOAT)rand()/RAND_MAX-0.5f;
		vec[1] = (FLOAT)rand()/RAND_MAX-0.5f;
		vec[2] = (FLOAT)rand()/RAND_MAX-0.5f;
		VectorMul( vec, 2.0f, vec );
	}while( VectorLength(vec) > 1.0f );
}// random a unit cube bounded vector {x:[-1,1];y:[-1,1];z:[-1,1]}

inline void	RandomDirnVector( FLOAT *vec ){
	do{
		vec[0] = (FLOAT)rand()/RAND_MAX-0.5f;
		vec[1] = (FLOAT)rand()/RAND_MAX-0.5f;
		vec[2] = (FLOAT)rand()/RAND_MAX-0.5f;
	}while( VectorLength(vec) <1e-4f );
	VectorNorm( vec );
}// random a unit length direction vector

inline DWORD RandomColorNorm(){
	FLOAT	colors[3]={(FLOAT)rand()/RAND_MAX,(FLOAT)rand()/RAND_MAX,(FLOAT)rand()/RAND_MAX};
	VectorNorm( colors );
	return	(((DWORD)(colors[0]*255+0.5f))<<16) | (((DWORD)(colors[1]*255+0.5f))<<8) | ((DWORD)(colors[2]*255+0.5f));
}// generate a random color with unit intensity

inline DWORD RandomColor( int intense=0 ){
	FLOAT	mul = 255.0f/RAND_MAX;
	DWORD	color;
re:
	color = (((DWORD)(rand()*mul+0.5f))<<16) | (((DWORD)(rand()*mul+0.5f))<<8) | ((DWORD)(rand()*mul+0.5f));
	if( intense ){
		if( intense==1 ) return	RandomColorNorm();
		BYTE r=(BYTE)(color&0xff), g=(BYTE)((color>>8)&0xff), b=(BYTE)((color>>16)&0xff);
		BYTE m=max(r,g);
		m = max(m,b);
		if(!m) goto re;
		mul = 255.0f/m;
		color = (((DWORD)(b*mul+0.5f))<<16) | (((DWORD)(g*mul+0.5f))<<8) | ((DWORD)(r*mul+0.5f));
	}// stretch intensity to max
	return	color;
}// generate a random color*/

inline DWORD InterColor( DWORD c1, DWORD c2, float s_factor ){
	register BYTE *colors = (BYTE*)&c1;
	FLOAT	cs1[4] = {colors[0], colors[1], colors[2], colors[3]};
	colors = (BYTE*)&c2;
	FLOAT	cs2[4] = {colors[0], colors[1], colors[2], colors[3]};
	Vector4Inter( cs1, cs2, s_factor, cs1 );
	Vector4Add( cs1, 0.5f, cs1 );
	return	(((DWORD)cs1[3])<<24) | (((DWORD)cs1[2])<<16) | (((DWORD)cs1[1])<<8) | ((DWORD)cs1[0]);
}// Interpolate color, including alpha

inline FLOAT PhaseDiff( FLOAT f1, FLOAT f2 ){
	register FLOAT df = abs(f1-f2);
	return	min( df, (FLOAT)M_2PI-df );
}// absolute phase difference

inline int PhaseDiff( int v1, int v2, int period ){
	register int phaseDiff = v2-v1;
	if( phaseDiff>0 ) return phaseDiff>(period>>1) ? period-phaseDiff : phaseDiff;
	else return (-phaseDiff)>(period>>1) ? period+phaseDiff : phaseDiff;
	
}// return discrete phase difference

inline FLOAT getMin( FLOAT *data, int size ){
	register FLOAT minV = *data;
	register FLOAT *pData= data, *pEnd=&data[size];
	for( ; pData<pEnd; pData++ ){
		if( *pData<minV ) minV = *pData;
	}
	return	minV;
}// Obtain minimum point

inline FLOAT getMax( FLOAT *data, int size ){
	register FLOAT maxV = *data;
	register FLOAT *pData= data, *pEnd=&data[size];
	for( ; pData<pEnd; pData++ ){
		if( *pData>maxV ) maxV = *pData;
	}
	return	maxV;
}// Obtain maximum value

inline FLOAT findMinNon0( FLOAT *data, int size ){
	register FLOAT	minV = FLT_MAX;
	for( int x=0; x<size; x++ ){
		register FLOAT fval = data[x];
		if( fval ) if( fval<minV ) minV = fval;
	}
	return	(minV==FLT_MAX)?0:minV;
}// Obtain minimum non-zero point

inline FLOAT findPeakHeight( FLOAT *data, int size, int posi ){
	register FLOAT	hLeft, hRight;
	register int	x;
	for( x=posi; x>=0 && data[x-1]<data[x]; x-- );
	if( (hLeft=data[posi]-data[x]) <= FLT_MIN ) return 0;
	for( x=posi; x<size && data[x+1]<data[x]; x++ );
	if( (hRight=data[posi]-data[x]) <= FLT_MIN )return 0;
	return	1/(1/hLeft+1/hRight);
}// Obtain the height and width of a peak

inline int findPeakPosi( FLOAT *data, int size, int posi ){
	int iLeft, iRight;
	register int x;
	for( x=posi; x>=0 && data[x-1]>data[x]; x-- );
	iLeft = (x==posi?0:x);
	for( x=posi; x<size && data[x+1]>data[x]; x++ );
	iRight = (x==posi?size:x);
	if( !iLeft && iRight==size ) return	posi;
	return	((posi-iLeft) < (iRight-posi))?iLeft:iRight;
}// Obtain the position of closest peak

inline FLOAT calcPeakHeight( FLOAT *data, int size, FLOAT posi ){
	int x = findPeakPosi( data, size, (int)(posi+0.5f) );
	register FLOAT y = (FLOAT)((posi-x)/(posi*TempoPeakSharp));
	return	findPeakHeight( data, size, x )*exp(-y*y);
}// Obtain normalized peak height value

inline int findMax( FLOAT *data, int size ){
	int	posi = 0;
	register FLOAT	minV = -FLT_MAX;
	for( int x=0; x<size; x++ ){
		if( data[x]>minV ){
			minV = data[x];
			posi = x;
		}
	}
	return	posi;
}// Obtain maximum point

inline void normCorr( FLOAT *data, int size ){
	register FLOAT	maxV = -FLT_MAX;
	register FLOAT	minV = findMinNon0( data, size );

	if( data[0]==minV ) return;
	register FLOAT	scale= (FLOAT)1.0/(data[0]-minV);
	for( int x=1; x<size; x++ ){
		register FLOAT fval = (data[x]-minV)*scale;//*ExpWindowFunc[x];
		data[x]	= fval<0?0:fval;
		if( fval>maxV ) if( data[x]>data[x-1] ) maxV = fval;
	}
	*data = (maxV==-FLT_MAX)?1:maxV;
	//CheckFloat(data,size);
	//*data = 1;
}// Normalize autocorrelation

inline void addCorr( FLOAT *dst, FLOAT *src, int size, FLOAT factor=1 ){
	if(factor>1) __asm int 3
	//if( !CheckFloat( &factor ) ) return;
	if( factor==1 )
		for( int x=0; x<size; x++ ){
			register FLOAT val = src[x];//*corr_scale;
			dst[x] += val*val;
		}
	else{
		factor *= factor;
		for( int x=0; x<size; x++ ){
			register FLOAT val = src[x];//*corr_scale;
			dst[x] += val*val*factor;
		}
	}
}// Add correlation spectrum

inline void getPeakSpectrum( FLOAT *dst, FLOAT *src, int size ){
	memset( dst, 0, sizeof(FLOAT)*size );
	for( register int x=1,y=size-1; x<y; x++ )
		if( src[x]>src[x-1] && src[x]>src[x+1] ) dst[x] = src[x];
}// Get peak spectrum, non-peak position = 0

inline FLOAT calcSpecWeightByMaxPeakHeight( FLOAT *data, int size ){
	FLOAT	maxPeakHeight = 0, peakVal, lowVal=*(data+1), fval;
	register FLOAT	*pData=data+2, *pMax=&data[size];
re:
	// reach first maxima
	for( ; pData<pMax && *pData>=*(pData-1); pData++ );
	peakVal = *(pData-1);
	// reach first nonzero minima
	for( ; pData<pMax && *pData<=*(pData-1); pData++ );
	{
		FLOAT	hLeft = peakVal-lowVal;
		FLOAT	hRight= peakVal-*(pData-1);
		if( hLeft<=FLT_MIN || hRight<=FLT_MIN ) fval = 0;
		else{
			fval = 1/(1/hLeft+1/hRight);
			if(fval>maxPeakHeight) maxPeakHeight = fval;
		}
		lowVal = *(pData-1);
	}
	if( pData<pMax ) goto re;
	return	(*data)>FLT_MIN ? maxPeakHeight/(*data): maxPeakHeight;
}

inline FLOAT interPeakPosi( FLOAT *middle ){
	FLOAT	left = *middle-*(middle-1);
	FLOAT	right= *middle-*(middle+1);
	FLOAT	sum = left+right;
	if( abs(sum)<=FLT_MIN ) return 0;
	if( left<0 ) return	-0.5f;
	if( right<0 ) return 0.5f;
	return	(FLOAT)(left/sum-0.5);
}// interpolate peak position

FLOAT* resizeData(	int srcWidth,int srcHeight,FLOAT *srcData,
			  		int dstWidth,int dstHeight,FLOAT *dstData	){
	if(srcWidth<=0 || srcHeight<=0 || dstWidth<=0 || dstHeight<=0)return dstData;
	if(srcData==NULL) return dstData;
	if(dstData==NULL) dstData = new FLOAT [dstWidth*dstHeight];

	int		x, y, posi;
	FLOAT	xStart, xEnd, yStart, yEnd, dx, dy, bandRadio;
	FLOAT	xRadio	=	(FLOAT)srcWidth/dstWidth;
	FLOAT	yRadio	=	(FLOAT)srcHeight/dstHeight;

	for(y=0; y<dstHeight; y++)for(x=0; x<dstWidth; x++){
		xStart	=	xRadio*x;
		xEnd	=	xRadio+xStart;
		yStart	=	yRadio*y;
		yEnd	=	yRadio+yStart;
		posi	=	y*dstWidth+x;
		dstData[posi]	=	0;
		for(dy=yStart; dy-yEnd<-0.01; floor(dy)==dy?dy++:dy=ceil(dy)){
			if(floor(dy)==dy)
				bandRadio = (dy+1>yEnd?yEnd-dy:1)/(yEnd-yStart);
			else
				bandRadio = (ceil(dy)>yEnd?yEnd-dy:ceil(dy)-dy)/(yEnd-yStart);
			for(dx=xStart; dx-xEnd<-0.01; floor(dx)==dx?dx++:dx=ceil(dx)){
				if(floor(dx)==dx)dstData[posi]+=bandRadio*srcData[(int)dy*srcWidth+(int)dx]
					*((dx+1>xEnd?xEnd-dx:1)/(xEnd-xStart));
				else	dstData[posi]+=bandRadio*srcData[(int)dy*srcWidth+(int)dx]
					*((ceil(dx)>xEnd?xEnd-dx:ceil(dx)-dx)/(xEnd-xStart));
			}
		}//-0.01 instead of 0 to prevent underflow.
	}
	return dstData;
}// Resize array using interpolation

inline bool	isIntegerMultiple( int i1, int i2 ){
	if( !i1 || !i2 ) return	false;
	if( i2 > i1 ) swap( i1, i2 );
	int		quot = (int)((float)i1/i2+0.5f);
	float	f2 = (float)i1/quot;
	return	abs(f2-i2)/(f2+i2) < 0.0625f/sqrt((FLOAT)quot);
}// Whether i1(i2) is an integer multiple of i2(i1)

inline bool	isHarmonicMultiple( int i1, int i2 ){
	if( !i1 || !i2 ) return	false;
	if( i2 > i1 ) swap( i1, i2 );
	int		quot = (int)((float)i1/i2+0.5f);
	float	f2 = (float)i1/quot;
	if( abs(f2-i2)/(f2+i2) > 0.0625f/sqrt((FLOAT)quot) ) return	false;
	while( !(quot&1) ) quot>>=1;
	while( !(quot%3) ) quot/=3;
	return	quot==1;
}// Whether i1(i2) is an harmonic integer (no factor other than 2&3) multiple of i2(i1)

int	getMeter( FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous=NULL ){
	FLOAT	level3, level21, level22;
	level3	= (calcPeakHeight( spec, size, pri_tempo*0.333333f )
			 +calcPeakHeight( spec, size, pri_tempo*0.666666f) )/2;
	level21	= calcPeakHeight( spec, size, pri_tempo*0.5f );
	level22	= (calcPeakHeight( spec, size, pri_tempo*0.25f )
			 +calcPeakHeight( spec, size, pri_tempo*0.75f ))/2;
	if( bAmbiguous ){
		if( abs(level3-level21)/max(level3,level21)	< MeterAmbiThreshold*2 )
			*bAmbiguous |= 1;
		if( abs(level3-level22)/max(level3,level22) < MeterAmbiThreshold*2 )
			*bAmbiguous |= 2;
	}
	if( pri_tempo<CTempoPeriod*TempoPrecision ){
		FLOAT	factor = CTempoPeriod*TempoPrecision/pri_tempo;
		return	(level3>level21*factor && level3>level22*factor)?3:2;
	}
	return	(level3>level21 && level3>level22)?3:2;
}// determine whether it's a 3/3 meter

int	getOuterMeter( FLOAT pri_tempo, FLOAT *spec, int size, int *bAmbiguous=NULL ){
	FLOAT	level3, level2, level4;
	level2	= calcPeakHeight( spec, size, pri_tempo*2 );
	level3	= calcPeakHeight( spec, size, pri_tempo*3 );
	level4	= calcPeakHeight( spec, size, pri_tempo*4 );
	if( bAmbiguous ){
		if( abs(level3-level2)/max(level3,level2) < MeterAmbiThreshold
			&& abs(level3-level4)/max(level3,level4) < MeterAmbiThreshold )
			*bAmbiguous = 1;
	}
	if( pri_tempo<CTempoPeriod*TempoPrecision ){
		FLOAT	factor = CTempoPeriod*TempoPrecision/pri_tempo;
		return	(level3>level2*factor && level3>level4*factor)?3:2;
	}
	return	(level3>level2 && level3>level4)?3:2;
}// determine whether it's a 3/3 meter

bool testLower( FLOAT pri_tempo, FLOAT lo_tempo, FLOAT *TempoSpec, int size ){
	int		bAmbi;
	FLOAT	cur_tempo = pri_tempo;
	if(abs(cur_tempo-lo_tempo)/lo_tempo<0.0625f) return true;
	while( cur_tempo>lo_tempo ){
		bAmbi = 0;
		cur_tempo /= getMeter(cur_tempo,TempoSpec,size,&bAmbi);
		if( bAmbi )	if( cur_tempo<2*CTempoPeriod*TempoPrecision || !(bAmbi&1) ) break;
		int	new_tempo = findPeakPosi(TempoSpec,size,(int)(cur_tempo+0.5f));
		if(abs(cur_tempo-new_tempo)/cur_tempo>0.0625f)
			return	false;
		cur_tempo = (FLOAT)new_tempo;
		if(abs(cur_tempo-lo_tempo)/lo_tempo<0.0625f) return true;
	}
	return	false;
}// test whether can goto lower harmonics

bool testUpper( FLOAT pri_tempo, FLOAT hi_tempo, FLOAT *TempoSpec, int size ){
	int		bAmbi;
	FLOAT	cur_tempo = pri_tempo;
	if(abs(cur_tempo-hi_tempo)/hi_tempo<0.0625f) return true;
	while( cur_tempo<hi_tempo ){
		if( cur_tempo*4>=size ) break;
		bAmbi = 0;
		cur_tempo *= getOuterMeter(cur_tempo,TempoSpec,size,&bAmbi);
		if( bAmbi ) break;
		int	new_tempo = findPeakPosi(TempoSpec,size,(int)(cur_tempo+0.5f));
		if(abs(cur_tempo-new_tempo)/cur_tempo>0.0625f)
			return	false;
		cur_tempo = (FLOAT)new_tempo;
		if(abs(cur_tempo-hi_tempo)/hi_tempo<0.0625f) return true;
	}
	return	false;
}// test whether can goto upper harmonics

inline int	adjustTempo( float pri_tempo, int min_tempo, int max_tempo,
						FLOAT *TempoSpec, FLOAT *PeakSpec,
						FLOAT *Window2, FLOAT *Window3 ){
	assert( pri_tempo>0 && pri_tempo<max_tempo );

	float	best_tempo = pri_tempo;
	float	new_val, cur_val = TempoSpec[(int)(pri_tempo+0.5f)]*
		(getMeter(pri_tempo,TempoSpec,max_tempo)==2?Window2[(int)(pri_tempo+0.5f)]:Window3[(int)(pri_tempo+0.5f)]);

	//float	window = getMeter(pri_tempo,PeakSpec,max_tempo)==2?Window2[pri_tempo]:Window3[pri_tempo];
	for( int x=min_tempo,y=(int)(pri_tempo*0.5625f); x<=y; x++ ){
		if( !PeakSpec[x] ) continue;
		if( !isHarmonicMultiple((int)(pri_tempo+0.5f),x) ) continue;
		new_val = (getMeter( (FLOAT)x, TempoSpec, max_tempo )==2?Window2[x]:Window3[x])*TempoSpec[x];
		if( new_val > cur_val ){
			if( !testLower((FLOAT)pri_tempo,(FLOAT)x,TempoSpec,max_tempo) ) continue;
			cur_val = new_val;
			best_tempo = (FLOAT)x;
		}
	}
	for( int x=(int)(pri_tempo*1.875f); x<max_tempo; x++ ){
		if( !PeakSpec[x] ) continue;
		if( !isHarmonicMultiple(x,(int)(pri_tempo+0.5f)) ) continue;
		new_val = (getMeter( (FLOAT)x, TempoSpec, max_tempo )==2?Window2[x]:Window3[x])*TempoSpec[x];
		if( new_val > cur_val ){
			if( !testUpper((FLOAT)pri_tempo,(FLOAT)x,TempoSpec,max_tempo) ) continue;
			cur_val = new_val;
			best_tempo = (FLOAT)x;
		}
	}

	return	(int)(best_tempo+0.5f);
}// adjust tempo index according to perceptual window

void	DrawSpikeArray( FLOAT*vals, int size, float Vstart, float Vend, DWORD color=0xffffffff, bool zeroMin=false ){
	FLOAT	fmin=*vals, fmax=*vals, fval;
	FLOAT	*data = new FLOAT [size*sizeof(FLOAT)*4], *pDst=data, *pEnd=&data[size*sizeof(FLOAT)*4];
	for( int x=0; x<size; x++ ){
		*pDst = (FLOAT)x;
		pDst += 2;
		*pDst++ = (FLOAT)x;
		*pDst++ = fval = vals[x];
		if( fval > fmax ) fmax = fval;
		else if( fval < fmin ) fmin = fval;
	}
	if( zeroMin ) fmin=0;
	for( pDst=&data[1]; pDst<pEnd; pDst+=4 ) *pDst = fmin;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,size,fmin-(fmax-fmin)*(1-Vend)/(Vend-Vstart),fmax+(fmax-fmin)*Vstart/(Vend-Vstart),-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, data );
	glColor4ubv((BYTE*)&color);
	glDrawArrays( GL_LINES, 0, size*2 );
	glPopClientAttrib();

	delete [] data;
	//for( int x=0;x<size;x++ ) if( vals[x]<0 ) __asm int 3
}

void	DrawPointArray( FLOAT*vals, int size, float Vstart, float Vend, DWORD color=0xffffffff, bool zeroMin=false ){
	FLOAT	fmin=*vals, fmax=*vals;
	FLOAT	*data = new FLOAT [size*sizeof(FLOAT)*2], *pEnd=&data[size*sizeof(FLOAT)*2];
	register FLOAT fval, *pDst=data;

	for( int x=0; x<size; x++ ){
		*pDst++ = (FLOAT)x;
		*pDst++ = fval = vals[x];
		if( fval > fmax ) fmax = fval;
		else if( fval < fmin ) fmin = fval;
	}
	if( zeroMin ) fmin=0;
	for( pDst=&data[1]; pDst<pEnd; pDst+=4 ) *pDst = fmin;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1,size,fmin-(fmax-fmin)*(1-Vend)/(Vend-Vstart),fmax+(fmax-fmin)*Vstart/(Vend-Vstart),-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, data );
	glColor4ubv((BYTE*)&color);
	glDrawArrays( GL_POINTS, 0, size );
	glPopClientAttrib();

	delete [] data;
	//for( int x=0;x<size;x++ ) if( vals[x]<0 ) __asm int 3
}

//#pragma warning(disable:4244)
void FFT(float* s, int invert)
{
   int ii,jj,n,nn,limit,m,j,inc,i;
   float wx,wr,wpr,wpi,wi,theta;
   float xre,xri,x;
   
   n=*(int*)s;
   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j]; xri = s[j + 1];
         s[j] = s[i];  s[j + 1] = s[i + 1];
         s[i] = xre; s[i + 1] = xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = M_2PI / limit;
      if (invert) theta = -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta); 
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j] - wi * s[j + 1];
            xri = wr * s[j + 1] + wi * s[j];
            s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
            s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (invert)
      for (i = 1;i<=n;i++) 
         s[i] = s[i] / nn;
   
}

void Realft (float* s)
{
   int n, n2, i, i1, i2, i3, i4;
   float xr1, xi1, xr2, xi2, wrs, wis;
   float yr, yi, yr2, yi2, yr0, theta, x;

   n=*(int*)s / 2; n2 = n/2;
   theta = PI / n;
   FFT(s, FALSE);
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; wis = yi;
      xr1 = (s[i1] + s[i3])/2.0; xi1 = (s[i2] - s[i4])/2.0;
      xr2 = (s[i2] + s[i4])/2.0; xi2 = (s[i3] - s[i1])/2.0;
      s[i1] = xr1 + wrs * xr2 - wis * xi2;
      s[i2] = xi1 + wrs * xi2 + wis * xr2;
      s[i3] = xr1 - wrs * xr2 + wis * xi2;
      s[i4] = -xi1 + wrs * xi2 + wis * xr2;
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = s[1];
   s[1] = xr1 + s[2];
   s[2] = 0.0;
}
//#pragma warning( enable:4244 )

class	LoopBuffer{
	// All variables set to public for performance
public:
	int		N_total_frames;
	int		N_draw_frames;
	int		frame_size;
	int		current_frame;		// inclusive
	int		draw_size;
	int		data_size;
	int		status;				// 0:playing or stopped, 1:paused, 2:unpaused but buffering
	FLOAT	time_factor;
	FLOAT	time_stamp;

	char	*data;				// main data buffer
	char	*last_frame_data;

	// Functions

	int	AddFrame( FLOAT timeStamp, FLOAT *currFrame ){
		FLOAT	lastStamp = time_stamp;
		time_stamp = timeStamp;

		if( lastStamp == time_stamp ){
			memcpy( last_frame_data, currFrame, frame_size );
			return	0;
		}

		FLOAT	*lastFrame = (FLOAT*)last_frame_data;
		FLOAT	lastTime=lastStamp*time_factor, currTime=timeStamp*time_factor;
		int		lastIndex=(int)lastTime, currIndex=(int)currTime;
		for( int index=lastIndex+1; index<=currIndex; index++ ){
			FLOAT	b=(index-lastTime)/(currTime-lastTime), a=1.0f-b;
			if( ++current_frame == N_total_frames ){
				if(draw_size)
					memmove( &data[frame_size], &data[data_size-draw_size+frame_size], draw_size-frame_size );
				current_frame = N_draw_frames;
			}// scroll back
			FLOAT	*pDst = (FLOAT*)&data[frame_size*current_frame];
			for( int y=0,z=frame_size/sizeof(FLOAT); y<z; y++ ){
				pDst[y] = lastFrame[y]*a+currFrame[y]*b;
			}// do linear interpolation
		}
		memcpy( lastFrame, currFrame, frame_size );

		if(N_draw_frames>1)
			return	(currIndex-lastIndex)%N_draw_frames;
		return	(currIndex-lastIndex);
	}// constant scrolling speed

	int AddFrame( FLOAT timeStamp, FLOAT val ){
		FLOAT	lastStamp = time_stamp;
		time_stamp = timeStamp;

		if( lastStamp == time_stamp ){
			*(FLOAT*)last_frame_data = val;
			return	0;
		}

		FLOAT	lastTime=lastStamp*time_factor, currTime=timeStamp*time_factor;
		int		lastIndex=(int)lastTime, currIndex=(int)currTime;
		FLOAT	fstart = *(FLOAT*)last_frame_data;
		FLOAT	finc = (val-fstart)/(currTime-lastTime);
		int		last_frame = current_frame;
		fstart += finc*(lastIndex+1-lastTime);

		for( int index=lastIndex+1; index<=currIndex; index++ ){
			if( ++current_frame == N_total_frames ){
				if(draw_size) 
					memmove( &data[frame_size], &data[data_size-draw_size+frame_size], draw_size-frame_size );
				current_frame = N_draw_frames;
			}// scroll back
			((FLOAT*)data)[current_frame] = fstart;
			fstart += finc;
		}
		*(FLOAT*)last_frame_data = val;
		if(N_draw_frames>1)
			return	(currIndex-lastIndex)%N_draw_frames;
		return	currIndex-lastIndex;
	}


	LoopBuffer(	int		_N_total_frames,
				int		_N_draw_frames,
				int		_frame_size,
				FLOAT	_fps	){
		ResizeBuffer( _N_total_frames, _N_draw_frames, _frame_size, _fps );
	}
	~LoopBuffer(){
		if( data ) free( data );
	}
	void ResizeBuffer(	int		_N_total_frames,
						int		_N_draw_frames,
						int		_frame_size,
						FLOAT	_fps ){
		memset( this, 0, sizeof(LoopBuffer) );
		N_total_frames	= _N_total_frames;
		N_draw_frames	= _N_draw_frames;
		frame_size		= _frame_size;
		time_factor		= _fps;
		data			= (char*)realloc( data, N_total_frames*frame_size );
		last_frame_data	= (char*)realloc( last_frame_data, frame_size );
		Reset();
	}
	void	Reset(){
		current_frame	= N_draw_frames-1;
		draw_size		= N_draw_frames*frame_size;
		data_size		= N_total_frames*frame_size;
		status			= 0;
		time_stamp		= 0;
		memset( data, 0, data_size );
		memset( last_frame_data, 0, frame_size );
	}
	FLOAT	*getCurrentPtrFront(){
		return	(FLOAT*)&data[frame_size*(current_frame-N_draw_frames+1)];
	}
	FLOAT	*getCurrentPtrBack(){
		return	(FLOAT*)&data[frame_size*current_frame];
	}
};


class	Camera{
public:
	GLwindow *GLmain;
	FLOAT	eyev[3], centerv[3], upv[3], rightv[3];
	FLOAT	viewAngle;
	FLOAT	viewMat[16], projMat[16];

	Camera( GLwindow *pWin ){
		GLmain = pWin;
		reset();
	}
	~Camera(){}

	void reset(){
		GLwindow *pGLwin=GLmain;
		memset( this, 0, sizeof(Camera) );
		GLmain	= pGLwin;
		eyev[2] = 1.0f;
		upv[1]	= 1.0f;
		viewAngle = 45.0f;
		updateCamera( eyev, centerv, upv );
	}
	void setCamera(){
		glPushAttrib( GL_TRANSFORM_BIT );
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixf( viewMat );
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixf( projMat );
		glPopAttrib();
	}
	void updateCamera( FLOAT *eyev, FLOAT *objv, FLOAT *topv ){
		GLfloat	dirv[3];

		glPushAttrib( GL_TRANSFORM_BIT );
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();

		// Compute view matrix
		VectorCopy( this->eyev, eyev );
		VectorCopy( centerv, objv );
		VectorSub( topv, eyev, upv );
		VectorSub( centerv, eyev, dirv );
		VectorCross( dirv, upv, rightv );
		VectorCross( rightv, dirv, upv );
		glLoadIdentity();
		gluLookAt(	eyev[0], eyev[1], eyev[2],
					centerv[0], centerv[1], centerv[2],
					upv[0], upv[1], upv[2] );
		glGetFloatv( GL_MODELVIEW_MATRIX, viewMat );

		// Compute projection matrix
		glLoadIdentity();
		gluPerspective( viewAngle, (GLdouble)GLmain->Width/GLmain->Height, 0.01, VIEWDISTANCE );
		glGetFloatv( GL_MODELVIEW_MATRIX, projMat );

		glPopMatrix();
		glPopAttrib();
	}
	void shiftUp( FLOAT dist ){
		FLOAT	mov[3];
		VectorNorm( upv );
		VectorMul( upv, dist, mov );
		VectorAdd( eyev, mov, eyev );
		VectorAdd( centerv, mov, centerv );
		glPushAttrib( GL_TRANSFORM_BIT );
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();
		gluLookAt(	eyev[0], eyev[1], eyev[2],
					centerv[0], centerv[1], centerv[2],
					upv[0], upv[1], upv[2] );
		glGetFloatv( GL_MODELVIEW_MATRIX, viewMat );
		glPopMatrix();
		glPopAttrib();
	}// Shift camera upward
};


