#include "base.h"

class MPlayer : public IDropTarget{
public:
	MPlayer();
	int		m_state;	// 0:ready, 1:audio, 2:video
	void	Draw(IDirect3DDevice9 *Device);

	// implement IDropTarget
	HRESULT CALLBACK QueryInterface(const IID &,void **);
	ULONG CALLBACK AddRef(void);
	ULONG CALLBACK Release(void);
	HRESULT CALLBACK DragEnter(IDataObject *,DWORD,POINTL,DWORD *);
	HRESULT CALLBACK DragOver(DWORD,POINTL,DWORD *);
	HRESULT CALLBACK DragLeave(void);
	HRESULT CALLBACK Drop(IDataObject *,DWORD,POINTL,DWORD *);
};

extern MPlayer mplayer;
