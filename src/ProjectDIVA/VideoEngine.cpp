#include "VideoEngine.h"
#include "..\DshowClass\refclock.h"
#include "dshowtextures.h"
#include "dshowclock.h"
#include "graphEngine.h"
#include "notemap.h"
#include "GameMana.h"
#include <string>

using namespace std;

extern	int bSynchronize;
VideoEngine				videoEngine;
LPDIRECT3DTEXTURE9      g_pTexture; // Our texture
D3DFORMAT               m_TextureFormat;
BYTE	pBuffer[10000000];
bool	bBufferRefresh;

LPDIRECT3DVERTEXBUFFER9 m_pVB;
CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
CComPtr<IMediaControl>  m_pMC;          // Media Control
CComPtr<IMediaPosition> m_pMP;          // Media Position
CComPtr<IMediaSeeking>	m_pMS;          // Media Seeking
CComPtr<IMediaEvent>    m_pME;          // Media Event
CComPtr<IMediaFilter>	m_pMF;          // Media Filter
CComPtr<IBaseFilter>    m_pRenderer;    // our custom renderer
CComPtr<IBaseFilter>    m_pFSrc;        // Source Filter
CComPtr<IBasicAudio>	m_pAudio;		// audio
CComPtr<IPin>           m_pFSrcPinOut;  // Source Filter Output Pin
CTextureRenderer        *pCTR=NULL;
DShowClock				*pNewClock=NULL;

