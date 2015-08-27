/*****************************************************************************
 tempo.c/h/rc - Copyright (c) 2003-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
                                                      [http://www.jobnik.org]
                                                      [   bass_fx@jobnik.org]
 
 BASS_FX tempo / rate / pitch with dsp fx
 * Imports: bass.lib, bass_fx.lib
            kernel32.lib, user32.lib, comdlg32.lib, gdi32.lib
*****************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "bass.h"
#include "bass_fx.h"
#include "tempo.h"

HWND win=NULL;
HINSTANCE inst;

HSTREAM chan;			// tempo channel handle
HFONT Font;				// font handle
HFX fxEQ;				// dsp peaking eq handle
float oldfreq;			// old sample rate

#define MESS(id,m,w,l) SendDlgItemMessage(win,id,m,(WPARAM)w,(LPARAM)l)

OPENFILENAME ofn;
char path[MAX_PATH];

// display error dialogs
void Error(const char *es)
{
	char mes[200];
	sprintf(mes,"%s\n\n(error code: %d)",es,BASS_ErrorGetCode());
	MessageBox(win,mes,"Error",MB_ICONEXCLAMATION);
}

// update dsp eq
void UpdateFX(int b)
{
	BASS_BFX_PEAKEQ eq;
	int v = MESS(IDC_SLDEQ1+b,TBM_GETPOS,0,0);
    
	eq.lBand = b;	// get values of the selected band
    BASS_FXGetParameters(fxEQ, &eq);
		eq.fGain = (float)(10 - v);
	BASS_FXSetParameters(fxEQ, &eq);
}

// set dsp eq
void SetDSP_EQ(float fGain, float fBandwidth, float fQ, float fCenter_Bass, float fCenter_Mid, float fCenter_Treble)
{
	BASS_BFX_PEAKEQ eq;

	// set peaking equalizer effect with no bands
	fxEQ=BASS_ChannelSetFX(chan, BASS_FX_BFX_PEAKEQ,0);

	eq.fGain=fGain;
	eq.fQ=fQ;
	eq.fBandwidth=fBandwidth;
	eq.lChannel=BASS_BFX_CHANALL;

	// create 1st band for bass
	eq.lBand=0;
	eq.fCenter=fCenter_Bass;
	BASS_FXSetParameters(fxEQ, &eq);

	// create 2nd band for mid
	eq.lBand=1;
	eq.fCenter=fCenter_Mid;
	BASS_FXSetParameters(fxEQ, &eq);

	// create 3rd band for treble
	eq.lBand=2;
	eq.fCenter=fCenter_Treble;
	BASS_FXSetParameters(fxEQ, &eq);

	// update dsp eq
	UpdateFX(0);
	UpdateFX(1);
	UpdateFX(2);
}

// show the approximate position in MM:SS format
void UpdatePositionLabel(void)
{
	if(!BASS_FX_TempoGetRateRatio(chan)) return;
	{
		char c[30];
		float totalsec=(float)MESS(IDC_POS,TBM_GETRANGEMAX,0,0)/BASS_FX_TempoGetRateRatio(chan);
		float posec=(float)MESS(IDC_POS,TBM_GETPOS,0,0)/BASS_FX_TempoGetRateRatio(chan);
		sprintf(c,"Playing position: %02d:%02d / %02d:%02d", (int)posec/60,(int)posec%60,(int)totalsec/60,(int)totalsec%60);
		MESS(IDC_SPOS,WM_SETTEXT,0,c);
	}
}

// get the file name from the file path
char *GetFileName(const char *filepath)
{
	unsigned char slash_location;
	filepath = strrev(filepath);
	slash_location = strchr(filepath, '\\') - filepath;
	return (strrev(filepath) + strlen(filepath) - slash_location);
}

BOOL CALLBACK dialogproc(HWND h,UINT m,WPARAM w,LPARAM l)
{
	DWORD p;
	float freq;
	char c[30];

	switch (m) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case ID_OPEN:
					{
						char file[MAX_PATH]="";
						ofn.lpstrFilter="playable files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*aif\0All files\0*.*\0\0";
						ofn.lpstrFile=file;
						if (GetOpenFileName(&ofn)) {
							memcpy(path,file,ofn.nFileOffset);
							path[ofn.nFileOffset-1]=0;

							// free previous streams/dsp handles
							BASS_StreamFree(chan);

							// create decode stream channel
							chan=BASS_StreamCreateFile(FALSE, file, 0, 0, BASS_STREAM_DECODE);

							// create decode music channel
							if (!chan) chan=BASS_MusicLoad(FALSE, file, 0, 0, BASS_MUSIC_RAMP|BASS_MUSIC_PRESCAN|BASS_STREAM_DECODE,0);

							if (!chan){
								// not a WAV/MP3 or MOD
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Selected file couldn't be loaded!");
								break;
							}

							// update the position slider
							p=(DWORD)BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE));
							MESS(IDC_POS,TBM_SETRANGEMAX,0,p);
							MESS(IDC_POS,TBM_SETPOS,TRUE,0);

							// get the current sample rate
							BASS_ChannelGetAttribute(chan, BASS_ATTRIB_FREQ, &freq);
							oldfreq = freq;

							// create a new stream - decoded & resampled :)
							if (!(chan=BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP|BASS_FX_FREESOURCE))){
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Couldn't create a resampled stream!");
								BASS_StreamFree(chan);
								BASS_MusicFree(chan);
								break;
							}

							// set dsp eq to channel
							SetDSP_EQ(0.0f, 2.5f, 0.0f, 125.0f, 1000.0f, 8000.0f);

							// update the Button to show the loaded file name
							MESS(ID_OPEN,WM_SETTEXT,0,GetFileName(file));

							// set Volume
							p=MESS(IDC_VOL,TBM_GETPOS,0,0);
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, (float)p/100.0f);

							// update tempo sliders
							MESS(IDC_RATE,TBM_SETRANGEMAX,0,(long)(freq * 1.3f));
							MESS(IDC_RATE,TBM_SETRANGEMIN,0,(long)(freq * 0.7f));
							MESS(IDC_RATE,TBM_SETPOS,TRUE,(long)freq);
							MESS(IDC_RATE,TBM_SETPAGESIZE,0,(long)(freq * 0.01f));		// by 1%

							sprintf(c,"Samplerate = %dHz", (long)freq);
							MESS(IDC_SRATE,WM_SETTEXT,0,c);

							MESS(IDC_TEMPO,TBM_SETPOS,TRUE,0);
							MESS(IDC_STEMPO,WM_SETTEXT,0,"Tempo = 0%");

							MESS(IDC_PITCH,TBM_SETPOS,TRUE,0);
							MESS(IDC_SPITCH,WM_SETTEXT,0,"Pitch Scaling = 0 semitones");

							// update the approximate time in seconds view
							UpdatePositionLabel();

							// play new created stream
							BASS_ChannelPlay(chan,FALSE);
						}
					}
					return 1;
			}
			break;

		case WM_VSCROLL:
			{
				if(l){
					UpdateFX(GetDlgCtrlID((HWND)l)-IDC_SLDEQ1);
				}
			}
			break;

		case WM_HSCROLL:
			{
				if(!BASS_ChannelIsActive(chan)) break;

				switch (GetDlgCtrlID((HWND)l)) {
					case IDC_TEMPO:
							// set new tempo
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, (float)MESS(IDC_TEMPO, TBM_GETPOS, 0, 0));

							// update tempo static text
							sprintf(c,"Tempo = %d%%", MESS(IDC_TEMPO, TBM_GETPOS, 0, 0));
							MESS(IDC_STEMPO,WM_SETTEXT,0,c);
					case IDC_RATE:
							// set new samplerate
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_FREQ, (float)MESS(IDC_RATE, TBM_GETPOS, 0, 0));

							// update samplerate static text
							sprintf(c,"Samplerate = %dHz", MESS(IDC_RATE, TBM_GETPOS, 0, 0));
							MESS(IDC_SRATE,WM_SETTEXT,0,c);

							// update all bands fCenters after changing samplerate
							{
								BASS_BFX_PEAKEQ eq;
								int i;

								for(i=0;i<3;i++){
									eq.lBand = i;
									BASS_FXGetParameters(fxEQ, &eq);
										eq.fCenter = eq.fCenter * (float)MESS(IDC_RATE, TBM_GETPOS, 0, 0) / oldfreq;
									BASS_FXSetParameters(fxEQ, &eq);
								}
								oldfreq = (float)MESS(IDC_RATE, TBM_GETPOS, 0, 0);
							}
					case IDC_PITCH:
						{
							// set new pitch scale
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_PITCH, (float)MESS(IDC_PITCH, TBM_GETPOS, 0, 0));

							// update pitch static text
							sprintf(c,"Pitch Scaling = %d semitones", MESS(IDC_PITCH, TBM_GETPOS, 0, 0));
							MESS(IDC_SPITCH,WM_SETTEXT,0,c);

							// update the approximate time in seconds view
							UpdatePositionLabel();
						}
					break;
					case IDC_POS:
						// change the position
						if (LOWORD(w) == SB_ENDSCROLL) { // seek to new pos
							BASS_ChannelSetPosition(chan, (QWORD)BASS_ChannelSeconds2Bytes(chan, (double)MESS(IDC_POS,TBM_GETPOS,0,0)), BASS_POS_BYTE);
						}
						// update the approximate time in seconds view
						UpdatePositionLabel();
					break;
					case IDC_VOL:
						BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, (float)MESS(IDC_VOL,TBM_GETPOS,0,0)/100.0f);
					break;
				}
			}
			return 1;
			break;

		case WM_CLOSE:
			KillTimer(h,1);
			EndDialog(h, 0);
			return 1;
		break;

		case WM_INITDIALOG:
			win=h;
			GetCurrentDirectory(MAX_PATH,path);
			memset(&ofn,0,sizeof(ofn));
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=h;
			ofn.hInstance=inst;
			ofn.nMaxFile=MAX_PATH;
			ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;

			// setup output - default device, 44100hz, stereo, 16 bits
			if (!BASS_Init(-1,44100,0,win,NULL)) {
				Error("Can't initialize device");
				DestroyWindow(win);
				return 1;
			}

			// initialize dsp eq sliders
			MESS(IDC_SLDEQ1,TBM_SETRANGE,FALSE,MAKELONG(0,20));
			MESS(IDC_SLDEQ1,TBM_SETPOS,TRUE,10);

			MESS(IDC_SLDEQ2,TBM_SETRANGE,FALSE,MAKELONG(0,20));
			MESS(IDC_SLDEQ2,TBM_SETPOS,TRUE,10);

			MESS(IDC_SLDEQ3,TBM_SETRANGE,FALSE,MAKELONG(0,20));
			MESS(IDC_SLDEQ3,TBM_SETPOS,TRUE,10);

			// volume range
			MESS(IDC_VOL,TBM_SETRANGEMAX,0,100);
			MESS(IDC_VOL,TBM_SETPOS,TRUE,50);
			MESS(IDC_VOL,TBM_SETPAGESIZE,0,5);

			// tempo
			MESS(IDC_TEMPO,TBM_SETRANGEMAX,TRUE,30);
			MESS(IDC_TEMPO,TBM_SETRANGEMIN,TRUE,-30);
			MESS(IDC_TEMPO,TBM_SETPOS,TRUE,0);
			MESS(IDC_TEMPO,TBM_SETPAGESIZE,0,1);

			// rate
			MESS(IDC_RATE,TBM_SETRANGEMAX,0,(long)(44100.0f * 1.3f));
			MESS(IDC_RATE,TBM_SETRANGEMIN,0,(long)(44100.0f * 0.7f));
			MESS(IDC_RATE,TBM_SETPOS,TRUE,44100);
			MESS(IDC_RATE,TBM_SETPAGESIZE,0,(long)(44100.0f * 0.01f));	// by 1%

			// pitch
			MESS(IDC_PITCH,TBM_SETRANGEMAX,TRUE,30);
			MESS(IDC_PITCH,TBM_SETRANGEMIN,TRUE,-30);
			MESS(IDC_PITCH,TBM_SETPOS,TRUE,0);
			MESS(IDC_PITCH,TBM_SETPAGESIZE,0,1);

			// set bold font for IDC_SDSPEQ
			Font=CreateFont(-11,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_STROKE_PRECIS,
							CLIP_STROKE_PRECIS,DRAFT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"MS Sans Serif");
			MESS(IDC_SDSPEQ,WM_SETFONT,Font,0);
			return 1;
	}
	return 0;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
	inst=hInstance;

	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
		MessageBox(0,"An incorrect version of BASS.DLL was loaded (2.4 is required)","Incorrect BASS.DLL",MB_ICONERROR);
		return 1;
	}

	// check the correct BASS_FX was loaded
	if (HIWORD(BASS_FX_GetVersion())!=BASSVERSION) {
		MessageBox(0,"An incorrect version of BASS_FX.DLL was loaded (2.4 is required)","Incorrect BASS_FX.DLL",MB_ICONERROR);
		return 1;
	}

	DialogBox(inst,(char*)1000,0,&dialogproc);

	BASS_Free();

	DeleteObject(Font);

	return 0;
}