/////////////////////////////////////////////////////////////////////////////
//
// TempoVis.h : Declaration of the CTempoVis
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TEMPOVIS_H_
#define __TEMPOVIS_H_

#include "resource.h"
#include "effects.h"
#include "iTempoVis.h"

// preset values
enum {
    PRESET_NORMAL = 0,
    PRESET_NORMAL2,
	PRESET_MEASURE,
	PRESET_MEASURE2,
    PRESET_COUNT
};

/////////////////////////////////////////////////////////////////////////////
// CTempoVis
class ATL_NO_VTABLE CTempoVis : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTempoVis, &CLSID_TempoVis>,
    public IDispatchImpl<ITempoVis, &IID_ITempoVis, &LIBID_TEMPOVISLib>,
    public IWMPEffects2
{
private:
    COLORREF    m_clrForeground;    // foreground color
    LONG        m_nPreset;          // current preset

    HRESULT WzToColor(const WCHAR *pwszColor, COLORREF *pcrColor);
    HRESULT ColorToWz( BSTR* pbstrColor, COLORREF crColor);
    DWORD SwapBytes(DWORD dwRet);

public:
    CTempoVis();
    ~CTempoVis();

DECLARE_REGISTRY_RESOURCEID(IDR_TEMPOVIS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTempoVis)
    COM_INTERFACE_ENTRY(ITempoVis)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IWMPEffects)
    COM_INTERFACE_ENTRY(IWMPEffects2)
END_COM_MAP()

public:

    // CComCoClass Overrides
    HRESULT FinalConstruct();
    void FinalRelease();

    // ITempoVis
    STDMETHOD(get_foregroundColor)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_foregroundColor)(/*[in]*/ BSTR newVal);

    // IWMPEffects
    STDMETHOD(Render)(TimedLevel *pLevels, HDC hdc, RECT *rc);
    STDMETHOD(MediaInfo)(LONG lChannelCount, LONG lSampleRate, BSTR bstrTitle);
    STDMETHOD(GetCapabilities)(DWORD * pdwCapabilities);
    STDMETHOD(GoFullscreen)(BOOL fFullScreen) { return E_NOTIMPL; };
    STDMETHOD(RenderFullScreen)(TimedLevel *pLevels) { return E_NOTIMPL; };
    STDMETHOD(DisplayPropertyPage)(HWND hwndOwner);
    STDMETHOD(GetTitle)(BSTR *bstrTitle);
    STDMETHOD(GetPresetTitle)(LONG nPreset, BSTR *bstrPresetTitle);
    STDMETHOD(GetPresetCount)(LONG *pnPresetCount);
    STDMETHOD(SetCurrentPreset)(LONG nPreset);
    STDMETHOD(GetCurrentPreset)(LONG *pnPreset);

    // IWMPEffects2
    STDMETHOD(SetCore)(IWMPCore * pCore);
    STDMETHOD(Create)(HWND hwndParent);
    STDMETHOD(Destroy)();
    STDMETHOD(NotifyNewMedia)(IWMPMedia *pMedia);
    STDMETHOD(OnWindowMessage)(UINT msg, WPARAM WParam, LPARAM LParam, LRESULT *plResultParam );
    STDMETHOD(RenderWindowed)(TimedLevel *pLevels, BOOL fRequiredRender );

    TCHAR        m_szPluginText[MAX_PATH];

private:
    void         ReleaseCore();

    HWND                        m_hwndParent;
    CComPtr<IWMPCore>           m_spCore;
};

#endif //__TEMPOVIS_H_
