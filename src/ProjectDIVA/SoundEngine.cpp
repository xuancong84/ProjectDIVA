#include "SoundEngine.h"
#include "UIScreen.h"

const float BassSoundEngine::soundSpd = defaultVolume/2000.0f;	// absolute volume change per millisecond
extern CComPtr<IBasicAudio>	m_pAudio;				// audio

SoundEngine *soundEngine = NULL;

double Lin2dB(double val)
{
	if(val<1e-10)
		return -10000;
	double ret = log10(val*val)*1000.0;
	return ret;
}

// BASS sound system
void BassSoundEngine::ERRCHECK(DWORD result, bool bQuit)
{
	if (result == FALSE)
    {
		char str[256];
        sprintf(str,"BASS error! (code=%d)\n", BASS_ErrorGetCode());
        if(bQuit){
			MessageBoxW(NULL, Ansi2WChar(str).data(), NULL, MB_OK|MB_ICONEXCLAMATION);
			exit(-1);
		}
		ShowMessage(str,2,0,32);
    }
}

void BassSoundEngine::Init()
{
	nowChannel = 0;
	sampleRate_factor = pitch_factor = tempo_factor = 0;
	intended_pitch = 1.0;

	DWORD hr;
	hr = BASS_Init(-1, defaultSampleRate, BASS_DEVICE_LATENCY, base::hwnd, NULL);
	hr = BASS_FX_GetVersion();
	hitWav = BASS_SampleLoad(false, "gamedata\\hit.wav", 0, 0, 16, 0);
#if 1==0
	HSTREAM fs1, fs2;
	fs1 = BASS_StreamCreateFile(false,"D:\\Project DIVA PC\\gamedata\\cg.mp3",0,0,BASS_STREAM_DECODE);
	fs2 = BASS_FX_TempoCreate(fs1, 0);
	hr = BASS_ErrorGetCode();
	hr = BASS_ChannelPlay(fs2, FALSE);
	hr = BASS_ChannelSetAttribute(fs2, BASS_ATTRIB_TEMPO, 50); 
	Sleep(100000);
	hr = BASS_ChannelStop(fs1);
	Sleep(2000);
#endif
	hr = 0;
}

void BassSoundEngine::Free()
{
	Clear();
	BASS_Stop();
	BASS_Free();
}

CHANNEL BassSoundEngine::GetNewChannel()
{
	// Attempt to find an unused channel
	for( int x=1; x<maxChannel; x++)
		if(channel[x]==NULL){
			nowChannel = max(nowChannel, x+1);
			return x;
		}
	// Free all inactive channels
	for( int x=1; x<maxChannel; x++)
		if(BASS_ChannelIsActive(channel[x])!=BASS_ACTIVE_PLAYING){
			channel[x] = NULL;
		}
	// Attempt to find a free channel again
	for( int x=1; x<maxChannel; x++)
		if(channel[x]==NULL){
			nowChannel = max(nowChannel, x+1);
			return x;
		}
	return -1;
}

void BassSoundEngine::PlayHit(float factor)
{
	float vol = systemIni.sndVolume*(0.5f+factor*0.25f);
	/*
	FMOD::Channel *channel;
	system->playSound(FMOD_CHANNEL_FREE, hitWav, false, &channel);
	channel->setVolume(min(1.0f,vol)*defaultVolume);*/
	//channel->setPaused(false);
	//ERRCHECK(result);
	HCHANNEL chan = BASS_SampleGetChannel(hitWav, FALSE);
	BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, min(1.0f,vol)*defaultVolume);
	BASS_ChannelPlay(chan, true);
}

BOOL BassSoundEngine::SetSampleRate(double factor, int channel_id, bool immediate)
{
	if(immediate){
		BOOL hr;
		float current_sampleRate = music_sampleRate * factor;
		hr = BASS_ChannelSetAttribute(channel[channel_id], BASS_ATTRIB_FREQ, current_sampleRate);
		ERRCHECK(hr);
		return hr;
	}else{
		sampleRate_factor = factor;
		return TRUE;
	}
}

