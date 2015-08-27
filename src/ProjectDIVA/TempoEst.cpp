
const float delta_mul = 3.0f/(delta_width*(delta_width+1)*(2*delta_width+1));

typedef struct{
   int		frameSize;       	/* speech frameSize */
   int		numChans;        	/* number of channels */
   long		sampPeriod;			/* sample period */
   int		fftN;            	/* fft size */
   int		klo,khi;         	/* lopass to hipass cut-off fft indices */
   bool		usePower;    		/* use power rather than magnitude */
   bool		takeLogs;    		/* log filterbank channels */
   float	fres;				/* scaled fft resolution */
   float* 	cf;          	 	/* array[1..pOrder+1] of centre freqs */
   short* 	loChan;				/* array[1..fftN/2] of loChan index */
   float* 	loWt;				/* array[1..fftN/2] of loChan weighting */
   float* 	x;					/* array[1..fftN] of fftchans */
}FBankInfo;

float	*CreateVector( int size ){
	float	*v = new float [(size+1)*sizeof(float)];
	*(int*)v = size;
	return	v;
}
short	*CreateShortVector( int size ){
	short	*v = new short [(size+1)*sizeof(short)];
	*(short*)v = (short)size;
	return	v;
}

float Mel(int k,float fres){
	return 1127 * log(1 + (k-1)*fres);
}


void deltaSpec( float *spec, float *out, int size ){
	for( int x=0; x<size; x++ ){
		if( x+delta_width>=size || x-delta_width<0 ){
			out[x] = 0;
			continue;
		}
		float num = 0;
		for( int y=1; y<=delta_width; y++ ){
			num += (spec[x+y]-spec[x-y])*y;
		}
		out[x] = num*delta_mul;
		if(out[x]<0) out[x] = 0;
	}
}

FBankInfo InitFBank(int frameSize, long sampPeriod, int numChans,
					float lopass, float hipass, bool usePower, bool takeLogs )
{
	FBankInfo fb;
	float mlo,mhi,ms,melk;
	int k,chan,maxChan,Nby2;

	/* Save sizes to cross-check subsequent usage */
	fb.frameSize = frameSize; fb.numChans = numChans;
	fb.sampPeriod = sampPeriod;
	fb.usePower = usePower; fb.takeLogs = takeLogs;
	/* Calculate required FFT size */
	fb.fftN = 2;
	while (frameSize>fb.fftN) fb.fftN *= 2;
	Nby2 = fb.fftN / 2;
	fb.fres = 1.0e7f/(sampPeriod * fb.fftN * 700.0f);
	maxChan = numChans+1;
	/* set lo and hi pass cut offs if any */
	fb.klo = 2; fb.khi = Nby2;       /* apply lo/hi pass filtering */
	mlo = 0; mhi = Mel(Nby2+1,fb.fres);
	if (lopass>=0.0) {
		mlo = 1127.0f*log(1+lopass/700.0f);
		fb.klo = (int) ((lopass * sampPeriod * 1.0e-7 * fb.fftN) + 2.5);
		if (fb.klo<2) fb.klo = 2;
	}
	if (hipass>=0.0) {
		mhi = 1127.0f*log(1+hipass/700.0f);
		fb.khi = (int) ((hipass * sampPeriod * 1.0e-7 * fb.fftN) + 0.5);
		if (fb.khi>Nby2) fb.khi = Nby2;
	}

	/* Create vector of fbank centre frequencies */
	fb.cf = CreateVector(maxChan);
	ms = mhi - mlo;
	for (chan=1; chan <= maxChan; chan++) {
		fb.cf[chan] = ((float)chan/(float)maxChan)*ms + mlo;
	}

	/* Create loChan map, loChan[fftindex] -> lower channel index */
	fb.loChan = CreateShortVector(Nby2);
	for (k=1,chan=1; k<=Nby2; k++){
		melk = Mel(k,fb.fres);
		if (k<fb.klo || k>fb.khi) fb.loChan[k]=-1;
		else {
			while (fb.cf[chan] < melk  && chan<=maxChan) ++chan;
			fb.loChan[k] = chan-1;
		}
	}

	/* Create vector of lower channel weights */
	fb.loWt = CreateVector(Nby2);
	for (k=1; k<=Nby2; k++) {
		chan = fb.loChan[k];
		if (k<fb.klo || k>fb.khi) fb.loWt[k]=0.0;
		else {
			if (chan>0)
				fb.loWt[k] = ((fb.cf[chan+1] - Mel(k,fb.fres)) /
				(fb.cf[chan+1] - fb.cf[chan]));
			else
				fb.loWt[k] = (fb.cf[1]-Mel(k,fb.fres))/(fb.cf[1] - mlo);
		}
	}
	/* Create workspace for fft */
	fb.x = CreateVector(fb.fftN);
	return fb;
}

