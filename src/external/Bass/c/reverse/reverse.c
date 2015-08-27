/*******************************************************************************
 reverse.c/h/rc - Copyright (c) 2002-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
                                                        [http://www.jobnik.org]
                                                        [   bass_fx@jobnik.org]
 
 BASS_FX playing in reverse with tempo & dx8 fx
 * Imports: bass.lib, bass_fx.lib, kernel32.lib, user32.lib, comdlg32.lib
*******************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "bass.h"
#include "bass_fx.h"
#include "reverse.h"

HWND win=NULL;
HINSTANCE inst;

HSTREAM chan;		// reversed tempo handle
HFX fx;				// dx8 reverb handle

#define MESS(id,m,w,l) SendDlgItemMessage(win,id,m,(WPARAM)w,(LPARAM)l)
#define DLGITEM(id) GetDlgItem(win,id)

OPENFILENAME ofn;
char path[MAX_PATH];

// set v/h scrollers
DWORD SetScroller(LPARAM l, WPARAM w, int _LINE, int _PAGE, DWORD _MAX)
{
	DWORD a = SendMessage((HWND)l,SBM_GETPOS,0,0);

	switch (LOWORD(w)) {
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			a = HIWORD(w);
		break;
		case SB_LINELEFT:
			if(a == 0)return a;
			a = max( a - _LINE, 0);
		break;
		case SB_LINERIGHT:
			a = (int)min(a + _LINE, _MAX);
		break;
		case SB_PAGELEFT:
			a = max(a - _PAGE, 0);
		break;
		case SB_PAGERIGHT:
			a = (int)min(a + _PAGE, _MAX);
		break;
	}
	return a;
}

// display error dialogs
void Error(const char *es)
{
	char mes[200];
	sprintf(mes,"%s\n\n(error code: %d)",es,BASS_ErrorGetCode());
	MessageBox(win,mes,"Error",MB_ICONEXCLAMATION);
}

// update dx8 reverb
void UpdateFX(int a)
{
	BASS_DX8_REVERB p;

	BASS_FXGetParameters(fx, &p);
		p.fReverbMix = -0.012f * (float)(a * a * a);
	BASS_FXSetParameters(fx, &p);
}

// show the position the file at
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
	DWORD p=0, PAGE=5, MAX=20, LINE=1;
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

							// free previous tempo, reverse & reverb handles
							BASS_StreamFree(chan);

#if 1 // with FX flag
							if (!(chan=BASS_StreamCreateFile(FALSE,file,0,0,BASS_STREAM_DECODE|BASS_STREAM_PRESCAN|BASS_SAMPLE_FX))
								&& !(chan=BASS_MusicLoad(FALSE,file,0,0,BASS_MUSIC_DECODE|BASS_MUSIC_RAMP|BASS_MUSIC_PRESCAN|BASS_SAMPLE_FX,0))) {
#else // without FX flag
							if (!(chan=BASS_StreamCreateFile(FALSE,file,0,0,BASS_STREAM_DECODE|BASS_STREAM_PRESCAN))
								&& !(chan=BASS_MusicLoad(FALSE,file,0,0,BASS_MUSIC_DECODE|BASS_MUSIC_RAMP|BASS_MUSIC_PRESCAN,0))) {
#endif
								// not a WAV/MP3 or MOD
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Selected file couldnt be loaded!");
								break;
							}

							// create new stream - decoded & reversed
							// 2 seconds decoding block as a decoding channel
							if (!(chan=BASS_FX_ReverseCreate(chan, 2, BASS_STREAM_DECODE|BASS_FX_FREESOURCE))) {
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Couldnt create a reversed stream!");
								BASS_StreamFree(chan);
								break;
							}

							// create a new stream - decoded & resampled
							if(!(chan=BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP|BASS_FX_FREESOURCE))) {
								MESS(ID_OPEN,WM_SETTEXT,0,"click here to open a file && play it...");
								Error("Couldnt create a resampled stream!");
								BASS_StreamFree(chan);
								break;
							}

							// update the Button to show the loaded file name
							MESS(ID_OPEN,WM_SETTEXT,0,GetFileName(file));

							// update tempo view
							MESS(IDC_TEMPO,TBM_SETPOS,TRUE,0);
							MESS(IDC_STEMPO,WM_SETTEXT,0,"Tempo = 0%");

							// set dx8 Reverb
							fx=BASS_ChannelSetFX(chan, BASS_FX_DX8_REVERB, 0);
							UpdateFX(MESS(IDC_VSRVB,SBM_GETPOS,0,0));

							// set Volume
							p=MESS(IDC_VOLUME,TBM_GETPOS,0,0);
							BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL,(float)p/100.0f);

							// set max to position slider
							p=(DWORD)BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE));

							// update the position slider
							MESS(IDC_POS,TBM_SETPOS,TRUE,0);
							MESS(IDC_POS,TBM_SETRANGEMAX,0,(long)p);
							MESS(IDC_POS,TBM_SETPOS,TRUE,(long)p);

							// update the approximate time in seconds view
							UpdatePositionLabel();

							// play the new stream
							BASS_ChannelPlay(chan, FALSE);
						}
					}
					return 1;
				case IDC_BTNDIR:
					{
						DWORD srcChan=BASS_FX_TempoGetSource(chan);
						float dir;
						BASS_ChannelGetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, &dir);

						if (dir<0) {
							BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_FORWARD);
							MESS(IDC_BTNDIR,WM_SETTEXT,0,"Playing Direction - Forward");
						} else {
							BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_REVERSE);
							MESS(IDC_BTNDIR,WM_SETTEXT,0,"Playing Direction - Reverse");
						}
					}
					return 1;
			}
			break;

			case WM_VSCROLL:
				if (!BASS_ChannelIsActive(chan)) break;
				switch (GetDlgCtrlID((HWND)l)) {
					case IDC_TEMPO:
						BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, (float)MESS(IDC_TEMPO, TBM_GETPOS, 0, 0) * -1.0f);
						sprintf(c,"Tempo = %d%%", MESS(IDC_TEMPO, TBM_GETPOS, 0, 0) * -1);
						MESS(IDC_STEMPO,WM_SETTEXT,0,c);
						// update the approximate time in seconds view
						UpdatePositionLabel();
					break;
					case IDC_VSRVB:
						p=SetScroller(l,w,LINE,PAGE,MAX);
						SendMessage((HWND)l,SBM_SETPOS,p,1);
						UpdateFX(p);
					break;
				}
			break;

			case WM_HSCROLL:
				if(!BASS_ChannelIsActive(chan)) break;

				switch (GetDlgCtrlID((HWND)l)) {
					case IDC_POS:
						// change the position
						if (LOWORD(w) == SB_ENDSCROLL) { // seek to new pos
							p=MESS(IDC_POS,TBM_GETPOS,0,0);
							BASS_ChannelSetPosition(chan, (QWORD)BASS_ChannelSeconds2Bytes(chan, (double)p), BASS_POS_BYTE);
						}
						// update the approximate time in seconds view
						UpdatePositionLabel();
					break;
					case IDC_VOLUME:
						BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, (float)MESS(IDC_VOLUME,TBM_GETPOS,0,0)/100.0f);
					break;
				}
			break;
			return 1;

			case WM_CLOSE:
				EndDialog(h,0);
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

				{
					// check if DX8 features are available
					BASS_INFO bi={sizeof(bi)};
					BASS_GetInfo(&bi);
					if (bi.dsver<8) {
						Error("DirectX version 8 is not Installed!!!\nYou won't be able to use any DX8 Effects!");
						EnableWindow(DLGITEM(IDC_VSRVB),FALSE);
					}
				}

				// volume
				MESS(IDC_VOLUME,TBM_SETRANGEMAX,0,100);
				MESS(IDC_VOLUME,TBM_SETPOS,TRUE,50);
				MESS(IDC_VOLUME,TBM_SETPAGESIZE,0,5);

				// tempo
				MESS(IDC_TEMPO,TBM_SETRANGEMIN,TRUE,-30);
				MESS(IDC_TEMPO,TBM_SETRANGEMAX,TRUE,30);
				MESS(IDC_TEMPO,TBM_SETPOS,TRUE,0);
				MESS(IDC_TEMPO,TBM_SETPAGESIZE,0,1);

				//dx8 reverb
				MESS(IDC_VSRVB,SBM_SETRANGE,0,20);
				MESS(IDC_VSRVB,SBM_SETPOS,20,1);

				MESS(IDC_POS,TBM_SETPOS,TRUE,100);	// set position slider to end

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

	return 0;
}