BOOL BassSoundEngine::SetPitch(double ratio, int channel_id, bool immediate)
{
	if(immediate){
		intended_pitch = ratio;
		float factor = log(ratio)*(12.0f/log(2.0f));
		BOOL hr = BASS_ChannelSetAttribute(channel[channel_id], BASS_ATTRIB_TEMPO_PITCH, factor);
		ERRCHECK(hr);
		return hr;
	}else{
		pitch_factor = ratio;
		return TRUE;
	}
}

BOOL BassSoundEngine::SetTempo(double factor, int channel_id, bool immediate)
{
	if(immediate){
		BOOL hr = BASS_ChannelSetAttribute(channel[channel_id], BASS_ATTRIB_TEMPO, (factor-1.0)*100.0);
		ERRCHECK(hr);
		return hr;
	}else{
		tempo_factor = factor;
		return TRUE;
	}
}

BOOL BassSoundEngine::SetChannelVolume(double volume, int channel_id, bool bAlpha)
{
	volumeDes[channel_id] = volume*defaultVolume;
	if(!bAlpha)
		return BASS_ChannelSetAttribute(channel[channel_id], BASS_ATTRIB_VOL, volume*defaultVolume);
/*
		float now_volume;
		BASS_ChannelGetAttribute(channel[channel_id], BASS_ATTRIB_VOL, &now_volume);
		float delta_volume = abs(volume*defaultVolume-now_volume);
		return BASS_ChannelSlideAttribute(channel[channel_id], BASS_ATTRIB_VOL, volume*defaultVolume, delta_volume/soundSpd);
*/
}

void BassSoundEngine::FreeChannel(int chan_id, bool bImmediate)
{
	if(channel[chan_id] == NULL)
		return;
	if(BASS_ChannelIsActive(channel[chan_id])==BASS_ACTIVE_PLAYING)
	{
		if(bImmediate){
			BASS_ChannelStop(channel[chan_id]);
		}else{
			int new_id = GetNewChannel();
			channel[new_id] = channel[chan_id];
			SetChannelVolume(0, new_id, true);
		}
	}
	channel[chan_id] = NULL;
}

void BassSoundEngine::Update(float dwTimeMilliSecond)
{
	if(ScreenState==SCREEN_GAMEMAIN)
	{
		if(BASS_ChannelIsActive(channel[0])!=BASS_ACTIVE_PLAYING || !channel[0]){	
			if(m_pAudio)	// audio stopped, resume audio in video
				m_pAudio->put_Volume(Lin2dB(defaultVolume));
		}else{				// channel 0 is active
			if(m_pAudio)
				m_pAudio->put_Volume(-10000);

			if(pitch_factor!=0)
				SetPitch(pitch_factor,0,true), pitch_factor = 0;

			if(sampleRate_factor!=0)
				SetSampleRate(sampleRate_factor,0,true), sampleRate_factor = 0;

			if(tempo_factor!=0)
				SetTempo(tempo_factor,0,true), tempo_factor = 0;
		}
	}

	for(int i = 0; i < nowChannel; i++)
	{
		if(channel[i]!=NULL)
		{
			float volume;
			BASS_ChannelGetAttribute(channel[i], BASS_ATTRIB_VOL, &volume);
			if(volume==0){
				BASS_ChannelStop(channel[i]);
				channel[i] = NULL;
			}

			if(volume>volumeDes[i])
			{
				volume -= soundSpd*dwTimeMilliSecond;
				if(volume<volumeDes[i]) volume = volumeDes[i];
			}
			if(volume<volumeDes[i])
			{
				volume += soundSpd*dwTimeMilliSecond;
				if(volume>volumeDes[i]) volume = volumeDes[i];
			}
			BASS_ChannelSetAttribute(channel[i], BASS_ATTRIB_VOL, volume);

			if(volume==0)
				FreeChannel(i, true);
		}
	}
}