void Wave2FBank( float* s, float* fbank, float *te, FBankInfo& info )
{
   const float melfloor = 1.0;
   int k, bin;
   float t1,t2;   /* real and imag parts */
   float ek;      /* energy of k'th fft channel */
   float mul=(float)M_PI/(info.frameSize-1);

   /* Compute frame energy if needed */
   if (te != NULL){
      *te = 0.0;
      for (k=1; k<=info.frameSize; k++)
         *te += (s[k]*s[k]);
   }
   /* Apply FFT */
   for (k=1; k<=info.frameSize; k++)
      info.x[k] = s[k]*(sin(mul*(k-1))+1);    /* copy to workspace */
   for (k=info.frameSize+1; k<=info.fftN; k++)
      info.x[k] = 0.0;   /* pad with zeroes */
   Realft(info.x);                            /* take fft */

   /* Fill filterbank channels */
   memset( &fbank[1], 0, *(int*)fbank*sizeof(float) );
   for (k = info.klo; k <= info.khi; k++) {             /* fill bins */
      t1 = info.x[2*k-1]; t2 = info.x[2*k];
      if (info.usePower)
         ek = t1*t1 + t2*t2;
      else
         ek = sqrt(t1*t1 + t2*t2);
      bin = info.loChan[k];
      t1 = info.loWt[k]*ek;
      if (bin>0) fbank[bin] += t1;
      if (bin<info.numChans) fbank[bin+1] += ek - t1;
   }

   /* Take logs */
   if (info.takeLogs)
      for (bin=1; bin<=info.numChans; bin++) {
         t1 = fbank[bin];
         if (t1<melfloor) t1 = melfloor;
         fbank[bin] = log(t1);
      }
}