HRESULT VideoEngine::UpgradeGeometry(LONG lActualW, LONG lTextureW,LONG lActualH, LONG lTextureH )
{
	HRESULT hr = S_OK;
	if( 0 == lTextureW || 0 == lTextureH )
	{
		return E_INVALIDARG;
	}
	FLOAT tuW = (FLOAT)lActualW / (FLOAT)lTextureW;
	FLOAT tvH = (FLOAT)lActualH / (FLOAT)lTextureH;

	// Fill the vertex buffer. We are setting the tu and tv texture
	// coordinates, which range from 0.0 to 1.0
	CUSTOMVERTEX* pVertices;
	if ( FAILED( hr = m_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
	{
		//Msg(TEXT("Could not lock the vertex buffer!  hr=0x%x"), hr);
		return E_FAIL;
	}

	int i=0;

	//注意v轴的单位,选负数!
	//左下角
	pVertices[i].tu = 0.0f;
	pVertices[i].tv = 1.0f*tvH;
	i++;

	//左上角
	pVertices[i].tu = 0.0f;
	pVertices[i].tv = 0.0f;
	i++;

	//右下角
	pVertices[i].tu = 1.0f*tuW;
	pVertices[i].tv = 1.0f*tvH;
	i++;

	//右上角
	pVertices[i].tu = 1.0f*tuW;
	pVertices[i].tv = 0.0f;

	m_pVB->Unlock();
	return S_OK;
}

const char *convert_options[]={
	"",
	"-s 1024x576 -q 4",
	"-s 1024x576 -q 6",
	"-s 1024x576 -q 8",
	"-s 640x360 -q 4",
	"-s 640x360 -q 6",
	"-s 640x360 -q 8",
	"-s 480x270 -q 4",
	"-s 480x270 -q 6",
	"-s 480x270 -q 8",
};

string convertVideo(string video_filename)
{// convert .mp4 to .avi
	int posi = video_filename.find_last_of('.');
	ostringstream oss;
	oss << video_filename.substr(0,posi) << '_' << systemIni.lowVideo << ".avi";
	string out_video_filename = oss.str();
	ifstream fp(out_video_filename.c_str());
	if(!fp)	// if .avi does not exist
	{
		string cmd = "ffmpeg -i \""+video_filename+"\" "+convert_options[systemIni.lowVideo]+" \""+out_video_filename+"\"";
		system(cmd.c_str());
	}
	return out_video_filename;
}

//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT VideoEngine::InitDShowTextureRenderer(string filename)
{
	HRESULT hr = S_OK;
	//CTextureRenderer        *pCTR=0;        // DirectShow Texture renderer

	// Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_pGB);
	if (FAILED(hr))
	{
		Msg(TEXT("Could not create DirectShow!  hr=0x%x"), hr);
		return E_FAIL;
	}

	// Create the Texture Renderer object
	pCTR = new CTextureRenderer( NULL, &hr );

	// Comment out this to disable synchronous clock
	pNewClock = new DShowClock( &hr );

	if (FAILED(hr) || !pCTR)
	{
		delete pCTR;
		Msg(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
		return E_FAIL;
	}

	// Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
	m_pRenderer = pCTR;
	if (FAILED(hr = m_pGB->AddFilter(m_pRenderer, L"TEXTURERENDERER")))
	{
		Msg(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
		return hr;
	}

	// Get the graph's media control, event & position interfaces
	m_pGB->QueryInterface(&m_pMC);
	m_pGB->QueryInterface(&m_pMP);
	m_pGB->QueryInterface(&m_pME);
	m_pGB->QueryInterface(&m_pMS);
	m_pGB->QueryInterface(&m_pMF);
	m_pGB->QueryInterface(&m_pAudio);

	// Render the media file
retry:
	wstring _filename = Ansi2WChar(filename.data());

	if (FAILED(hr = m_pGB->RenderFile(_filename.data(),NULL)))
	{
		hr = MessageBoxW(NULL, Ansi2WChar(StringTable(131)).c_str(), NULL, MB_YESNOCANCEL|MB_ICONEXCLAMATION);
		switch(hr){
		case IDYES:
			PROCESS_INFORMATION p_info;
			STARTUPINFO s_info;
			ZeroMemory(&p_info, sizeof(p_info));
			ZeroMemory(&s_info, sizeof(s_info));
			if(CreateProcess("K-Lite_Codec_Pack_1110_Mega.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &s_info, &p_info)){
				WaitForSingleObject( p_info.hProcess, INFINITE );
				CloseHandle( p_info.hProcess );
				CloseHandle( p_info.hThread );
			}
			char fn[1024];
			GetModuleFileName( NULL, fn, 1024);
			WinExec(fn,SW_SHOWNORMAL);
			ExitProcess(0);
		case IDNO:
			// convert MP4 into AVI
			filename = convertVideo(filename);
			goto retry;
		}
		return hr;
	}

	// Set reference clock
	if(pNewClock)
		hr = m_pMF->SetSyncSource(pNewClock);

	// Start the graph running;
	hr = m_pMC->Run();
	m_pMC->Pause();

	// Obtain video duration
	if(m_pMP->get_Duration(&m_totalTime) != S_OK)
		m_totalTime = 0;

	srcRect = MakeRect(0,0,pCTR->m_lVidWidth,pCTR->m_lVidHeight);
/*
	// If the song has associated audio file, then mute audio channel in video
	if(bHasAudio && m_pAudio)
		m_pAudio->put_Volume(-10000);
*/		
	return S_OK;
}

HRESULT VideoEngine::Init(void)
{
	HRESULT hr=S_OK;
	hr = CoInitialize(NULL);
	return hr;
}
HRESULT VideoEngine::Read(string filename)
{
	HRESULT hr = S_OK;
	if(read)
		Stop();
	read = true;
	current_filename = filename;
	hr |= InitDShowTextureRenderer(filename);  //初始化DirectShow，它会间接建立纹理
	
	// Seek to starting video position
	if(m_pMP)
		hr |= m_pMP->put_CurrentPosition(m_pTime);

	return hr;
}
HRESULT VideoEngine::Play(string filename, int setTime)
{
	HRESULT hr = S_OK;
	if(play){
		if(current_filename==filename)
			return SetPosition(0);
		else
			Read(filename);
	}
	if(!read)
		Read(filename);
	play = true;
	bBufferRefresh = false;
	drawRect = MakefRect(0, 0, WIDTH, HEIGHT);
	if(base::GetAspectRatio(srcRect.right-srcRect.left,srcRect.bottom-srcRect.top)>0){
		drawRect = base::FitWindowRect(MakefRect(srcRect),MakefRect(0, 0, WIDTH, HEIGHT),true);
	}
	if(setTime) if(pNewClock)
		pNewClock->SetTimeOffset();
	hr = m_pMC->Run();
	return hr;
}
HRESULT VideoEngine::SetPosition(double timeInSeconds)
{
	if(m_pMP)
		return m_pMP->put_CurrentPosition(m_pTime+timeInSeconds);
	return S_OK;
}
void VideoEngine::Update()
{
	if(pNewClock) pNewClock->TriggerThread();

	if(play)
	{
		if(!IsPlaying())
			Stop();
		else
		{
			if(pCTR)
				pCTR->LoadTexture();
		}
	}
}
void VideoEngine::Draw()
{
	if(play&&g_pTexture)
		graphEngine.Draw(g_pTexture,srcRect,drawRect,(alpha_adjust<<24)|0x00ffffff );
}
void VideoEngine::Paused(bool Paused)
{
	if(play)
	{
		if(Paused)
			m_pMC->Pause();
		else
			m_pMC->Run();
	}
}
void VideoEngine::Stop()
{
	if(play)
	{
		play = false;
		read = false;
		Clear();
	}
}
bool VideoEngine::IsPlaying()
{
	LONG   evCode,   evParam1,   evParam2;
	if (m_pME)
	{
		if (SUCCEEDED(m_pME->GetEvent(&evCode,(LONG_PTR*)&evParam1,(LONG_PTR*)&evParam2,0)))
		{
			return	!(evCode == EC_COMPLETE);
		}
		return true;
	}
	return false; 
}

void VideoEngine::Clear()
{
	bBufferRefresh = false;
	//m_pMF->SetSyncSource(NULL);
	if(g_pTexture)
	{
		g_pTexture->Release();
		g_pTexture = NULL;
	}
	if (pCTR)	{pCTR->Stop(); pCTR = NULL;}
	if (m_pMC)	{m_pMC->Stop(); m_pMC.Release(); }
	if (m_pME)	m_pME.Release();
	if (m_pMS)	m_pMS.Release();
	if (m_pMF)	m_pMF.Release();
	if (m_pMP)	m_pMP.Release();
	if (m_pGB)	m_pGB.Release();
	if (m_pAudio)		m_pAudio.Release();
	if (m_pFSrc)        m_pFSrc.Release();        // Source Filter
	if (m_pFSrcPinOut)  m_pFSrcPinOut.Release();  // Source Filter Output Pin
	if (m_pRenderer)	m_pRenderer.Release();
	if (pNewClock)	{delete pNewClock; pNewClock = NULL;}
}
