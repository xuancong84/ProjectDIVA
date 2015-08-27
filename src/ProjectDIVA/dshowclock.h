//-----------------------------------------------------------------------------
// File: DShowTextures.h
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef DSHOWCLOCK_H
#define DSHOWCLOCK_H

//-----------------------------------------------------------------------------
// File: DShowTextures.h
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include <map>
#include "..\DShowClass\streams.h"

using namespace std;

#define MAXEVENTS	16
//-----------------------------------------------------------------------------
// CTextureRenderer Class Declarations
//-----------------------------------------------------------------------------
//class DShowClock : public CBaseReferenceClock 
class DShowClock : public IReferenceClock 
{
	struct Event{
		bool		inUse;
		LONGLONG	time;
		HANDLE		hEvent;
	};
	struct Semaphore{
		bool		inUse;
		LONGLONG	time, interval;
		HANDLE		hSemaphore;
		int			n_triggered;
	};
	int		n_event, n_sema;
	Event	events[MAXEVENTS];
	Semaphore semaphores[MAXEVENTS];
	HANDLE events_mutex, semaphores_mutex;
	int	ref_count;

public:
	double	mul_factor;
	LONGLONG *nowTime;
	LONGLONG tm_offset;
	DShowClock(HRESULT *hr);
	void TriggerThread();
	void SetTimeOffset();
	~DShowClock();

	/**/
	HRESULT WINAPI GetTime(REFERENCE_TIME *pTime);
	HRESULT WINAPI AdvisePeriodic(
		REFERENCE_TIME rtStartTime,
		REFERENCE_TIME rtPeriodTime,
		HSEMAPHORE hSemaphore,
		DWORD_PTR *pdwAdviseCookie
	);
	HRESULT WINAPI AdviseTime(
		REFERENCE_TIME rtBaseTime,
		REFERENCE_TIME rtStreamTime,
		HEVENT hEvent,
		DWORD_PTR *pdwAdviseCookie
	);
	HRESULT WINAPI Unadvise(DWORD_PTR dwAdviseCookie);

	HRESULT WINAPI QueryInterface( REFIID riid, void **ppvObject);
	ULONG WINAPI AddRef( void);
	ULONG WINAPI Release( void);
	/*
	bool bInit;
	REFERENCE_TIME startTime;
	REFERENCE_TIME GetPrivateTime();
	*/
};

#endif