float	ComputeTempo( float *data, int size, int sr ){	//sr: sampling rate
	const int	winpts = (int)(window_size*sr+0.5);
	const int	hoppts = (int)(hop_size*sr+0.5);
	const int	fftpts = (int)pow(2.0,ceil(log((double)winpts)/log(2.0)));
	const int	nframe = (int)((double)(size-winpts)/hoppts+1.0);
	const int	nfft_2 = fftpts/2;
	float	win_time = (float)winpts/sr;
	float	hop_time = (float)hoppts/sr;
	float	mul = (float)M_PI/(winpts-1);

	// Allocate persistent buffer first
	if(!b_est) for(int x=0;x<nTotalBufs+4;x++){
		est_spec[x] = new float [TempoMaxShift+1];
		*(int*)est_spec[x] = TempoMaxShift;
		b_est = 1;
	}

	FLOAT	TempoCorr[nTotalBufs][TempoMaxShift],
			TempoSpec[TempoMaxShift],
			TempoSpecP[TempoMaxShift],
			Tempo2WindowFunc[TempoMaxShift],
			Tempo3WindowFunc[TempoMaxShift],
			ExpWindowFunc[TempoMaxShift];

	// Initialize filterbank
	FBankInfo fb_info = InitFBank( fftpts, (long)(1e7f/sr+0.5), nFilterBands, 0.0f, 8000.0f, true, false );

	// Initialize window functions
	{
		FLOAT	f2 = 2*CTempoPeriod/hop_size;
		FLOAT	f3 = 3*CTempoPeriod/hop_size;
		for( int x=0; x<TempoMaxShift; x++ ){
			Tempo2WindowFunc[x]	= pow((FLOAT)M_E*x/f2,(FLOAT)f2/x)*x*(exp(-(FLOAT)M_E*x/f2) );
			Tempo3WindowFunc[x]	= pow((FLOAT)M_E*x/f2,(FLOAT)f3/x)*x*(exp(-(FLOAT)M_E*x/f3) );
			ExpWindowFunc[x]	= 1-exp( -pow((FLOAT)M_E*x/(CTempoPeriod/hop_size),2) );
		}
		Tempo2WindowFunc[0] = 0;
		Tempo3WindowFunc[0] = 0;
		f2 = 1.0f/getMax( Tempo2WindowFunc, TempoMaxShift );
		f3 = 1.0f/getMax( Tempo3WindowFunc, TempoMaxShift );
		for( int x=0; x<TempoMaxShift; x++ ){
			Tempo2WindowFunc[x]	*= f2;
			Tempo3WindowFunc[x]	*= f3;
		}
	}

	// Allocate dynamic buffers
	float	fbank[nFilterBands+1];
	float	*AllBuf	= new float [nframe*nTotalParams];
	float	*DifBuf	= new float [nframe];
	float	*fdata	= new float [fftpts+1];

	// Extract all frames
	for( int x=0; x<nframe; x++ ){
		memset( &fdata[1], 0, fftpts*sizeof(float) );
		memcpy( &fdata[1], &data[x*hoppts], winpts*sizeof(float) );

		// get amplitude
		AllBuf[x] = getMax(&fdata[1],winpts)-getMin(&fdata[1],winpts);

		// get filterbank and energy
		*(int*)fdata  = fftpts;
		*(int*)&fbank = nFilterBands;
		Wave2FBank( fdata, fbank, &AllBuf[(nTotalParams-1)*nframe+x], fb_info );
		for( int y=1; y<=nFilterBands; y++ ) AllBuf[y*nframe+x] = fbank[y];
	}

	// Process feature
	memset( TempoSpec, 0, sizeof(TempoSpec) );
	float	fact;
	int		N = 0;
	for( int x=0; x<nTotalParams; x++ ){
		fact = (x==0 || x==(nTotalParams-1))?1.0f:1.0f;
		normCorr(autoCorr( TempoCorr[N], &AllBuf[x*nframe], TempoMaxShift, nframe ),TempoMaxShift);
		addCorr( TempoSpec, TempoCorr[N], TempoMaxShift, est_fact[N]=fact*calcSpecWeightByMaxPeakHeight(TempoCorr[N],TempoMaxShift) );
		N++;
		assert(N<=nTotalBufs);

		if( !x ) continue;	// don't do delta and delta-delta for amplitude

		// add delta spectrum
		deltaSpec( &AllBuf[x*nframe], DifBuf, nframe );
		memcpy( &AllBuf[x*nframe], DifBuf, nframe*sizeof(float) );
		normCorr(autoCorr( TempoCorr[N], &AllBuf[x*nframe], TempoMaxShift, nframe ),TempoMaxShift);
		addCorr( TempoSpec, TempoCorr[N], TempoMaxShift, est_fact[N]=fact*calcSpecWeightByMaxPeakHeight(TempoCorr[N],TempoMaxShift) );
		N++;
		assert(N<=nTotalBufs);

		// add delta-delta spectrum
		deltaSpec( &AllBuf[x*nframe], DifBuf, nframe );
		memcpy( &AllBuf[x*nframe], DifBuf, nframe*sizeof(float) );
		normCorr(autoCorr( TempoCorr[N], &AllBuf[x*nframe], TempoMaxShift, nframe ),TempoMaxShift);
		addCorr( TempoSpec, TempoCorr[N], TempoMaxShift, est_fact[N]=fact*calcSpecWeightByMaxPeakHeight(TempoCorr[N],TempoMaxShift) );
		N++;
		assert(N<=nTotalBufs);
	}

	getPeakSpectrum( TempoSpecP, TempoSpec, TempoMaxShift );
	for( int x=1; x<TempoMaxShift; x++ ){
		if( !TempoSpecP[x] ) continue;
		double	fval = sqrt(TempoSpecP[x]);
		for( int y=0; y<N; y++ )
			fval = hypot( fval, est_fact[y]*calcPeakHeight( TempoCorr[y], TempoMaxShift, (FLOAT)x ) );
		TempoSpecP[x] = (float)(fval*ExpWindowFunc[x]*ExpWindowFunc[TempoMaxShift-x]);
	}

	for( int x=0; x<nTotalBufs; x++ ){
		memcpy(&est_spec[x][1],TempoCorr[x],sizeof(FLOAT)*TempoMaxShift);
	}
	
	memcpy(&est_spec[nTotalBufs][1],TempoSpec,sizeof(FLOAT)*TempoMaxShift);
	memcpy(&est_spec[nTotalBufs+1][1],TempoSpecP,sizeof(FLOAT)*TempoMaxShift);
/*/	memcpy(&est_spec[nTotalBufs][1],Tempo2WindowFunc,sizeof(float)*TempoMaxShift);
	memcpy(&est_spec[nTotalBufs+1][1],Tempo3WindowFunc,sizeof(float)*TempoMaxShift);
	memcpy(&est_spec[nTotalBufs+2][1],ExpWindowFunc,sizeof(float)*TempoMaxShift);
*/	n_est = nTotalBufs+2;

	// Release buffers
	delete	[] fdata;
	delete	[] DifBuf;
	delete	[] AllBuf;


	// Obtain primary tempo peak
	int	offset  = (int)(TempoMinPeriod/hop_size+0.5);
	int	itempo	= findMax(&TempoSpecP[offset],TempoMaxShift-offset)+offset;

	// Extract relevant correlation spectrums
	memset( TempoSpec, 0, sizeof(FLOAT)*TempoMaxShift );
	for( int x=0; x<nTotalBufs; x++ ){
		int posi = findPeakPosi( TempoCorr[x], TempoMaxShift, itempo );
		register FLOAT f = (FLOAT)((posi-itempo)/(itempo*TempoPeakSharp));
		addCorr( TempoSpec, TempoCorr[x], TempoMaxShift, est_fact2[x]=est_fact[x]*exp(-4*f*f) );
	}

	// Adjust tempo
	itempo	=	adjustTempo(itempo+interPeakPosi(&TempoSpec[itempo]), (int)(TempoMinPeriod/hop_size+0.5),
							TempoMaxShift, TempoSpec, TempoSpecP, Tempo2WindowFunc, Tempo3WindowFunc );
	float	tempo = itempo+interPeakPosi(&TempoSpec[itempo]);
	bool	bAmbi = false;
	if( getMeter(tempo,TempoSpec,TempoMaxShift)==3 ) if(!bAmbi) tempo = -tempo;
	return	(float)hop_size*tempo;
}