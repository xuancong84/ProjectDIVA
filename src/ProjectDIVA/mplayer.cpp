#include "mplayer.h"
#include "VideoEngine.h"
#include "graphEngine.h"

MPlayer mplayer;

MPlayer::MPlayer(){
	m_state=0;
}

void MPlayer::Draw(IDirect3DDevice9 *Device){
	switch(m_state){
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	}
}

HRESULT MPlayer::QueryInterface(const IID &,void **){
	return S_OK;
}
ULONG MPlayer::AddRef(void){
	return 0;
}
ULONG MPlayer::Release(void){
	return 0;
}
HRESULT MPlayer::DragEnter(IDataObject *,DWORD,POINTL,DWORD *){
	return S_OK;
}
HRESULT MPlayer::DragOver(DWORD,POINTL,DWORD *){
	return S_OK;
}
HRESULT MPlayer::DragLeave(void){
	return S_OK;
}
HRESULT MPlayer::Drop(IDataObject *,DWORD,POINTL,DWORD *){
	return S_OK;
}