void BassSoundEngine::Clear()
{
	StopAllMusic();

	for( auto ptr = sample_pool.begin(); ptr != sample_pool.end(); ptr++ )
		BASS_SampleFree( ptr->second );
	sample_pool.clear();

	for( auto ptr = stream_pool.begin(); ptr != stream_pool.end(); ptr++ )
		BASS_StreamFree( ptr->second );
	stream_pool.clear();

	memset(channel,0,sizeof(channel));
	nowChannel = 0;
	intended_pitch = 1.0;
}

void BassSoundEngine::PlaySound(string filename, float volume)
{
	HSAMPLE sound = ReadSound(filename);
	HSTREAM channel = BASS_SampleGetChannel(sound, FALSE);
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume*defaultVolume);
	BASS_ChannelPlay(channel, TRUE);
}

HSAMPLE BassSoundEngine::ReadSound(string filename)
{
	HSAMPLE sound;
	if(sample_pool.find(filename)!=sample_pool.end())
		sound = sample_pool[filename];
	else{
		sound = BASS_SampleLoad(FALSE, filename.data(), 0, 0, maxChannel, 0);
		sample_pool[filename] = sound;
	}

	// get the music's sampling rate
	BASS_SAMPLE info;
	BASS_SampleGetInfo(sound, &info);
	music_sampleRate = info.freq;

	return sound;
}

