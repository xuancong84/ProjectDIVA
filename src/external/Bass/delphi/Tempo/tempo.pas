{==========================================================================
 tempo.pas - Copyright (c) 2003-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
                                                   [http://www.jobnik.org]
                                                   [   bass_fx@jobnik.org]

 BASS_FX tempo / pitch / rate with dsp fx
 * Requires: BASS 2.4 (available @ www.un4seen.com)
==========================================================================}

unit tempo;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls, BASS, BASS_FX;

type
  TfrmTempo = class(TForm)
    GroupBox: TGroupBox;
    lbl125Hz: TLabel;
    lbl1KHz: TLabel;
    lbl8KHz: TLabel;
    lblTempo: TLabel;
    lblSamplerate: TLabel;
    lblPitch: TLabel;
    lblPosition: TLabel;
    vsEQBASS: TScrollBar;
    vsEQMID: TScrollBar;
    vsEQTREBLE: TScrollBar;
    tbTempo: TTrackBar;
    tbPitch: TTrackBar;
    tbSamplerate: TTrackBar;
    btnOpen: TButton;
    od: TOpenDialog;
    tbVolume: TTrackBar;
    lblVolume: TLabel;
    lblDSPEQ: TLabel;
    hsPosition: TScrollBar;
    procedure FormCreate(Sender: TObject);
    procedure tbVolumeChange(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure btnOpenClick(Sender: TObject);
    procedure lblTempoClick(Sender: TObject);
    procedure lblSamplerateClick(Sender: TObject);
    procedure lblPitchClick(Sender: TObject);
    procedure tbTempoChange(Sender: TObject);
    procedure tbSamplerateChange(Sender: TObject);
    procedure tbPitchChange(Sender: TObject);
    procedure hsPositionChange(Sender: TObject);
    procedure hsPositionScroll(Sender: TObject; ScrollCode: TScrollCode; var ScrollPos: Integer);
    procedure vsEQBASSChange(Sender: TObject);
    procedure vsEQMIDChange(Sender: TObject);
    procedure vsEQTREBLEChange(Sender: TObject);
    procedure Error(msg: string);
    procedure UpdatePositionLabel();
    procedure UpdateFX(band, pos: integer);
    procedure SetDSP_EQ(fGain, fBandwidth, fQ, fCenter_Bass, fCenter_Mid, fCenter_Treble: FLOAT);
  private
    { Private declarations }
    chan: DWORD;               // tempo channel handle
    fxEQ: DWORD;               // dsp peaking eq handle
    eq: BASS_BFX_PEAKEQ;       // dsp peaking eq
    info: BASS_CHANNELINFO;    // channel info
    freq: FLOAT;               // sample rate
    oldfreq: FLOAT;            // old sample rate
  public
    { Public declarations }
  end;

var
  frmTempo: TfrmTempo;

implementation

{$R *.DFM}

// display error dialogs
procedure TFrmTempo.Error(msg: string);
var s: string;
begin
    s := msg + #13#10 + #13#10 + '(error code: ' + IntToStr(BASS_ErrorGetCode) + ')';

  MessageBox(handle, PChar(s), 'Error', MB_ICONEXCLAMATION);
end;

procedure TfrmTempo.FormCreate(Sender: TObject);
begin
  // check the correct BASS was loaded
  If (HiWord(BASS_GetVersion()) <> BASSVERSION) then begin
    application.MessageBox('An incorrect version of BASS.DLL was loaded (2.4 is required)', 'Incorrect BASS.DLL', MB_ICONERROR or MB_OK);
    application.Terminate;
  end;

  // check the correct BASS_FX was loaded
  If (HiWord(BASS_FX_GetVersion()) <> BASSVERSION) then begin
    application.MessageBox('An incorrect version of BASS_FX.DLL was loaded (2.4 is required)', 'Incorrect BASS_FX.DLL', MB_ICONERROR or MB_OK);
    application.Terminate;
  end;

  // setup output - default device, 44100hz, stereo, 16 bits
  If BASS_Init(-1, 44100, 0, frmTempo.handle, nil) = false then begin
    Error('Couldnt Initialize Digital Output');
    application.Terminate;
  end;

  freq := 44100;
end;

procedure TfrmTempo.FormDestroy(Sender: TObject);
begin
  BASS_Free;    // free BASS & BASS_FX
end;

procedure TfrmTempo.tbVolumeChange(Sender: TObject);
begin
  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, tbVolume.position / 100);
end;

procedure TfrmTempo.btnOpenClick(Sender: TObject);
begin
  od.Filename := '';
  od.Filter :=  'Playable files|*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif|All files|*.*';
  If od.Execute = false then exit;

  // free previous streams/music/dsps
  BASS_StreamFree(chan);

  // create decode channel
  chan := BASS_StreamCreateFile(FALSE, pChar(od.filename), 0, 0, BASS_STREAM_DECODE);

  // check for MOD
  If chan = 0 Then chan := BASS_MusicLoad(FALSE, pChar(od.filename), 0, 0, BASS_MUSIC_RAMP or BASS_MUSIC_PRESCAN or BASS_MUSIC_DECODE, 0);

  If chan = 0 then begin
    btnOpen.Caption := 'click here to open a file && play it...';
    Error('Selected file couldnt be loaded!');
    Exit;
  end;

  // get channel info
  BASS_ChannelGetInfo(chan, info);

  freq := info.freq;
  oldfreq := freq;

  // set MAX length to hsPosition Scroller in seconds
  hsPosition.max := round(BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE)));
  hsPosition.position := 0;

  // create a new stream - decoded & resampled :)
  chan := BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP or BASS_FX_FREESOURCE);
  If (chan = 0) Then begin
    btnOpen.Caption := 'click here to open a file && play it...';
    Error('Couldn''t create a resampled stream!');
    BASS_StreamFree(chan);
    BASS_MusicFree(chan);
    Exit;
  End;

  // set dsp eq to channel
  SetDSP_EQ(0, 2.5, 0, 125, 1000, 8000);

  // update the Button to show the loaded file name
  btnOpen.Caption := ExtractFileName(pChar(od.Filename));

  // set Volume
  tbVolume.OnChange(Self);

  // set rate min/max values according to current frequency
  With tbSamplerate do begin
    min := 0;    // to avoit min > max
    max := 1;    // to avoid max < min
    max := round(freq * 1.3);
    min := round(freq * 0.7);
    pagesize := round(freq * 0.01);      // by 1%
  End;

  // update the Rate label hint
  lblSamplerate.hint := 'click to set Samplerate to ' + FloatToStr(freq) + 'Hz';

  // update the approximate time in seconds view
  UpdatePositionLabel;

  // play new created stream
  BASS_ChannelPlay(chan, FALSE);

  // set tempo/rate/pitch to normal
  lblTempo.OnClick(Self);
  lblSamplerate.OnClick(Self);
  lblPitch.OnClick(Self);
