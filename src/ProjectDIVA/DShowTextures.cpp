//-----------------------------------------------------------------------------
// File: DShowTextures.cpp
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 9.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include <mfapi.h>
#include "DShowTextures.h"
#include "VideoEngine.h"
#include "GameMana.h"
#include "dshowclock.h"


extern DShowClock *pNewClock;
extern bool	bAlarm;
D3DFORMAT	g_TextureFormat;		// Texture format
GUID		g_MediaFormat;			// GUID of media format
bool		g_bFlip;
float		video_auto_dim;
void YUV2_RGB32_float(BYTE*,BYTE*,int,int,int,int);

//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
: CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
					 NAME("Texture Renderer"), pUnk, phr),
					 m_bUseDynamicTextures(FALSE)
{
	// Store and AddRef the texture for our use.
	ASSERT(phr);
	if (phr) *phr = S_OK;
	video_auto_dim = *((FloatResource*)g_res.getResource("video_auto_dim"))->value;
}

CTextureRenderer::~CTextureRenderer()
{
	// Do nothing
}

#include <InitGuid.h>

DEFINE_GUID( MFVideoFormat_RGB32,0x00000022,0x0000,0x0010,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71 );
DEFINE_GUID( MFVideoFormat_RGB24,0x00000014,0x0000,0x0010,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71 );
DEFINE_GUID( MFVideoFormat_YUY2, 0x32595559,0x0000,0x0010,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71 );

HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
	VIDEOINFO *pvi=0;
	CheckPointer(pmt,E_POINTER);
/*
	// Reject the connection if this is not a video type
	if( *pmt->FormatType() != FORMAT_VideoInfo ) {
		return E_INVALIDARG;
	}
*/
	HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;

	// Obtain video format
	pvi = (VIDEOINFO *)pmt->Format();

	// Support 32-bit video format in preference to 24-bit as 32-bit is faster
	if(			IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB32 )
			||	IsEqualGUID( *pmt->Subtype(), MFVideoFormat_RGB32 )){
		g_MediaFormat = MEDIASUBTYPE_RGB32;
		hr = S_OK;
	}/*else if(	IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YUY2 )
			||	IsEqualGUID( *pmt->Subtype(), MFVideoFormat_YUY2 )){
		g_MediaFormat = MEDIASUBTYPE_YUY2;
		hr = S_OK;
	}*/
	else if(	IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24 )
			||	IsEqualGUID( *pmt->Subtype(), MFVideoFormat_RGB24 )){
		g_MediaFormat = MEDIASUBTYPE_RGB24;
		hr = S_OK;
	}

	return hr;
}

typedef struct tagVIDEOINFOHEADER2 {
	RECT                rcSource;
	RECT                rcTarget;
	DWORD               dwBitRate;
	DWORD               dwBitErrorRate;
	REFERENCE_TIME      AvgTimePerFrame;
	DWORD               dwInterlaceFlags;   // use AMINTERLACE_* defines. Reject connection if undefined bits are not 0
	DWORD               dwCopyProtectFlags; // use AMCOPYPROTECT_* defines. Reject connection if undefined bits are not 0
	DWORD               dwPictAspectRatioX; // X dimension of picture aspect ratio, e.g. 16 for 16x9 display
	DWORD               dwPictAspectRatioY; // Y dimension of picture aspect ratio, e.g.  9 for 16x9 display
	union {
		DWORD dwControlFlags;               // use AMCONTROL_* defines, use this from now on
		DWORD dwReserved1;                  // for backward compatiblity (was "must be 0";  connection rejected otherwise)
	};
	DWORD               dwReserved2;        // must be 0; reject connection otherwise
	BITMAPINFOHEADER    bmiHeader;
} VIDEOINFOHEADER2;

HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
	HRESULT hr;

	UINT uintWidth = 2;
	UINT uintHeight = 2;

	// Retrive the size of this media type
	D3DCAPS9 caps;
	BYTE *pviBmp = pmt->Format();

	if(pmt->formattype==FORMAT_VideoInfo2){
		m_lVidWidth  = ((VIDEOINFOHEADER2*)pviBmp)->bmiHeader.biWidth;
		m_lVidHeight = ((VIDEOINFOHEADER2*)pviBmp)->bmiHeader.biHeight;
	}else{
		m_lVidWidth  = ((VIDEOINFOHEADER*)pviBmp)->bmiHeader.biWidth;
		m_lVidHeight = ((VIDEOINFOHEADER*)pviBmp)->bmiHeader.biHeight;
	}

	// Flipped video has negative heights
	g_bFlip = (m_lVidHeight<0);
	m_lVidHeight = abs(m_lVidHeight);

	if(IsEqualGUID(g_MediaFormat,MEDIASUBTYPE_RGB32))
		m_lVidPitch  = m_lVidWidth * 4;
	else if(IsEqualGUID(g_MediaFormat,MEDIASUBTYPE_YUY2))
		m_lVidPitch  = m_lVidWidth * 2;
	else
		m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24

	// here let's check if we can use dynamic textures
	ZeroMemory( &caps, sizeof(D3DCAPS9));
	hr = base::Device->GetDeviceCaps( &caps );
	if( caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES )
	{
		m_bUseDynamicTextures = TRUE;
	}

	// For efficiency, do not use non-power-of-2 texture size
	while( (LONG)uintWidth < m_lVidWidth )
	{
		uintWidth = uintWidth << 1;
	}
	while( (LONG)uintHeight < m_lVidHeight )
	{
		uintHeight = uintHeight << 1;
	}

	// Create the texture that maps to this media type
	hr = E_UNEXPECTED;

	SAFE_RELEASE(g_pTexture);
	hr = base::Device->CreateTexture(uintWidth, uintHeight, 1, 0, 
		D3DFMT_X8R8G8B8,D3DPOOL_MANAGED, &g_pTexture, NULL);

	if( FAILED(hr))
	{
		Msg(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
		return hr;
	}

	// CreateTexture can silently change the parameters on us
	D3DSURFACE_DESC ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));

	if ( FAILED( hr = g_pTexture->GetLevelDesc( 0, &ddsd ) ) ) {
		Msg(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
		return hr;
	}

	CComPtr<IDirect3DSurface9> pSurf; 

	if (SUCCEEDED(hr = g_pTexture->GetSurfaceLevel(0, &pSurf)))
		pSurf->GetDesc(&ddsd);

	// Save format info
	g_TextureFormat = ddsd.Format;

	if (g_TextureFormat != D3DFMT_X8R8G8B8 &&
		g_TextureFormat != D3DFMT_A1R5G5B5) {
			Msg(TEXT("Texture is format we can't handle! Format = 0x%x"), g_TextureFormat);
			return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}

#define SAMPLE_PIXELS	1024
float estimate_brightness_RGB32( BYTE *buf, int max_size ){
	DWORD sum = 0;
	for(int x=0; x<SAMPLE_PIXELS; x++)
	{
		BYTE *addr = (BYTE*)&((DWORD*)buf)[(int)((float)rand()/RAND_MAX*max_size)];
		sum += ((DWORD)addr[0]+addr[1]+addr[2]);
	}
	return	(float)sum/(3*SAMPLE_PIXELS);
}
float estimate_brightness_RGB24( BYTE *buf, int max_size ){
	DWORD sum = 0;
	for(int x=0; x<SAMPLE_PIXELS; x++)
	{
		BYTE *addr = &buf[(int)((float)rand()/RAND_MAX*max_size)*3];
		sum += ((DWORD)addr[0]+addr[1]+addr[2]);
	}
	return	(float)sum/(3*SAMPLE_PIXELS);
}
int adjust_brightness( float mean )
{
	return 255.5f-video_auto_dim*mean*mean;
}

HRESULT CTextureRenderer::LoadTexture()
{
	// If video image is not updated (slow FPS), don't waste time copying the same texture again
	if(bBufferRefresh){
		bBufferRefresh = false;
	}else return S_OK;

	if(g_pTexture==NULL)
		return S_OK;

	BYTE  *pBmpBuffer,*pTxtBuffer;	// Bitmap buffer, texture buffer
	LONG  lTxtPitch;                // Pitch of bitmap, texture

	CheckPointer(g_pTexture,E_UNEXPECTED);
	pBmpBuffer = pBuffer;

	// Lock the Texture
	D3DLOCKED_RECT d3dlr;
	if( m_bUseDynamicTextures )
	{
		if( FAILED(g_pTexture->LockRect(0, &d3dlr, 0, D3DLOCK_DISCARD)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(g_pTexture->LockRect(0, &d3dlr, 0, 0)))
			return E_FAIL;
	}

	// Get the texture buffer & pitch
	pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
	lTxtPitch = d3dlr.Pitch;

	// estimate and adjust video brightness
	if(systemIni.particle>=3 && GAMEDATA.gamemode!=GAME_MODE_PV && video_auto_dim>0){
		float brightness = IsEqualGUID(g_MediaFormat,MEDIASUBTYPE_RGB32)?
					estimate_brightness_RGB32(pBmpBuffer,m_lVidHeight*m_lVidWidth)
					:estimate_brightness_RGB24(pBmpBuffer,m_lVidHeight*m_lVidWidth);
		videoEngine.alpha_adjust = adjust_brightness(brightness);
	}
	else
		videoEngine.alpha_adjust = 0xff;

	// Copy the bits
	if (g_TextureFormat == D3DFMT_X8R8G8B8) 
	{
		if(IsEqualGUID(g_MediaFormat,MEDIASUBTYPE_RGB32))
		{
			// Copy 32-bit video image into 32-bit texture buffer, this is faster
			if(!g_bFlip) pTxtBuffer += lTxtPitch*(UINT)m_lVidHeight;
			for( UINT row=0; row<(UINT)m_lVidHeight; row++ )
			{
				pTxtBuffer  += (!g_bFlip?-lTxtPitch:lTxtPitch);
				memcpy(pTxtBuffer,pBmpBuffer,m_lVidWidth<<2);
				pBmpBuffer  += m_lVidPitch;
			}
		}
		else if(IsEqualGUID(g_MediaFormat,MEDIASUBTYPE_YUY2))
		{
			videoEngine.alpha_adjust = 0xff;
			YUV2_RGB32_float(pBmpBuffer,pTxtBuffer,m_lVidWidth,m_lVidHeight,m_lVidPitch,lTxtPitch);
		}
		else
		{
			// Instead of copying data bytewise, we use DWORD alignment here.
			// We also unroll loop by copying 4 pixels at once.
			//
			// original BYTE array is [b0][g0][r0][b1][g1][r1][b2][g2][r2][b3][g3][r3]
			//
			// aligned DWORD array is     [b1 r0 g0 b0][g2 b2 r1 g1][r3 g3 b3 r2]
			//
			// We want to transform it to [ff r0 g0 b0][ff r1 g1 b1][ff r2 g2 b2][ff r3 b3 g3]
			// below, bitwise operations do exactly this.
			BYTE  * pbS = NULL;
			DWORD * pdwS = NULL;
			DWORD * pdwD = NULL;

			UINT dwordWidth = m_lVidWidth / 4; // aligned width of the row, in DWORDS
			// (pixel by 3 bytes over sizeof(DWORD))

			pTxtBuffer += lTxtPitch*(UINT)m_lVidHeight;

			for( UINT row = 0; row< (UINT)m_lVidHeight; row++)
			{
				pTxtBuffer -= lTxtPitch;

				pdwS = ( DWORD*)pBmpBuffer;
				pdwD = ( DWORD*)pTxtBuffer;

				for( UINT col = 0; col < dwordWidth; col ++ )
				{
					pdwD[0] =  pdwS[0] | 0xFF000000;
					pdwD[1] = ((pdwS[1]<<8)  | 0xFF000000) | (pdwS[0]>>24);
					pdwD[2] = ((pdwS[2]<<16) | 0xFF000000) | (pdwS[1]>>16);
					pdwD[3] = 0xFF000000 | (pdwS[2]>>8);
					pdwD +=4;
					pdwS +=3;
				}

				// we might have remaining (misaligned) bytes here
				pbS = (BYTE*) pdwS;
				for( UINT col = 0; col < (UINT)m_lVidWidth % 4; col++)
				{
					*pdwD = 0xFF000000     |
						(pbS[2] << 16) |
						(pbS[1] <<  8) |
						(pbS[0]);
					pdwD++;
					pbS += 3;           
				}

				pBmpBuffer  += m_lVidPitch;

			}// for rows
		}
	}
	else if (g_TextureFormat == D3DFMT_A1R5G5B5) 
	{
		UINT bmpBufferInc = (IsEqualGUID(g_MediaFormat,MEDIASUBTYPE_RGB32)?4:3);
		for(int y = 0; y < m_lVidHeight; y++ ) 
		{
			BYTE *pBmpBufferOld = pBmpBuffer;
			BYTE *pTxtBufferOld = pTxtBuffer;   

			for (int x = 0; x < m_lVidWidth; x++) 
			{
				*(WORD *)pTxtBuffer = (WORD)
					(0x8000 +
					((pBmpBuffer[2] & 0xF8) << 7) +
					((pBmpBuffer[1] & 0xF8) << 2) +
					(pBmpBuffer[0] >> 3));

				pTxtBuffer += 2;
				pBmpBuffer += bmpBufferInc;
			}

			pBmpBuffer = pBmpBufferOld + m_lVidPitch;
			pTxtBuffer = pTxtBufferOld + lTxtPitch;
		}
	}

	// Unlock the Texture
	if (FAILED(g_pTexture->UnlockRect(0)))
		return E_FAIL;

	return S_OK;
}

int bSynchronize = -1;
LONGLONG tm_diff = 0;
extern int n_alarmed;
extern void printWinTitle( const char *fmt, ... );
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
	BYTE  *pBmpBuffer;

	CheckPointer(pSample,E_POINTER);
	CheckPointer(g_pTexture,E_UNEXPECTED);

	// Get the video bitmap buffer
	pSample->GetPointer( &pBmpBuffer );
	int size = pSample->GetSize();
	memcpy(pBuffer,pBmpBuffer,sizeof(BYTE)*size);

	/*
	// Alarm delay
	LONGLONG t1,t2;
	HRESULT hr = pSample->GetTime(&t1, &t2);
	pNewClock->GetTime(&t2);
	//t2 = *(pNewClock->nowTime);
	printWinTitle("sync=%d alarm=%d %I64u %I64d %I64d",bSynchronize,n_alarmed,t2,t1+tm_diff,t2-(t1+tm_diff));

	if(bSynchronize>0) bSynchronize--;
	else if(!bSynchronize)
	{
		tm_diff = t2-t1;
		bSynchronize = -1;
	}
	else if(t2-(t1+tm_diff)>5000000) bAlarm = true;
	*/
	// Notify the D3D renderer that the video image has changed, so need to do copying
	bBufferRefresh = true;

	/*	Don't copy video image into D3D texture now, the video may have a much higher FPS,
	more than 1 video frame may have arrived for 1 frame of D3D rendering update,
	so do the video image copying only when on demand! */

	return S_OK;
}

//-----------------------------------------------------------------------------
// Msg: Display an error message box if needed
//-----------------------------------------------------------------------------
void Msg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	_vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');

	MessageBoxW(NULL, Ansi2WChar(szBuffer).data(), L"DirectShow Texture3D9 Sample", MB_OK | MB_ICONERROR);
}