HSTREAM BassSoundEngine::ReadMusic(string filename)
{
	HSTREAM sound;
	if(stream_pool.find(filename)!=stream_pool.end())
		sound = stream_pool[filename];
	else{
		sound = BASS_StreamCreateFile(FALSE, filename.data(), 0, 0, BASS_STREAM_PRESCAN|BASS_STREAM_DECODE);
		sound = BASS_FX_TempoCreate(sound, BASS_FX_FREESOURCE);
		BOOL hr;
		float sequence_ms = systemIni.BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS;
		hr = BASS_ChannelSetAttribute(sound, BASS_ATTRIB_TEMPO_OPTION_SEQUENCE_MS, sequence_ms);
		ERRCHECK(hr);
		float seekwindow_ms = systemIni.BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS;
		hr = BASS_ChannelSetAttribute(sound, BASS_ATTRIB_TEMPO_OPTION_SEEKWINDOW_MS, seekwindow_ms);
		ERRCHECK(hr);
		float overlap_ms = systemIni.BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS;
		hr = BASS_ChannelSetAttribute(sound, BASS_ATTRIB_TEMPO_OPTION_OVERLAP_MS, overlap_ms);
		ERRCHECK(hr);
		stream_pool[filename] = sound;
	}
	return sound;
}
HCHANNEL BassSoundEngine::PlayMusic(int sound_id, CHANNEL Ichannel, bool loop, float volume)
{
	if(Ichannel<0||Ichannel>=maxChannel){
		Ichannel = GetNewChannel();
		if(Ichannel<0)
			return NULL;
	}else
		FreeChannel(Ichannel);
	HSTREAM sound = playMusic[sound_id];
	channel[Ichannel] = sound;

	DWORD hr;
	bool useDSP = false;

	// set position
	float freq;
	if(Ichannel==0)
	{
		hr = BASS_ChannelGetAttribute(sound, BASS_ATTRIB_FREQ, &freq);
		hr = SetPosition(0, sound_id, 0);
		hr = BASS_FX_TempoGetRateRatio(sound);
		if(hr>0)
			useDSP = true;
		music_sampleRate = freq;
	}

	// set volume
	volumeDes[Ichannel] = volume*defaultVolume;
	hr = BASS_ChannelSetAttribute(sound, BASS_ATTRIB_VOL, volumeDes[Ichannel]);

	// start play
	hr = BASS_ChannelPlay(sound, useDSP?FALSE:TRUE);

	// set loop
	if(loop)
		hr = BASS_ChannelFlags(sound, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

	if(Ichannel==0)
		Update(0);

	return sound;
}

HCHANNEL BassSoundEngine::PlayMusic(string filename, CHANNEL Ichannel, bool loop, float volume)
{
	if(Ichannel<0||Ichannel>=maxChannel)
	{
		Ichannel = GetNewChannel();
		if(Ichannel<0)
			return NULL;
	}else
		FreeChannel(Ichannel);

	HSTREAM sound = ReadMusic(filename);

	DWORD hr;

	// start play
	hr = BASS_ChannelPlay(sound, TRUE);

	// set volume
	volumeDes[Ichannel] = volume*defaultVolume;
	hr = BASS_ChannelSetAttribute(sound, BASS_ATTRIB_VOL, volumeDes[Ichannel]);

	// set loop
	if(loop)
		hr = BASS_ChannelFlags(sound, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

	channel[Ichannel] = sound;

	return sound;
}

void BassSoundEngine::StopMusic(CHANNEL Ichannel, bool alphaout)
{
	FreeChannel(Ichannel, !alphaout);
}

void BassSoundEngine::StopAllMusic(bool alphaout)
{
	for(int i = 0; i < nowChannel; i++)
		FreeChannel(i, !alphaout);
}

void BassSoundEngine::PauseMusic(CHANNEL Ichannel, bool Pause)
{
	if(!Check(Ichannel))
		return;
	if(Pause)
		BASS_ChannelPause(channel[Ichannel]);
	else
		BASS_ChannelPlay(channel[Ichannel], FALSE);
	//channel[Ichannel]->setPaused(Pause);
}

void BassSoundEngine::PauseAllMusic(bool Pause)
{
	if(Pause){
		for(int i = 0; i < nowChannel; i++)
			if(channel[i]!=NULL)
				BASS_ChannelPause(channel[i]);
	}else{
		for(int i = 0; i < nowChannel; i++)
			if(channel[i]!=NULL)
				BASS_ChannelPlay(channel[i], FALSE);
	}
}

BOOL BassSoundEngine::SetPosition(int chan_id, int sound_id, double timeInSeconds)
{
	double tm_offset = timeInSeconds+playOffset[sound_id]*0.001;
	QWORD byte_posi = BASS_ChannelSeconds2Bytes(channel[chan_id], tm_offset);
	return BASS_ChannelSetPosition(channel[chan_id], byte_posi, BASS_POS_BYTE);
}


// FMOD sound system
void FmodSoundEngine::ERRCHECK(FMOD_RESULT result, bool bQuit)
{
	if (result != FMOD_OK)
	{
		char str[256];
		sprintf(str,"FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		if(bQuit){
			MessageBoxW(NULL, Ansi2WChar(str).data(), NULL, MB_OK|MB_ICONEXCLAMATION);
			exit(-1);
		}
		ShowMessage(str,2,0,32);
	}
}

void FmodSoundEngine::Init()
{
	FMOD_SPEAKERMODE speakermode;

	nowChannel = 0;
	sampleRate_factor = pitch_factor = tempo_factor = 0;
	intended_pitch = 1.0;
	pitchDSP = NULL;

	result = FMOD::System_Create(&system);
	ERRCHECK(result);

	result = system->setDSPBufferSize(systemIni.AudioDSPBufferSize, systemIni.AudioDSPBufferCount);
	ERRCHECK(result);

	result = system->init(maxChannel, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		result = system->setSoftwareFormat(defaultSampleRate, FMOD_SPEAKERMODE_STEREO, 0);
		ERRCHECK(result);

		result = system->init(maxChannel, FMOD_INIT_NORMAL, 0);
		ERRCHECK(result);
	}
	Clear();

	system->createSound("gamedata\\hit.wav", FMOD_DEFAULT, 0, &hitWav);
	ERRCHECK(result);
}

void FmodSoundEngine::Free()
{
	Clear();
	system->close();
	system->release();
}

void FmodSoundEngine::PlayHit(float factor)
{
	FMOD::Channel *channel;
	system->playSound(hitWav, NULL, false, &channel);
	float vol = systemIni.sndVolume*(0.5f+factor*0.25f);
	channel->setVolume(min(1.0f,vol)*defaultVolume);
	//channel->setPaused(false);
	//ERRCHECK(result);
	//ERRCHECK(result);
}

void FmodSoundEngine::Update(float dwTimeMilliSecond)
{
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	if(ScreenState==SCREEN_GAMEMAIN){
		bool status = false;
		channel[0]->isPlaying(&status);
		if(!status || !channel[0]){	// audio stopped, resume audio in video
			if(m_pAudio)
				m_pAudio->put_Volume(Lin2dB(defaultVolume));
		}else{			// channel 0 is active
			if(m_pAudio)
				m_pAudio->put_Volume(-10000);

			if(pitch_factor!=0)
				SetPitch(pitch_factor,0,true), pitch_factor = 0;

			if(sampleRate_factor!=0)
				SetSampleRate(sampleRate_factor,0,true), sampleRate_factor = 0;

			if(tempo_factor!=0)
				SetTempo(tempo_factor,0,true), tempo_factor = 0;
		}
	}
	system->update();
	for(int i = 0; i < nowChannel; i++)
	{
		if(channel[i]!=NULL)
		{
			float volume;
			result = channel[i]->getVolume(&volume);
			if(volume>volumeDes[i])
			{
				volume -= soundSpd*dwTimeMilliSecond;
				if(volume<volumeDes[i]) volume = volumeDes[i];
			}
			else if(volume<volumeDes[i])
			{
				volume += soundSpd*dwTimeMilliSecond;
				if(volume>volumeDes[i]) volume = volumeDes[i];
			}
			channel[i]->setVolume(volume);
			if(volume==0)
				FreeChannel(i, true);
		}
	}
}

CHANNEL FmodSoundEngine::GetNewChannel()
{
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	// Attempt to find an unused channel
	for( int x=1; x<maxChannel; x++)
		if(channel[x]==NULL){
			nowChannel = max(nowChannel, x+1);
			return x;
		}
	// Free all inactive channels
	for( int x=1; x<maxChannel; x++){
		bool status;
		FMOD_RESULT result = channel[x]->isPlaying(&status);
		if(!status)
			channel[x] = NULL;
	}
	// Attempt to find a free channel again
	for( int x=1; x<maxChannel; x++)
		if(channel[x]==NULL){
			nowChannel = max(nowChannel, x+1);
			return x;
		}
	return -1;
}

void FmodSoundEngine::Clear()
{
	StopAllMusic();
	nowChannel = 0;
	intended_pitch = 1.0;
	memset(volumeDes,0,sizeof(volumeDes));
	memset(channel,0,sizeof(channel));
	for(map<string,FMOD::Sound*>::iterator ptr = pool.begin(); ptr != pool.end(); ptr++)
		ptr->second->release();
	pool.clear();
}

void FmodSoundEngine::PlaySound(string filename, float volume)
{
	FMOD::Sound *sound;
	FMOD::Channel *channel;
	sound = (FMOD::Sound*)ReadMusic(Ansi2UTF8(filename).data());
	result = system->playSound(sound, NULL, true, &channel);
	//ERRCHECK(result);
	result = channel->setVolume((float)volume*defaultVolume);
	//ERRCHECK(result);
	result = channel->setPaused(false);
	//ERRCHECK(result);
}

DWORD FmodSoundEngine::ReadMusic(string filename)
{
	if(pool.find(filename)!=pool.end())
		return (DWORD)pool[filename];

	FMOD::Sound *sound;
	result = system->createStream(Ansi2UTF8(filename).data(), FMOD_ACCURATETIME|FMOD_MPEGSEARCH, NULL, &sound);
	if(result!=FMOD_OK)
		ShowMessage(StringTable(132)+" "+FMOD_ErrorString(result),2,0,32);
	pool[filename] = sound;

	if(!pitchDSP){
		result = system->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pitchDSP);
		ERRCHECK(result);
		result = pitchDSP->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, systemIni.FMOD_DSP_FFT_WINDOW);
		ERRCHECK(result);
		result = pitchDSP->setActive(true);
		ERRCHECK(result);
	}

	result = pitchDSP->disconnectAll(true, true);
	ERRCHECK(result);
	result = pitchDSP->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, 1.0f);
	ERRCHECK(result);

	return (DWORD)sound;
}