end;

procedure TfrmTempo.lblTempoClick(Sender: TObject);
begin
  tbTempo.position := 0;
  tbTempo.OnChange(Self);
end;

procedure TfrmTempo.lblSamplerateClick(Sender: TObject);
begin
  tbSamplerate.position := Round(freq);
  tbSamplerate.OnChange(Self);
end;

procedure TfrmTempo.lblPitchClick(Sender: TObject);
begin
  tbPitch.position := 0;
  tbPitch.OnChange(Self);
end;

procedure TfrmTempo.tbTempoChange(Sender: TObject);
begin
  If (BASS_ChannelIsActive(chan) = 0) Then Exit;

  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, tbTempo.position);
  lblTempo.Caption := 'Tempo = ' + IntToStr(tbTempo.position) + '%';

  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmTempo.tbSamplerateChange(Sender: TObject);
var i: DWORD;
begin
  If (BASS_ChannelIsActive(chan) = 0) Then Exit;

  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_FREQ, tbSamplerate.position);
  lblSamplerate.Caption := 'Samplerate = ' + IntToStr(tbSamplerate.position) + 'Hz';

  // update all bands fCenters after changing samplerate
  for i := 0 to 2 do begin
    eq.lBand := i;
    BASS_FXGetParameters(fxEQ, @eq);
      eq.fCenter := eq.fCenter * tbSamplerate.position / oldfreq;
    BASS_FXSetParameters(fxEQ, @eq);
  end;

  oldfreq := tbSamplerate.position;

  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmTempo.tbPitchChange(Sender: TObject);
