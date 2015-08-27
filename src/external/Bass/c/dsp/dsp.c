/***************************************************************************
 dsp.c/h/rc - Copyright (c) 2002-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
                                                    [http://www.jobnik.org]
                                                    [   bass_fx@jobnik.org]
 
 BASS_FX dsp fx in action
 * Imports: bass.lib, bass_fx.lib
            kernel32.lib, user32.lib, comdlg32.lib, gdi32.lib
***************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "bass.h"
#include "bass_fx.h"
#include "dsp.h"

HWND win=NULL;
HINSTANCE inst;

HFONT Font;					// font handle
DWORD chan;					// a handle
DWORD floatable;			// floating-point channel support?
HFX fxEQ, fxPhaser;			// dsp fx handles
float oldfreq;				// old sample rate

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

// get the file name from the file path
char *GetFileName(char *filepath)
{
	unsigned char slash_location;
	filepath = strrev(filepath);
	slash_location = strchr(filepath, '\\') - filepath;
	return (strrev(filepath) + strlen(filepath) - slash_location);
}

BOOL CALLBACK dialogproc(HWND h,UINT m,WPARAM w,LPARAM l)
{
	DWORD a=0;
	float freq;
	char c[30];

	BASS_BFX_PEAKEQ eq;		// dsp peaking equalizer
	BASS_BFX_PHASER phs;	// dsp phaser

	switch (m) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case ID_OPEN:
					{
						char file[MAX_PATH]="";
						ofn.lpstrFilter="playable files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif\0All files\0*.*\0\0";
						ofn.lpstrFile=file;
						if (GetOpenFileName(&ofn)) {
							memcpy(path,file,ofn.nFileOffset);
							path[ofn.nFileOffset-1]=0;

							// free previous dsp effects & handles
							BASS_StreamFree(chan);	// free stream
							BASS_MusicFree(chan);	// free music

							if(!(chan=BASS_StreamCreateFile(FALSE, file, 0, 0, BASS_SAMPLE_LOOP|floatable))&&
								!(chan=BASS_MusicLoad(FALSE, file, 0, 0, BASS_MUSIC_LOOP|BASS_MUSIC_RAMP|floatable,0))){
								// not a WAV/MP3 or MOD
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Selected file couldnt be loaded!");
								break;
							}

							// update the Button to show the loaded file name
							MESS(ID_OPEN,WM_SETTEXT,0,GetFileName(file));

							// set dsp effects
							SendMessage(win,WM_COMMAND,IDC_CHKEQ,l);
							SendMessage(win,WM_COMMAND,IDC_CHKPHS,l);

							// get current sample rate
							BASS_ChannelGetAttribute(chan, BASS_ATTRIB_FREQ, &freq);
							oldfreq = freq;

							// set the dx sample rate & view
							MESS(IDC_DXRATE,TBM_SETRANGEMAX,0,(long)(freq * 1.3f));
							MESS(IDC_DXRATE,TBM_SETRANGEMIN,0,(long)(freq * 0.7f));
							MESS(IDC_DXRATE,TBM_SETPOS,TRUE,(long)freq);
							MESS(IDC_DXRATE,TBM_SETPAGESIZE,0,(long)(freq * 0.01f));	// by 1%

							sprintf(c,"DirectX Samplerate = %dHz", (long)freq);
							MESS(IDC_SDXRATE,WM_SETTEXT,0,c);

							// play it!
							BASS_ChannelPlay(chan, FALSE);
						}
					}
					return 1;

				case IDC_CHKEQ:
					if (MESS(IDC_CHKEQ,BM_GETCHECK,0,0))
						SetDSP_EQ(0.0f, 2.5f, 0.0f, 125.0f, 1000.0f, 8000.0f);
					else
						BASS_ChannelRemoveFX(chan, fxEQ);
				return 1;

				case IDC_CHKPHS:
					if(MESS(IDC_CHKPHS,BM_GETCHECK,0,0)){
						fxPhaser=BASS_ChannelSetFX(chan, BASS_FX_BFX_PHASER,0);

						BASS_FXGetParameters(fxPhaser, &phs);
							phs.fWetMix = (float)MESS(IDC_WETMIX,TBM_GETPOS,0,0) / 1000.0f;
							phs.fDryMix = (float)MESS(IDC_DRYMIX,TBM_GETPOS,0,0) / 1000.0f;
							phs.fFeedback = (float)MESS(IDC_FEEDBACK,TBM_GETPOS,0,0) / 1000.0f;
							phs.fRate = (float)MESS(IDC_RATE,TBM_GETPOS,0,0) / 10.0f;
							phs.fRange = (float)MESS(IDC_RANGE,TBM_GETPOS,0,0) / 10.0f;
							phs.fFreq = (float)MESS(IDC_FREQ,TBM_GETPOS,0,0) / 10.0f;
						BASS_FXSetParameters(fxPhaser, &phs);
					}else
						BASS_ChannelRemoveFX(chan, fxPhaser);
				return 1;

			}
			return 1;

		case WM_VSCROLL:
			if(l){
				UpdateFX(GetDlgCtrlID((HWND)l)-IDC_SLDEQ1);
			}
		return 1;

		case WM_HSCROLL:
			if(!BASS_ChannelIsActive(chan)) break;

			switch (GetDlgCtrlID((HWND)l)) {
				case IDC_DXRATE:
					BASS_ChannelSetAttribute(chan, BASS_ATTRIB_FREQ, (float)MESS(IDC_DXRATE, TBM_GETPOS, 0, 0));

					sprintf(c,"DirectX Samplerate = %dHz", MESS(IDC_DXRATE, TBM_GETPOS, 0, 0));
					MESS(IDC_SDXRATE,WM_SETTEXT,0,c);

					// update all bands fCenters after changing samplerate
					{
						int i;
						for(i=0;i<3;i++){
							eq.lBand = i;
							BASS_FXGetParameters(fxEQ, &eq);
								eq.fCenter = eq.fCenter * (float)MESS(IDC_DXRATE, TBM_GETPOS, 0, 0) / oldfreq;
							BASS_FXSetParameters(fxEQ, &eq);
						}
						oldfreq = (float)MESS(IDC_DXRATE, TBM_GETPOS, 0, 0);
					}
				break;

				case IDC_DRYMIX:
				case IDC_WETMIX:
				case IDC_FEEDBACK:
				case IDC_RATE:
				case IDC_RANGE:
				case IDC_FREQ:
					BASS_FXGetParameters(fxPhaser, &phs);
						phs.fWetMix = (float)MESS(IDC_WETMIX,TBM_GETPOS,0,0) / 1000.0f;
						phs.fDryMix = (float)MESS(IDC_DRYMIX,TBM_GETPOS,0,0) / 1000.0f;
						phs.fFeedback = (float)MESS(IDC_FEEDBACK,TBM_GETPOS,0,0) / 1000.0f;
						phs.fRate = (float)MESS(IDC_RATE,TBM_GETPOS,0,0) / 10.0f;
						phs.fRange = (float)MESS(IDC_RANGE,TBM_GETPOS,0,0) / 10.0f;
						phs.fFreq = (float)MESS(IDC_FREQ,TBM_GETPOS,0,0) / 10.0f;
					BASS_FXSetParameters(fxPhaser, &phs);
			}
			return 1;

		case WM_INITDIALOG:
			win=h;
			GetCurrentDirectory(MAX_PATH,path);
			memset(&ofn,0,sizeof(ofn));
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=h;
			ofn.hInstance=inst;
			ofn.nMaxFile=MAX_PATH;
			ofn.Flags=OFN_HIDEREADONLY|OFN_EXPLORER;

			// enable floating-point DSP
			BASS_SetConfig(BASS_CONFIG_FLOATDSP, TRUE);

			// setup output - default device, 44100hz, stereo, 16 bits
			if (!BASS_Init(-1,44100,0,win,NULL)) {
				Error("Can't initialize device");
				DestroyWindow(win);
				return 1;
			}

			// check for floating-point capability
			floatable = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, 0, 0);
			if (floatable) {
				BASS_StreamFree(floatable);  //woohoo!
			    floatable = BASS_SAMPLE_FLOAT;
			}

			// initialize dsp eq sliders
			MESS(IDC_SLDEQ1,TBM_SETRANGE,FALSE,MAKELONG(0,20));
			MESS(IDC_SLDEQ1,TBM_SETPOS,TRUE,10);

			MESS(IDC_SLDEQ2,TBM_SETRANGE,FALSE,MAKELONG(0,20));
			MESS(IDC_SLDEQ2,TBM_SETPOS,TRUE,10);

			MESS(IDC_SLDEQ3,TBM_SETRANGE,FALSE,MAKELONG(0,20));
			MESS(IDC_SLDEQ3,TBM_SETPOS,TRUE,10);

			// dx rate
			MESS(IDC_DXRATE,TBM_SETRANGEMAX,0,(long)(44100.0f*1.3f));
			MESS(IDC_DXRATE,TBM_SETRANGEMIN,0,(long)(44100.0f*0.7f));
			MESS(IDC_DXRATE,TBM_SETPOS,TRUE,44100);
			MESS(IDC_DXRATE,TBM_SETPAGESIZE,0,(long)(44100.0f*0.01f));	// by 1%

			// DryMix
			MESS(IDC_DRYMIX,TBM_SETRANGE,0,MAKELONG(-2000,2000));
			MESS(IDC_DRYMIX,TBM_SETPOS,TRUE,-999);

			// WetMix
			MESS(IDC_WETMIX,TBM_SETRANGE,0,MAKELONG(-2000,2000));
			MESS(IDC_WETMIX,TBM_SETPOS,TRUE,999);

			// Feedback
			MESS(IDC_FEEDBACK,TBM_SETRANGE,0,MAKELONG(-1000,1000));
			MESS(IDC_FEEDBACK,TBM_SETPOS,TRUE,-60);

			// Rate
			MESS(IDC_RATE,TBM_SETRANGE,0,MAKELONG(0,100));
			MESS(IDC_RATE,TBM_SETPOS,TRUE,2);

			// Range
			MESS(IDC_RANGE,TBM_SETRANGE,0,MAKELONG(0,100));
			MESS(IDC_RANGE,TBM_SETPOS,TRUE,60);

			// Freq
			MESS(IDC_FREQ,TBM_SETRANGE,0,MAKELONG(0,10000));
			MESS(IDC_FREQ,TBM_SETPOS,TRUE,1000);

			Font=CreateFont(-12,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_STRING_PRECIS,
				            CLIP_STROKE_PRECIS,DRAFT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,
							"MS Sans Serif");
			// set the font for check boxes
			MESS(IDC_CHKEQ, WM_SETFONT, Font, TRUE);
			MESS(IDC_CHKPHS,WM_SETFONT, Font, TRUE);
		return 1;

		case WM_CLOSE:
			EndDialog(h,0);
			return 0;
		break;
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