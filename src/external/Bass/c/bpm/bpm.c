/***************************************************************************
 bpm.c/h/rc - Copyright (c) 2003-2012 (: JOBnik! :) [Arthur Aminov, ISRAEL]
                                                    [http://www.jobnik.org]
                                                    [   bass_fx@jobnik.org]
 BASS_FX bpm with tempo & samplerate changers
 * Imports: bass.lib, bass_fx.lib
            kernel32.lib, user32.lib, comdlg32.lib, gdi32.lib, winmm.lib
***************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "bass.h"
#include "bass_fx.h"
#include "bpm.h"

HWND win=NULL;
HINSTANCE inst;

HFONT Font;				// new bpm static text font

DWORD chan;				// tempo channel handle
DWORD hBPM;				// decoding bpm handle
float orgBPM;			// original bpm

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

float GetNewBPM(DWORD hBPM)
{
	return BASS_FX_BPM_Translate(hBPM, BASS_FX_TempoGetRateRatio(chan)*100.0f, BASS_FX_BPM_TRAN_PERCENT2);

	// or you could do it this way too :)
	// return (orgBPM * BASS_FX_TempoGetRateRatio(chan));
}

// get the original bpm after period of time
void CALLBACK GetBPM_Callback(DWORD handle, float bpm, void *user)
{
	// don't bother to update the BPM view if it's zero
	if(bpm){
		char c[30];
		sprintf(c,"BPM: %0.2f", GetNewBPM(handle));
		MESS(IDC_SBPM,WM_SETTEXT,0,c);
	}
}

// get the beat position in seconds
void CALLBACK beatTimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    if (BASS_FX_TempoGetRateRatio(chan)){
		double beatpos;
		char c[30];

		beatpos = BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetPosition(chan, BASS_POS_BYTE)) / BASS_FX_TempoGetRateRatio(chan);
		sprintf(c,"Beat pos: %0.2f", beatpos);
		MESS(IDC_SBEAT,WM_SETTEXT,0,c);
	}
	timeKillEvent(uTimerID);
}

void CALLBACK GetBeatPos_Callback(DWORD handle, double beatpos, void *user)
{
	double curpos;
    curpos = BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetPosition(handle, BASS_POS_BYTE));
    timeSetEvent((UINT)((beatpos - curpos) * 1000.0f), 0, (LPTIMECALLBACK)beatTimerProc, user, TIME_ONESHOT);
}

// get the bpm process detection in percents of a decoding channel
void CALLBACK GetBPM_Process(DWORD chan, float per, void *user)
{
	// update the progress bar
	MESS(IDC_PRGRSBPM,PBM_SETPOS,(int)per,0);
}

void DecodingBPM(BOOL newStream, double startSec, double endSec, const char *fp)
{
	char c[30];

	if (newStream){
		// Open file for bpm decoding detection
		hBPM = BASS_StreamCreateFile(FALSE, fp, 0, 0, BASS_STREAM_DECODE);
		if (!hBPM) hBPM = BASS_MusicLoad(FALSE, fp, 0, 0, BASS_MUSIC_DECODE|BASS_MUSIC_PRESCAN, 0);
	}

	orgBPM = BASS_FX_BPM_DecodeGet(hBPM, startSec, endSec, 0, BASS_FX_BPM_BKGRND|BASS_FX_BPM_MULT2|BASS_FX_FREESOURCE, (BPMPROCESSPROC*)GetBPM_Process, 0);

	// don't bother to update the BPM view if it's zero
	if (orgBPM) {
		// update the bpm view
		sprintf(c,"BPM: %0.2f", GetNewBPM(hBPM));
		MESS(IDC_SBPM,WM_SETTEXT,0,c);
	}
}

// get the file name from the file path
char *GetFileName(const char *fp)
{
	unsigned char slash_location;
	fp = strrev(fp);
	slash_location = strchr(fp, '\\') - fp;
	return (strrev(fp) + strlen(fp) - slash_location);
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
						ofn.lpstrFilter="playable files\0*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif\0All files\0*.*\0\0";
						ofn.lpstrFile=file;
						if (GetOpenFileName(&ofn)) {
							memcpy(path,file,ofn.nFileOffset);
							path[ofn.nFileOffset-1]=0;

							// free decode bpm & stream
							BASS_FX_BPM_Free(hBPM);

							// free tempo, stream, music & bpm/beat callbacks
							BASS_StreamFree(chan);

							// create decode stream channel
							chan=BASS_StreamCreateFile(FALSE, file, 0, 0, BASS_STREAM_DECODE);

							// create decode music channel
							if (!chan) chan = BASS_MusicLoad(FALSE, file, 0, 0, BASS_MUSIC_RAMP|BASS_MUSIC_PRESCAN|BASS_MUSIC_DECODE, 0);

							if (!chan){
								// not a WAV/MP3 or MOD
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Selected file couldn't be loaded!");
								break;
							}

							// get current samplerate
							BASS_ChannelGetAttribute(chan, BASS_ATTRIB_FREQ, &freq);

							// update the position slider
							p = (DWORD)BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE));
							MESS(IDC_POS,TBM_SETRANGEMAX,0,p);
							MESS(IDC_POS,TBM_SETPOS,TRUE,0);

							// create a new stream - decoded & resampled :)
							if (!(chan=BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP|BASS_FX_FREESOURCE))){
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Couldn't create a resampled stream!");
								BASS_StreamFree(chan);
								BASS_MusicFree(chan);
								break;
							}

							// update the button to show the loaded file name
							MESS(ID_OPEN,WM_SETTEXT,0,GetFileName(file));

							// set Volume
							p = MESS(IDC_VOL,TBM_GETPOS,0,0);
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, (float)(100 - p)/100.0f);

							// update tempo slider & view
							MESS(IDC_TEMPO,TBM_SETPOS,TRUE,0);
							MESS(IDC_STEMPO,WM_SETTEXT,0,"Tempo = 0%");

							// set rate min/max values according to current frequency
							MESS(IDC_RATE,TBM_SETRANGEMAX,0,(long)(freq * 1.3f));
							MESS(IDC_RATE,TBM_SETRANGEMIN,0,(long)(freq * 0.7f));
							MESS(IDC_RATE,TBM_SETPOS,TRUE,(long)freq);
							MESS(IDC_RATE,TBM_SETPAGESIZE,0,(long)(freq * 0.01f));	// by 1%

							sprintf(c,"Samplerate = %dHz", (long)freq);
							MESS(IDC_SRATE,WM_SETTEXT,0,c);

							// update the approximate time in seconds view
							UpdatePositionLabel();

							// play new created stream
							BASS_ChannelPlay(chan,FALSE);

							// set the callback bpm and beat
							SendMessage(win,WM_COMMAND,IDC_CHKPERIOD,l);
							SendMessage(win,WM_COMMAND,IDC_CHKBEAT,l);

							// get the bpm of 30 seconds from the start
							DecodingBPM(TRUE, 0.0f, 30.0f, file);
						}
					}
				return 1;

				case IDC_CHKPERIOD:
					if(MESS(IDC_CHKPERIOD,BM_GETCHECK,0,0)){
						GetDlgItemText(win, IDC_EPBPM, c, 5);
						BASS_FX_BPM_CallbackSet(chan, (BPMPROC*)GetBPM_Callback, (double)atof(c), 0, BASS_FX_BPM_MULT2, 0);
					}else
						BASS_FX_BPM_Free(chan);
				return 1;

				case IDC_CHKBEAT:
					if(MESS(IDC_CHKBEAT,BM_GETCHECK,0,0)){
						BASS_FX_BPM_BeatCallbackSet(chan, (BPMBEATPROC*)GetBeatPos_Callback, 0);
					}else
						BASS_FX_BPM_BeatFree(chan);
				return 1;
			}
			break;

		case WM_VSCROLL:
			if(GetDlgCtrlID((HWND)l) == IDC_VOL)
				BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, (float)(100 - MESS(IDC_VOL,TBM_GETPOS,0,0))/100.0f);
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
						{
							// set new samplerate
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_FREQ, (float)MESS(IDC_RATE, TBM_GETPOS, 0, 0));

							sprintf(c,"Samplerate = %dHz", MESS(IDC_RATE, TBM_GETPOS, 0, 0));
							MESS(IDC_SRATE,WM_SETTEXT,0,c);

							// update the bpm view
							if (MESS(IDC_CHKPERIOD,BM_GETCHECK,0,0))
								sprintf(c,"BPM: %0.2f", GetNewBPM(chan));
							else
								sprintf(c,"BPM: %0.2f", GetNewBPM(hBPM));

							MESS(IDC_SBPM,WM_SETTEXT,0,c);

							// update the approximate time in seconds view
							UpdatePositionLabel();
						}
					break;
					case IDC_POS:
						// change the position
						if (LOWORD(w) == SB_ENDSCROLL) { // seek to new pos
							BASS_ChannelSetPosition(chan, (QWORD)BASS_ChannelSeconds2Bytes(chan, (double)MESS(IDC_POS,TBM_GETPOS,0,0)), BASS_POS_BYTE);

							// get the bpm of last IDC_EPBPM seconds
							GetDlgItemText(win, IDC_EPBPM, c, 5);
							DecodingBPM(FALSE, (double)MESS(IDC_POS,TBM_GETPOS,0,0) - (double)atof(c), (double)MESS(IDC_POS,TBM_GETPOS,0,0),"");
						}
						// update the approximate time in seconds view
						UpdatePositionLabel();
					break;
				}
			}
			return 1;
			break;

		case WM_CLOSE:
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
    
			// volume
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

			// bpm detection process
			MESS(IDC_PRGRSBPM,PBM_SETRANGE32,0,100);

			// set the bpm period edit box, as a default of 10 seconds :)
			MESS(IDC_EPBPM,WM_SETTEXT,0,"10");

			// set the bpm static text font
			Font = CreateFont(-12,0,0,0,FW_BOLD,0,0,0,0,0,0,0,0,"Tahoma");
			MESS(IDC_SBPM, WM_SETFONT, Font, TRUE);
			MESS(IDC_SBEAT, WM_SETFONT, Font, TRUE);
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