begin
  If (BASS_ChannelIsActive(chan) = 0) Then Exit;

  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_PITCH, tbPitch.position);
  lblPitch.Caption := 'Pitch Scaling = ' + IntToStr(tbPitch.position) + 'semitones';

  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmTempo.hsPositionChange(Sender: TObject);
begin
  If (BASS_ChannelIsActive(chan) = 0) then exit;
  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmTempo.hsPositionScroll(Sender: TObject; ScrollCode: TScrollCode; var ScrollPos: Integer);
begin
  If (BASS_ChannelIsActive(chan) = 0) then exit;
  if ScrollCode = scEndScroll then BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, hsPosition.position), BASS_POS_BYTE);
  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmTempo.vsEQBASSChange(Sender: TObject);
begin
  UpdateFX(0, vsEQBASS.Position);
end;

procedure TfrmTempo.vsEQMIDChange(Sender: TObject);
begin
  UpdateFX(1, vsEQMID.Position);
end;

procedure TfrmTempo.vsEQTREBLEChange(Sender: TObject);
begin
  UpdateFX(2, vsEQTREBLE.Position);
end;

// Set DSP Peaking EQ
procedure TfrmTempo.SetDSP_EQ(fGain, fBandwidth, fQ, fCenter_Bass, fCenter_Mid, fCenter_Treble: FLOAT);
begin
  // set peaking equalizer effect with no bands
  fxEQ := BASS_ChannelSetFX(chan, BASS_FX_BFX_PEAKEQ, 0);

  eq.fGain := fGain;
  eq.fQ := fQ;
  eq.fBandwidth := fBandwidth;
  eq.lChannel := BASS_BFX_CHANALL;

  // create 1st band for bass
  eq.lBand := 0;
  eq.fCenter := fCenter_Bass;
  BASS_FXSetParameters(fxEQ, @eq);

  // create 2nd band for mid
  eq.lBand := 1;
  eq.fCenter := fCenter_Mid;
  BASS_FXSetParameters(fxEQ, @eq);

  // create 3rd band for treble
  eq.lBand := 2;
  eq.fCenter := fCenter_Treble;
  BASS_FXSetParameters(fxEQ, @eq);

  // update dsp eq
  UpdateFX(0, vsEQBASS.position);
  UpdateFX(1, vsEQMID.position);
  UpdateFX(2, vsEQTREBLE.position);
End;

// update dsp peaking eq
procedure TfrmTempo.UpdateFX(band, pos: integer);
begin
  eq.lBand := band;     // get values of the selected band
  BASS_FXGetParameters(fxEQ, @eq);
    eq.fGain := 10 - pos;
  BASS_FXSetParameters(fxEQ, @eq);
End;

// show the approximate position in MM:SS format
procedure TfrmTempo.UpdatePositionLabel();
var totalsec, posec: FLOAT;
begin
  if(BASS_FX_TempoGetRateRatio(chan)>0) then begin
    totalsec := hsPosition.max / BASS_FX_TempoGetRateRatio(chan);
    posec := hsPosition.position / BASS_FX_TempoGetRateRatio(chan);

    lblPosition.Caption := 'Playing position: ' +
          format('%2.2d:%2.2d', [trunc(posec) div 60, trunc(posec) mod 60]) + ' / ' +
          format('%2.2d:%2.2d', [trunc(totalsec) div 60, trunc(totalsec) mod 60]);
  end;
end;

end.