// For YUV2 format
#define ALIGNED __declspec(align(16))
ALIGNED float vec0[4]={
	0.0, 0.0, 0.0, 0.0
};
ALIGNED float vec255[4]={
	255.0, 255.0, 255.0, 255.0
};
ALIGNED float vecn128[4]={
	-128.0, -128.0, -128.0, -128.0
};
ALIGNED float vec_yuvconst1[4]={
	1.13983, 1.13983, 1.13983, 1.13983
};
ALIGNED float vec_yuvconst2[4]={
	-0.39466, -0.39466, -0.39466, -0.39466
};
ALIGNED float vec_yuvconst3[4]={
	-0.58060, -0.58060, -0.58060, -0.58060
};
ALIGNED float vec_yuvconst4[4]={
	2.03211, 2.03211, 2.03211, 2.03211
};
ALIGNED float blah[4]={
	0,0,0,0
};
ALIGNED float vecy[4]={
	0, 0, 0, 0
};
ALIGNED float vecu[4]={
	0, 0, 0, 0
};
ALIGNED float vecv[4]={
	0, 0, 0, 0
};

void YUV2_RGB32_float(	unsigned char *yuv_buffer, unsigned char *rgb_buffer,
						int width, int height, int src_inc, int dst_inc	){
	__asm{

		mov esi, yuv_buffer     ; yuv_buffer
		mov edi, rgb_buffer    ; rgb_buffer
		mov eax, [width]    ; width
		mov ecx, [height]    ; height
		mul ecx
		mov ecx, eax         ; ecx = width*height
		mov ebx, ecx

		mov ebp, esi
		add ebp, ebx     ; ebp = esi + (height*width,ebx) = u part of buffer
		shr ebx, 1       ; ebx + ebp = v part of buffer

		movaps xmm4, [vec0]          ; xmm4 = vector(0.0,,,)
		movaps xmm5, [vec255]        ; xmm5 = vector(255.0,,,)

		movaps xmm6, [vec_yuvconst1] ; xmm6 = vector(1.13983,,,)
		movaps xmm7, [vec_yuvconst4] ; xmm7 = vector(2.03211,,,)

		shr ecx, 2       ; ecx = width*height/4

yuv_loop:
		mov eax, [esi]    ; place y0,y1,y2,y3 into a vector
		mov byte ptr [vecy+12], al
		mov byte ptr [vecy+8], ah
		shr eax, 16
		mov byte ptr [vecy+4], al
		mov byte ptr [vecy], ah

		mov ax, [ebp]    ; place u0,u0,u1,u1 into a vector
		mov byte ptr [vecu], al
		mov byte ptr [vecu+4], al
		mov byte ptr [vecu+8], ah
		mov byte ptr [vecu+12], ah

		mov ax, [ebp+ebx] ; place v0,v0,v1,v1 into a vector
		mov byte ptr [vecv], al
		mov byte ptr [vecv+4], al
		mov byte ptr [vecv+8], ah
		mov byte ptr [vecv+12], ah

		cvtdq2ps xmm0, [vecy]    ; xmm0 = (float)vecy
		cvtdq2ps xmm1, [vecu]    ; xmm1 = (float)vecu
		cvtdq2ps xmm2, [vecv]    ; xmm2 = (float)vecv

		addps xmm1, [vecn128]     ; vecu = vecu - vec128
		addps xmm2, [vecn128]     ; vecv = vecv - vec128

		movaps xmm3, xmm2        ; vecr = y + 1.13983*(v-128)
		mulps xmm3, xmm6         ; *1.3983
		addps xmm3, xmm0         ; +y
		maxps xmm3, xmm4         ; max(r,0)
		minps xmm3, xmm5         ; min(r,255)
		cvttps2dq xmm3, xmm3     ; float to int
		pextrw eax, xmm3, 6
		mov byte ptr [edi], al
		pextrw eax, xmm3, 4
		mov byte ptr [edi+4], al
		pextrw eax, xmm3, 2
		mov byte ptr [edi+8], al
		pextrw eax, xmm3, 0
		mov byte ptr [edi+12], al

		movaps xmm3, xmm1        ; vecb = y + 2.03211*(u-128)
		mulps xmm3, xmm7         ; *2.03211
		addps xmm3, xmm0         ; +y
		maxps xmm3, xmm4         ; max(b,0)
		minps xmm3, xmm5         ; min(b,255)
		cvttps2dq xmm3, xmm3     ; float to int
		pextrw eax, xmm3, 6
		mov [edi+2], al
		pextrw eax, xmm3, 4
		mov [edi+6], al
		pextrw eax, xmm3, 2
		mov [edi+10], al
		pextrw eax, xmm3, 0
		mov [edi+14], al

		; vecg = y + -0.39466*(u-128) -0.58060*(v-128)
		mulps xmm1, [vec_yuvconst2] ; -0.39466*(u-128)
		mulps xmm2, [vec_yuvconst3] ; -0.58060*(v-128)
		addps xmm0, xmm1         ; +y
		addps xmm0, xmm2         ; +y
		maxps xmm0, xmm4         ; max(g,0)
		minps xmm0, xmm5         ; min(g,255)
		cvttps2dq xmm3, xmm0     ; float to int
		pextrw eax, xmm3, 6
		mov [edi+1], al
		pextrw eax, xmm3, 4
		mov [edi+5], al
		pextrw eax, xmm3, 2
		mov [edi+9], al
		pextrw eax, xmm3, 0
		mov [edi+13], al

		add edi, 16
		add esi, 4
		add ebp, 2

		dec ecx
		jnz yuv_loop
	}
}