DWORD FmodSoundEngine::PlayMusic(int sound_id, CHANNEL Ichannel, bool loop, float volume)
{
	if(Ichannel<0||Ichannel>=maxChannel)
	{
		Ichannel = GetNewChannel();
		if(Ichannel<0)
			return NULL;
	}
	FMOD::Sound *sound = (FMOD::Sound*)playMusic[sound_id];
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	nowChannel = max(nowChannel,Ichannel+1);
	if(loop)
	{
		result = sound->setMode(FMOD_LOOP_NORMAL);
		//ERRCHECK(result);
	}
	if(Ichannel==0){
		float freq;
		sound->getDefaults(&freq,NULL);
		music_sampleRate = freq;
	}
	result = system->playSound(sound, NULL, true, &channel[Ichannel]);
	//ERRCHECK(result);
	volumeDes[Ichannel] = volume*defaultVolume;
	result = channel[Ichannel]->setVolume(volumeDes[Ichannel]);
	//ERRCHECK(result);
	SetPosition(Ichannel, sound_id, 0);
	//ERRCHECK(result);
	result = channel[Ichannel]->setPaused(false);
	//ERRCHECK(result);

	if(Ichannel==0)
		Update(0);

	return (DWORD)sound;
}

DWORD FmodSoundEngine::PlayMusic(string filename, CHANNEL Ichannel, bool loop, float volume)
{
	if(Ichannel<0||Ichannel>=maxChannel)
	{
		Ichannel = GetNewChannel();
		if(Ichannel<0)
		{
			PlaySound(filename,volume);
			return NULL;
		}
	}
	nowChannel = max(nowChannel,Ichannel+1);
	FMOD::Sound *sound = (FMOD::Sound*)ReadMusic(Ansi2UTF8(filename).data());
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;

	if(loop)
	{
		result = sound->setMode(FMOD_LOOP_NORMAL);
		//ERRCHECK(result);
	}
	result = system->playSound(sound, NULL, true, &channel[Ichannel]);
	//ERRCHECK(result);
	volumeDes[Ichannel] = volume*defaultVolume;
	result = channel[Ichannel]->setVolume(volumeDes[Ichannel]);
	//ERRCHECK(result);

	result = channel[Ichannel]->setPaused(false);
	//ERRCHECK(result);
	return (DWORD)sound;
}

