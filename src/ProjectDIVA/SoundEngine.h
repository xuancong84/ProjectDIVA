#pragma once

#include "defines.h"

#pragma comment( lib, "fmod_vc.lib" )
#include "fmod.hpp"
#include "fmod_dsp.h"
#include "fmod_errors.h"

#include "bass.h"
#include "bass_fx.h"
#pragma comment( lib, "bass.lib" )
#pragma comment( lib, "bass_fx.lib" )

typedef int CHANNEL;

#define defaultVolume		0.25f
#define defaultSampleRate	44100

class SoundEngine{
protected:
	double	intended_pitch;

public:
	const static float soundSpd;
	const static int maxChannel = 1024;

	HCHANNEL channel[maxChannel];
	map <int,int> playOffset;
	map <int,DWORD> playMusic;

	virtual void Init(){}
	virtual void Free(){}
	virtual void Clear(){}
	virtual void ERRCHECK(DWORD result, bool bQuit=false){}
	virtual void FreeChannel(int chan, bool bImmediate=false){}
	virtual void Update(float dwTimeMilliSecond){}
	virtual bool Check(CHANNEL Ichannel){return FALSE;}
	
	virtual CHANNEL GetNewChannel(){return 0;}
	virtual void PlaySound(string filename, float volume = 1.000){}
	virtual void PlayHit(float){}
	virtual DWORD ReadMusic(string filename){return 0;}
	virtual DWORD ReadSound(string filename){return 0;}
	virtual DWORD PlayMusic(string filename, CHANNEL Ichannel = -1, bool loop = false, float volume = 1.000){return FALSE;}
	virtual DWORD PlayMusic(int sound_id, CHANNEL Ichannel = -1, bool loop = false, float volume = 1.000){return FALSE;}
	
	virtual void StopMusic(CHANNEL Ichannel, bool alphaout = false){}
	virtual void StopAllMusic(bool alphaout = false){}
	virtual void PauseMusic(CHANNEL Ichannel, bool Pause){}
	virtual void PauseAllMusic(bool Pause){}
	virtual BOOL SetChannelVolume(double factor, int channel_id=0, bool bAlpha=false){return FALSE;}
	virtual BOOL SetSampleRate(double factor, int channel_id=0, bool immediate=false){return FALSE;}
	virtual BOOL SetPitch(double factor, int channel_id=0, bool immediate=false){return FALSE;}
	virtual BOOL SetTempo(double factor, int channel_id=0, bool immediate=false){return FALSE;}
	virtual BOOL SetPosition(int channel, int sound_id, double timeInSeconds){return FALSE;}
	double GetPitch(){return intended_pitch;}
};

class BassSoundEngine:public SoundEngine
{
	int nowChannel;
	float volumeDes[maxChannel];
	map<string,HSAMPLE> sample_pool;
	map<string,HSTREAM> stream_pool;
	HSAMPLE hitWav;
	HSTREAM hTempo;
	double	music_sampleRate;
	double	sampleRate_factor, pitch_factor, tempo_factor;

public:
	BassSoundEngine(){}
	~BassSoundEngine(){Free();}

	void Init();
	void Free();
	void ERRCHECK(DWORD result, bool bQuit=false);
	void Clear();
	void FreeChannel(int chan, bool bImmediate=false);
	void Update(float dwTimeMilliSecond);
	void PlaySound(string filename, float volume = 1.000);
	void PlayHit(float);

	bool Check(CHANNEL Ichannel) {return (Ichannel>=0&&Ichannel<=nowChannel);}
	CHANNEL GetNewChannel();

	HSAMPLE ReadMusic(string filename);
	HSAMPLE ReadSound(string filename);
	HCHANNEL PlayMusic(string filename, CHANNEL Ichannel = -1, bool loop = false, float volume = 1.000);
	HCHANNEL PlayMusic(int sound_id, CHANNEL Ichannel = -1, bool loop = false, float volume = 1.000);

	//unsigned int  getTime() {if(channel[0]==NULL) return 0; unsigned int tmp; channel[0]->getPosition(&tmp,FMOD_TIMEUNIT_MS); return tmp;}

	//details control
	void StopMusic(CHANNEL Ichannel, bool alphaout = false);
	void StopAllMusic(bool alphaout = false);
	void PauseMusic(CHANNEL Ichannel, bool Pause);
	void PauseAllMusic(bool Pause);
	
	BOOL SetChannelVolume(double factor, int channel_id=0, bool bAlpha=false);
	BOOL SetSampleRate(double factor, int channel_id=0, bool immediate=false);
	BOOL SetPitch(double factor, int channel_id=0, bool immediate=false);
	BOOL SetTempo(double factor, int channel_id=0, bool immediate=false);
	BOOL SetPosition(int channel, int sound_id, double timeInSeconds);
};

class FmodSoundEngine:public SoundEngine
{
	int nowChannel;
	float volumeDes[maxChannel];
	FMOD::System    *system;
	FMOD::DSP		*pitchDSP;
	FMOD_RESULT      result;
	map<string,FMOD::Sound*> pool;
	FMOD::Sound *hitWav;
	double	music_sampleRate;
	double	sampleRate_factor, pitch_factor, tempo_factor;

public:
	FmodSoundEngine(){}
	~FmodSoundEngine(){Free();}

	void Init();
	void Free();
	void Clear();
	void ERRCHECK(FMOD_RESULT result, bool bQuit=false);
	void Update(float dwTimeMilliSecond);
	void FreeChannel(int chan, bool bImmediate=false);
	void PlayHit(float);
	void PlaySound(string filename, float volume = 1.000);

	bool Check(CHANNEL Ichannel) {return (Ichannel>=0&&Ichannel<=nowChannel);}
	CHANNEL GetNewChannel();

	DWORD ReadMusic(string filename);
	DWORD PlayMusic(string filename, CHANNEL Ichannel = -1, bool loop = false, float volume = 1.000);
	DWORD PlayMusic(int sound_id, CHANNEL Ichannel = -1, bool loop = false, float volume = 1.000);

	//details control
	void StopMusic(CHANNEL Ichannel, bool alphaout = false);
	void StopAllMusic(bool alphaout = false);
	void PauseMusic(CHANNEL Ichannel, bool Pause);
	void PauseAllMusic(bool Pause);

	BOOL SetChannelVolume(double factor, int channel_id=0, bool bAlpha=false);
	BOOL SetSampleRate(double factor, int channel_id=0, bool immediate=false);
	BOOL SetPitch(double factor, int channel_id=0, bool immediate=false);
	BOOL SetAbsolutePitch(double factor, int channel_id=0);
	BOOL SetTempo(double factor, int channel_id=0, bool immediate=false);
	BOOL SetPosition(int channel, int sound_id, double timeInSeconds);
};

// Frequently accessed class use global definition
extern SoundEngine *soundEngine;
