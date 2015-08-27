/////////////////////////////////////////////////////////////////////////////
//
// TempoVis.cpp : Implementation of CTempoVis
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TempoVis.h"
#include "CPropertyDialog.h"

#include <string>
using namespace std;


#pragma warning (disable:4996)
#define	_USE_MATH_DEFINES 1

// initial variables
LONG	i_nChannels;
LONG	i_sampleRate;
double	i_curr_media_duration;
string	i_media_name;


#include "TempoVis.hpp"

/////////////////////////////////////////////////////////////////////////////
// CTempoVis::CTempoVis
// Constructor

CTempoVis::CTempoVis() :
m_hwndParent(NULL),
m_clrForeground(0x0000FF),
m_nPreset(0)
{
    lstrcpyn(m_szPluginText, _T("Music Tempo"), sizeof(m_szPluginText) / sizeof(m_szPluginText[0]));
}

/////////////////////////////////////////////////////////////////////////////
// CTempoVis::~CTempoVis
// Destructor

CTempoVis::~CTempoVis()
{
}

/////////////////////////////////////////////////////////////////////////////
// CTempoVis:::FinalConstruct
// Called when an effect is first loaded. Use this function to do one-time
// intializations that could fail (i.e. creating offscreen buffers) instead
// of doing this in the constructor, which cannot return an error.

HRESULT CTempoVis::FinalConstruct()
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CTempoVis:::FinalRelease
// Called when an effect is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CTempoVis::FinalRelease()
{
    ReleaseCore();
}