void FmodSoundEngine::StopMusic(CHANNEL Ichannel, bool alphaout)
{
	if(!Check(Ichannel))
		return;
	bool iP1;
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	if(channel[Ichannel]!=NULL)
	{
		if(!alphaout)
		{
			channel[Ichannel]->isPlaying(&iP1);
			if(iP1)
				channel[Ichannel]->stop();
			channel[Ichannel] = NULL;
		}
		volumeDes[Ichannel] = 0;
	}
}

void FmodSoundEngine::StopAllMusic(bool alphaout)
{
	bool iP1;
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	for(int i = 0; i < nowChannel; i++)
		if(channel[i]!=NULL)
		{
			if(!alphaout)
			{
				result = channel[i]->isPlaying(&iP1);
				if(iP1)
				{
					result = channel[i]->stop();
				}
				channel[i] = NULL;
			}
			volumeDes[i] = 0;
		}
}

void FmodSoundEngine::PauseMusic(CHANNEL Ichannel, bool Pause)
{
	if(!Check(Ichannel))
		return;
	((FMOD::Channel*)channel[Ichannel])->setPaused(Pause);
}

void FmodSoundEngine::PauseAllMusic(bool Pause)
{
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	for(int i = 0; i < nowChannel; i++)
		if(channel[i]!=NULL)
			channel[i]->setPaused(Pause);
}

