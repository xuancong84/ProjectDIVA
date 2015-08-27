//-----------------------------------------------------------------------------
// File: DShowClock.cpp
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 9.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "dshowclock.h"
#include "GameMana.h"

extern GameCore core;

DShowClock::DShowClock(HRESULT *hr)
	//:CBaseReferenceClock("DShowClock", NULL, hr)
{
	nowTime = &core._notemana.nowTime;
	mul_factor = 1.0e7/base::SECOND;
	n_event = n_sema = 0;
	events_mutex = CreateMutex(NULL, false, NULL);
	semaphores_mutex = CreateMutex(NULL, false, NULL);
	tm_offset = 0;
	ref_count = 1;
	for(int x=0;x<MAXEVENTS;x++)
	{
		events[x].inUse = false;
		semaphores[x].inUse = false;
	}
	//bInit = false;
}

DShowClock::~DShowClock()
{
	CloseHandle(events_mutex);
	CloseHandle(semaphores_mutex);
}

void DShowClock::TriggerThread()
{
	if(!this) return;

	LONGLONG tm;
	GetTime(&tm);
	/*
	char str[100];
	sprintf(str,"event=%d, semaphore=%d",n_event,n_sema);
	SetWindowTitle(base::hwnd,str);
	*/
	int n_remain = n_event;
	for(int x=0; x<MAXEVENTS && n_remain>0; x++)
	{
		Event &e = events[x];
		if(!e.inUse) continue;
		if(tm>e.time)
		{
			SetEvent(e.hEvent);
			e.inUse = false;
			--n_event;
		}
		--n_remain;
	}

	n_remain = n_sema;
	for(int x=0; x<MAXEVENTS && n_remain>0; x++)
	{
		Semaphore &e = semaphores[x];
		if(!e.inUse) continue;
		if(tm<e.time) continue;
		int n_trigger = (tm-e.time)/e.interval+1;
		if(n_trigger>e.n_triggered)
		{
			ReleaseSemaphore(e.hSemaphore,n_trigger-e.n_triggered,NULL);
			e.n_triggered = n_trigger;
		}
		--n_remain;
	}
}

void DShowClock::SetTimeOffset()
{
	GetTime( &tm_offset );
}

HRESULT WINAPI DShowClock::GetTime(REFERENCE_TIME *pTime)
{
	//*pTime = (LONGLONG)*nowTime*mul_factor-tm_offset;
	*pTime = (LONGLONG)*nowTime*mul_factor;
	return S_OK;
}

HRESULT WINAPI DShowClock::AdvisePeriodic(
	REFERENCE_TIME rtStartTime,
	REFERENCE_TIME rtPeriodTime,
	HSEMAPHORE hSemaphore,
	DWORD_PTR *pdwAdviseCookie)
{
	int id;
	for(id=0; id<MAXEVENTS; id++)
		if(!events[id].inUse) break;
	Semaphore &semaphore = semaphores[id];
	semaphore.inUse = true;
	semaphore.n_triggered = 0;
	semaphore.time = rtStartTime;
	semaphore.interval = rtPeriodTime;
	semaphore.hSemaphore = (HANDLE)hSemaphore;
	*pdwAdviseCookie = MAXEVENTS+id;
	++n_sema;
	return S_OK;
}

HRESULT WINAPI DShowClock::AdviseTime(
	REFERENCE_TIME rtBaseTime,
	REFERENCE_TIME rtStreamTime,
	HEVENT hEvent,
	DWORD_PTR *pdwAdviseCookie)
{
	int id;
	for(id=0; id<MAXEVENTS; id++)
		if(!events[id].inUse) break;
	Event &event = events[id];
	event.hEvent = (HANDLE)hEvent;
	event.time = rtBaseTime+rtStreamTime;
	event.inUse = true;
	*pdwAdviseCookie = id;
	++n_event;
	return S_OK;
}

HRESULT WINAPI DShowClock::Unadvise(DWORD_PTR dwAdviseCookie)
{
	if((DWORD)dwAdviseCookie<MAXEVENTS)
	{
		int id=(DWORD)dwAdviseCookie;
		if(events[id].inUse)
		{
			events[id].inUse = false;
			--n_event;
		}
	}else{
		int id=(DWORD)dwAdviseCookie-MAXEVENTS;
		if(semaphores[id].inUse)
		{
			semaphores[id].inUse = false;
			--n_sema;
		}
	}
	return	S_OK;
}

/*/
REFERENCE_TIME DShowClock::GetPrivateTime()
{
	if(!bInit)
	{
		startTime = CBaseReferenceClock::GetPrivateTime();
		bInit = true;
		return startTime;
	}
	//CBaseReferenceClock::GetPrivateTime();
	return (LONGLONG)*nowTime*mul_factor+startTime;
}
/**/

HRESULT WINAPI DShowClock::QueryInterface( REFIID riid, void **ppvObject ){
	*ppvObject = this;
	return S_OK;
}
ULONG WINAPI DShowClock::AddRef(){
	return ++ref_count;
}
ULONG WINAPI DShowClock::Release(){
	int ret = --ref_count;
	//if(ret<0) delete this;
	return ret;
}