//////////////////////////////////////////////////////////////////////////////
// CTempoVis::Render
// Called when an effect should render itself to the screen.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::Render(TimedLevel *pLevels, HDC hdc, RECT *prc)
{
	TV_render( m_hwndParent, pLevels );
	return	S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::MediaInfo
// Everytime new media is loaded, this method is called to pass the
// number of channels (mono/stereo), the sample rate of the media, and the
// title of the media
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::MediaInfo(LONG lChannelCount, LONG lSampleRate, BSTR bstrTitle )
{
	if( g_tempoVis ){
		g_tempoVis->pVisual->g_nChannels	= lChannelCount;
		g_tempoVis->pVisual->g_sampleRate	= lSampleRate;
	}else{
		i_nChannels		= lChannelCount;
		i_sampleRate	= lSampleRate;
	}
    return S_OK;
}


//////////////////////////////////////////////////////////////////////////////
// CTempoVis::GetCapabilities
// Returns the capabilities of this effect. Flags that can be returned are:
//	EFFECT_CANGOFULLSCREEN		-- effect supports full-screen rendering
//	EFFECT_HASPROPERTYPAGE		-- effect supports a property page
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::GetCapabilities(DWORD * pdwCapabilities)
{
    if (NULL == pdwCapabilities)
    {
        return E_POINTER;
    }

    *pdwCapabilities = EFFECT_HASPROPERTYPAGE;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::DisplayPropertyPage
// Invoked when a host wants to display the property page for the effect
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::DisplayPropertyPage(HWND hwndOwner)
{
    CPropertyDialog dialog(this);

    dialog.DoModal(hwndOwner);

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::GetTitle
// Invoked when a host wants to obtain the title of the effect
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::GetTitle(BSTR* bstrTitle)
{
    USES_CONVERSION;

    if (NULL == bstrTitle)
    {
        return E_POINTER;
    }

    CComBSTR bstrTemp;
    bstrTemp.LoadString(IDS_EFFECTNAME); 
        
    if ((!bstrTemp) || (0 == bstrTemp.Length()))
    {
        return E_FAIL;
    }

    *bstrTitle = bstrTemp.Detach();

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::GetPresetTitle
// Invoked when a host wants to obtain the title of the given preset
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::GetPresetTitle(LONG nPreset, BSTR *bstrPresetTitle)
{
    USES_CONVERSION;

    if (NULL == bstrPresetTitle)
    {
        return E_POINTER;
    }

    if ((nPreset < 0) || (nPreset >= PRESET_COUNT))
    {
        return E_INVALIDARG;
    }

    CComBSTR bstrTemp;
    
    switch (nPreset)
    {
    case PRESET_NORMAL:
        bstrTemp.LoadString(IDS_MODE00); 
        break;

    case PRESET_NORMAL2:
        bstrTemp.LoadString(IDS_MODE01); 
        break;

    case PRESET_MEASURE:
        bstrTemp.LoadString(IDS_MODE10); 
        break;

    case PRESET_MEASURE2:
        bstrTemp.LoadString(IDS_MODE11); 
        break;
    }
    
    if ((!bstrTemp) || (0 == bstrTemp.Length()))
    {
        return E_FAIL;
    }

    *bstrPresetTitle = bstrTemp.Detach();

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::GetPresetCount
// Invoked when a host wants to obtain the number of supported presets
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::GetPresetCount(LONG *pnPresetCount)
{
    if (NULL == pnPresetCount)
    {
        return E_POINTER;
    }

    *pnPresetCount = PRESET_COUNT;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::SetCurrentPreset
// Invoked when a host wants to change the index of the current preset
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::SetCurrentPreset(LONG nPreset)
{
    if ((nPreset < 0) || (nPreset >= PRESET_COUNT))
    {
        return E_INVALIDARG;
    }

	m_nPreset = nPreset;
    switch (nPreset)
    {
    case PRESET_NORMAL:
        g_mode = 0;
        break;

    case PRESET_NORMAL2:
        g_mode = 1;
        break;

    case PRESET_MEASURE:
        g_mode = 2;
        break;

    case PRESET_MEASURE2:
        g_mode = 3;
        break;
		
	default:
		return	E_FAIL;
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::GetCurrentPreset
// Invoked when a host wants to obtain the index of the current preset
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::GetCurrentPreset(LONG *pnPreset)
{
    if (NULL == pnPreset)
    {
        return E_POINTER;
    }

    *pnPreset = m_nPreset;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::SetCore
// Set WMP core interface
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::SetCore(IWMPCore * pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.

    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::Create
// Invoked when the visualization should be initialized.
//
// If hwndParent != NULL, RenderWindowed() will be called and the visualization
// should draw into the window specified by hwndParent. This will be the
// behavior when the visualization is hosted in a window.
//
// If hwndParent == NULL, Render() will be called and the visualization
// should draw into the DC passed to Render(). This will be the behavior when
// the visualization is hosted windowless (like in a skin for example).
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::Create(HWND hwndParent)
{
    m_hwndParent = hwndParent;
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::Destroy
// Invoked when the visualization should be released.
//
// Any resources allocated for rendering should be released.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::Destroy()
{
    m_hwndParent = NULL;
	TV_release();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::NotifyNewMedia
// Invoked when a new media stream begins playing.
//
// The visualization can inspect this object for properties (like name or artist)
// that might be interesting for visualization.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::NotifyNewMedia(IWMPMedia *pMedia)
{
	if( g_tempoVis ){
		if( g_mode&2 && g_tempoVis->pVisual->preset_tempo==-1 )
			if( g_tempoVis->record_state ){
				g_tempoVis->Record(pause_state, false);
				FILE *fp = fopen( waveform_filename, "rb" );
				if( fp ){
					fseek( fp, 0, SEEK_END );
					long fsize = ftell( fp );
					fclose( fp );
					if( fsize > g_tempoVis->curr_media_duration*g_wavfmt.nAvgBytesPerSec*0.8f 
						&& g_tempoVis->curr_media_duration > MINWAVLENGTH ){
							SetThreadPriority(
								CreateThread( NULL, 0, PostEstThreadFunc, new string(g_tempoVis->curr_media_name), 0, (DWORD*)&fsize ),
								THREAD_PRIORITY_IDLE
							);
					}
				}
		}

		if( !pMedia ) return true;
		pMedia->get_duration( &g_tempoVis->curr_media_duration );

		// retrieve media file name
		BSTR	wstr;
		char	*str;
		int		size;
		float	preset_tempo;

		pMedia->getItemInfo( L"MediaType", &wstr );
		if( wstr == NULL ) goto end;
		if( wcsicmp( wstr, L"audio" ) ){
			if( g_tempoVis->record_state ) g_tempoVis->stop_rec();
			g_tempoVis->pVisual->preset_tempo = 0;
			goto end;
		}// not an audio file

		if( pMedia->get_sourceURL( &wstr ) ) goto end;
		size = WideCharToMultiByte( CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL );
		str = new char [size];
		WideCharToMultiByte( CP_ACP, 0, wstr, -1, str, size, NULL, NULL );
		preset_tempo = g_tempoVis->GetPresetTempo( extr_fn(str) );

		if( preset_tempo<0 && g_mode&2 && g_tempoVis->curr_media_duration<MAXWAVLENGTH ){
			SetThreadPriority(
				CreateThread( NULL, 0, PreEstThreadFunc, new string(str), 0, (DWORD*)&size ),
				THREAD_PRIORITY_IDLE
			);
		}else if( preset_tempo>=0 )	g_status = all_status[0];

		n_est = 0;
		g_tempoVis->pVisual->reset( NULL );
		delete str;
	}else if(pMedia){
		pMedia->get_duration( &i_curr_media_duration );

		// retrieve media file name
		BSTR	wstr;
		char	*str;
		int		size;

		pMedia->getItemInfo( L"MediaType", &wstr );
		if( wstr == NULL ) goto end;
		if( wcsicmp( wstr, L"audio" ) ) goto end;	// not an audio file

		if( pMedia->get_sourceURL( &wstr ) ) goto end;
		size = WideCharToMultiByte( CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL );
		str = new char [size];
		WideCharToMultiByte( CP_ACP, 0, wstr, -1, str, size, NULL, NULL );
		i_media_name = string(str);

		delete str;
	}
end:
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::OnWindowMessage
// Window messages sent to the parent window.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::OnWindowMessage(UINT msg, WPARAM WParam, LPARAM LParam, LRESULT *plResultParam )
{
    // return S_OK only if the plugin has handled the window message
    // return S_FALSE to let the defWindowProc handle the message
    return S_FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::RenderWindowed
// Called when an effect should render itself to the screen.
//
// The fRequiredRender flag specifies if an update is required, otherwise the
// update is optional. This allows visualizations that are fairly static (for example,
// album art visualizations) to only render when the parent window requires it,
// instead of n times a second for dynamic visualizations.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::RenderWindowed(TimedLevel *pLevels, BOOL fRequiredRender )
{
    // NULL parent window should not happen 
    TV_render( m_hwndParent, pLevels );
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CTempoVis::ReleaseCore
// Release WMP core interfaces
//////////////////////////////////////////////////////////////////////////////
void CTempoVis::ReleaseCore()
{
    if (m_spCore)
    {
        m_spCore = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
// CTempoVis::get_foregroundColor
// Property get to retrieve the foregroundColor prop via the public interface.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::get_foregroundColor(BSTR *pVal)
{
	return ColorToWz( pVal, m_clrForeground);
}


//////////////////////////////////////////////////////////////////////////////
// CTempoVis::put_foregroundColor
// Property put to set the foregroundColor prop via the public interface.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CTempoVis::put_foregroundColor(BSTR newVal)
{
	return WzToColor(newVal, &m_clrForeground);
}


//////////////////////////////////////////////////////////////////////////////
// CTempoVis::WzToColor
// Helper function used to convert a string into a COLORREF.
//////////////////////////////////////////////////////////////////////////////
HRESULT CTempoVis::WzToColor(const WCHAR *pwszColor, COLORREF *pcrColor)
{ 
    if (NULL == pwszColor)
    {
        //NULL color string passed in
        return E_POINTER;
    }

    if (0 == lstrlenW(pwszColor))
    {
        //Empty color string passed in
        return E_INVALIDARG;
    }

    if (NULL == pcrColor)
    {
        //NULL output color DWORD passed in
        return E_POINTER;
    }
    
    if (lstrlenW(pwszColor) != 7)
    {
        //hex color string is not of the correct length
        return E_INVALIDARG;
    }

    DWORD dwRet = 0;
    for (int i = 1; i < 7; i++)
    {
        // shift dwRet by 4
        dwRet <<= 4;
        // and add in the value of this string

        if ((pwszColor[i] >= L'0') && (pwszColor[i] <= L'9'))
        {
            dwRet += pwszColor[i] - '0';
        }
        else if ((pwszColor[i] >= L'A') && (pwszColor[i] <= L'F'))
        {
            dwRet += 10 + (pwszColor[i] - L'A');
        }
        else if ((pwszColor[i] >= L'a') && (pwszColor[i] <= L'f'))
        {
            dwRet += 10 + (pwszColor[i] - L'a');
        }
        else
        {
           //Invalid hex digit in color string
            return E_INVALIDARG;
        }
    }

    *pcrColor = SwapBytes(dwRet);

    return S_OK;
} 


//////////////////////////////////////////////////////////////////////////////
// CTempoVis::ColorToWz
// Helper function used to convert a COLORREF to a BSTR.
//////////////////////////////////////////////////////////////////////////////
HRESULT CTempoVis::ColorToWz( BSTR* pbstrColor, COLORREF crColor)
{
    _ASSERT( NULL != pbstrColor );
    _ASSERT( (crColor & 0x00FFFFFF) == crColor );

    *pbstrColor = NULL;

    WCHAR wsz[8];
    HRESULT hr  = S_OK;

    wsprintfW( wsz, L"#%06X", SwapBytes(crColor) );
    
    *pbstrColor = ::SysAllocString( wsz );

    if (!pbstrColor)
    {
        hr = E_FAIL;
    }

    return hr;
}


//////////////////////////////////////////////////////////////////////////////
// CTempoVis::SwapBytes
// Used to convert between a DWORD and COLORREF.  Simply swaps the lowest 
// and 3rd order bytes.
//////////////////////////////////////////////////////////////////////////////
inline DWORD CTempoVis::SwapBytes(DWORD dwRet)
{
    return ((dwRet & 0x0000FF00) | ((dwRet & 0x00FF0000) >> 16) | ((dwRet & 0x000000FF) << 16));
}