BOOL FmodSoundEngine::SetSampleRate(double factor, int channel_id, bool immediate)
{
	if(immediate){
		FMOD_RESULT result;

		float current_sampleRate;
		result = ((FMOD::Channel*)channel[channel_id])->getFrequency(&current_sampleRate);
		ERRCHECK(result);

		result = ((FMOD::Channel*)channel[channel_id])->setFrequency(music_sampleRate*factor);
		ERRCHECK(result);

		intended_pitch *= music_sampleRate*factor/current_sampleRate;
		return result==FMOD_OK;
	}else{
		sampleRate_factor = factor;
		return TRUE;
	}
}

BOOL FmodSoundEngine::SetPitch(double factor, int channel_id, bool immediate)
{
	if(immediate){
		float current_sampleRate;
		result = ((FMOD::Channel*)channel[channel_id])->getFrequency(&current_sampleRate);
		ERRCHECK(result);

		intended_pitch = factor;
		return SetAbsolutePitch(factor*music_sampleRate/current_sampleRate, channel_id);
	}else{
		pitch_factor = factor;
		return TRUE;
	}
}

BOOL FmodSoundEngine::SetAbsolutePitch(double factor, int channel_id)
{
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;

	result = channel[channel_id]->removeDSP(pitchDSP);
	ERRCHECK(result);

	result = pitchDSP->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, factor);
	ERRCHECK(result);

	result = channel[channel_id]->addDSP(0, pitchDSP);
	ERRCHECK(result);

	return result==FMOD_OK;
}

BOOL FmodSoundEngine::SetTempo(double factor, int channel_id, bool immediate)
{
	if(immediate){
		double _intended_pitch = intended_pitch;
		BOOL result = SetSampleRate(factor, channel_id, immediate);
		intended_pitch = _intended_pitch;
		result &= SetAbsolutePitch(_intended_pitch/factor, channel_id);
		return result;
	}else{
		tempo_factor = factor;
		return TRUE;
	}
}

BOOL FmodSoundEngine::SetChannelVolume(double volume, int channel_id, bool bAlpha)
{
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	volumeDes[channel_id] = volume*defaultVolume;
	if(!bAlpha)
		return channel[channel_id]->setVolume(volume*defaultVolume)==FMOD_OK;
}

void FmodSoundEngine::FreeChannel(int chan_id, bool bImmediate)
{
	if(channel[chan_id] == NULL)
		return;
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	FMOD_RESULT result;
	bool status;
	result = channel[chan_id]->isPlaying(&status);
	if(result==FMOD_OK && status)
	{
		if(bImmediate){
			channel[chan_id]->stop();
		}else{
			int new_id = GetNewChannel();
			channel[new_id] = channel[chan_id];
			SetChannelVolume(0, new_id, true);
		}
	}
	channel[chan_id] = NULL;
}

BOOL FmodSoundEngine::SetPosition(int chan_id, int sound_id, double timeInSeconds)
{
	FMOD::Channel **channel = (FMOD::Channel**)this->channel;
	int tm_offset = (int)(timeInSeconds*1000.0+playOffset[sound_id]+0.5);
	if(channel[chan_id])
		return channel[chan_id]->setPosition(tm_offset, FMOD_TIMEUNIT_MS);
	else
		return TRUE;
}

