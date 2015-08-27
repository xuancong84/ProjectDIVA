{========================================================================
 dsp.pas - Copyright (c) 2003-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
                                                 [http://www.jobnik.org]
                                                 [   bass_fx@jobnik.org]

 BASS_FX dsp fx in action
 * Requires: BASS 2.4 (available @ www.un4seen.com)
========================================================================}

unit dsp;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls, BASS, BASS_FX;

type
  TfrmDSP = class(TForm)
    btnOpen: TButton;
    od: TOpenDialog;
    GroupBox: TGroupBox;
    lbl1KHz: TLabel;
    lbl8KHz: TLabel;
    lblDryMix: TLabel;
    lblWetMix: TLabel;
    lblFeedback: TLabel;
    lblDX: TLabel;
    lbl125Hz: TLabel;
    lblRate: TLabel;
    vsEQBASS: TScrollBar;
    vsEQMID: TScrollBar;
    vsEQTREBLE: TScrollBar;
    chkPhaser: TCheckBox;
    tbDryMix: TTrackBar;
    tbWetMix: TTrackBar;
    tbFeedback: TTrackBar;
    tbDXRate: TTrackBar;
    chkEqualizer: TCheckBox;
    tbRate: TTrackBar;
    lblRange: TLabel;
    tbRange: TTrackBar;
    lblFreq: TLabel;
    tbFreq: TTrackBar;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure btnOpenClick(Sender: TObject);
    procedure lblDXClick(Sender: TObject);
    procedure tbDXRateChange(Sender: TObject);
    procedure chkEqualizerClick(Sender: TObject);
    procedure vsEQBASSChange(Sender: TObject);
    procedure vsEQMIDChange(Sender: TObject);
    procedure vsEQTREBLEChange(Sender: TObject);
    procedure chkPhaserClick(Sender: TObject);
    procedure tbDryMixChange(Sender: TObject);
    procedure tbWetMixChange(Sender: TObject);
    procedure tbFeedbackChange(Sender: TObject);
    procedure Error(msg: string);
    procedure UpdateEQ(b, pos: integer);
    procedure UpdatePHS();
  private
    { Private declarations }
    chan: DWORD;               // a handle
    freq: FLOAT;               // sample rate
    oldfreq: FLOAT;            // old sample rate
    floatable: DWORD;	       // floating-point channel support?
    fxEQ : DWORD;              // dsp peaking eq handle
    fxPhaser : DWORD;          // dsp phaser handle
    eq  : BASS_BFX_PEAKEQ;   // dsp peaking equalizer
    phs : BASS_BFX_PHASER;   // dsp phaser
  public
    { Public declarations }
  end;

var
  frmDSP: TfrmDSP;

implementation

{$R *.DFM}

// display error dialogs
procedure TFrmDSP.Error(msg: string);
var s: string;
begin
    s := msg + #13#10 + #13#10 + '(error code: ' + IntToStr(BASS_ErrorGetCode) + ')';

  MessageBox(handle, PChar(s), 'Error', MB_ICONEXCLAMATION);
end;

procedure TfrmDSP.FormCreate(Sender: TObject);
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

  // enable floating-point DSP
  BASS_SetConfig(BASS_CONFIG_FLOATDSP, 1);

  // setup output - default device, 44100hz, stereo, 16 bits
  If BASS_Init(-1, 44100, 0, frmDSP.handle, nil) = false then begin
    Error('Couldnt Initialize Digital Output');
    application.Terminate;
  end;

  // check for floating-point capability
  floatable := BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, nil, nil);
  if (floatable > 0) then begin
    BASS_StreamFree(floatable);  //woohoo!
    floatable := BASS_SAMPLE_FLOAT;
  end;

  freq := 44100;
end;

procedure TfrmDSP.FormDestroy(Sender: TObject);
begin
 BASS_Free;    // free BASS & BASS_FX
end;

procedure TfrmDSP.btnOpenClick(Sender: TObject);
begin
  od.Filename := '';
  od.Filter :=  'Playable files|*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif|All files|*.*';
  If od.Execute = false then exit;

  // free previous streams/music/dsps handles
  BASS_StreamFree(chan);
  BASS_MusicFree(chan);

  // check for a stream
  chan := BASS_StreamCreateFile(FALSE, pChar(od.filename), 0, 0, BASS_SAMPLE_LOOP Or floatable);

  // check for MOD
  If chan = 0 Then chan := BASS_MusicLoad(FALSE, pChar(od.filename), 0, 0, BASS_MUSIC_LOOP Or BASS_MUSIC_RAMP Or floatable, 0);

  If chan = 0 then begin
    btnOpen.Caption := 'click here to open a file && play it...';
    Error('Selected file couldnt be loaded!');
    Exit;
  end;

  // set dsp effects
  chkEqualizer.OnClick(Self);
  chkPhaser.OnClick(Self);

  // set the dx sample rate
  BASS_ChannelGetAttribute(chan, BASS_ATTRIB_FREQ, freq);
  oldfreq := freq;

  with tbDXRate do begin
    min := 0;    // to avoit min > max
    max := 1;    // to avoid max < min
    max := round(freq * 1.3);
    min := round(freq * 0.7);
    pagesize := round(freq * 0.01);      // by 1%
  end;

  // update the button to show the loaded filename
  btnOpen.Caption := ExtractFileName(pChar(od.Filename));

  // update lblDX Hint
  lblDX.hint := 'Click to set DX Samplerate to ' + FloatToStr(freq) + 'Hz';

  // it must be one of them :)
  BASS_ChannelPlay(chan, FALSE);

  // update the dx sample rate to normal
  lblDX.OnClick(self);
end;

procedure TfrmDSP.lblDXClick(Sender: TObject);
begin
  tbDXRate.Position := Round(freq);
  tbDXRate.OnChange(self);
end;

procedure TfrmDSP.tbDXRateChange(Sender: TObject);
var i: DWORD;
begin
  If BASS_ChannelIsActive(chan) = 0 Then Exit;

  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_FREQ, tbDXRate.position);

  lblDX.Caption := 'DirectX Samplerate = ' + IntToStr(tbDXRate.position) + 'Hz';

  // update all bands fCenters after changing samplerate
  for i := 0 to 2 do begin
    eq.lBand := i;
    BASS_FXGetParameters(fxEQ, @eq);
      eq.fCenter := eq.fCenter * tbDXRate.position / oldfreq;
    BASS_FXSetParameters(fxEQ, @eq);
  end;

  oldfreq := tbDXRate.position;
end;

procedure TfrmDSP.chkEqualizerClick(Sender: TObject);
begin
  If chkEqualizer.checked Then begin

    // set peaking equalizer effect with no bands
    fxEQ := BASS_ChannelSetFX(chan, BASS_FX_BFX_PEAKEQ, 0);

    with eq do begin
      fBandwidth := 2.5;
      FQ := 0;
      fGain := 0;
      lChannel := BASS_BFX_CHANALL;

      // create 1st band for bass
      eq.lBand := 0;
      eq.fCenter := 125;
      BASS_FXSetParameters(fxEQ, @eq);

      // create 2nd band for mid
      eq.lBand := 1;
      eq.fCenter := 1000;
      BASS_FXSetParameters(fxEQ, @eq);

      // create 3rd band for treble
      eq.lBand := 2;
      eq.fCenter := 8000;
      BASS_FXSetParameters(fxEQ, @eq);
    end;

    // update dsp eq
    vsEQBASS.OnChange(Self);
    vsEQMID.OnChange(Self);
    vsEQTREBLE.OnChange(Self);
  end
  Else
    BASS_ChannelRemoveFX(chan, fxEQ);
end;

procedure TfrmDSP.vsEQBASSChange(Sender: TObject);
begin
  UpdateEQ(0, vsEQBASS.Position);
end;

procedure TfrmDSP.vsEQMIDChange(Sender: TObject);
begin
  UpdateEQ(1, vsEQMID.Position);
end;

procedure TfrmDSP.vsEQTREBLEChange(Sender: TObject);
begin
  UpdateEQ(2, vsEQTREBLE.Position);
end;

procedure TfrmDSP.chkPhaserClick(Sender: TObject);
begin
  If chkPhaser.checked Then begin
    fxPhaser := BASS_ChannelSetFX(chan, BASS_FX_BFX_PHASER, 1);
      tbDryMix.OnChange(Self);
      tbWetMix.OnChange(Self);
      tbFeedback.OnChange(Self);
      tbRate.OnChange(Self);
      tbRange.OnChange(Self);
      tbFreq.OnChange(Self);
  end
  Else
    BASS_ChannelRemoveFX(chan, fxPhaser);
end;

procedure TfrmDSP.tbDryMixChange(Sender: TObject);
begin
  UpdatePHS;
end;

procedure TfrmDSP.tbWetMixChange(Sender: TObject);
begin
  UpdatePHS;
end;

procedure TfrmDSP.tbFeedbackChange(Sender: TObject);
begin
  UpdatePHS;
end;

// update the dsp eq fx
procedure TfrmDSP.UpdateEQ(b, pos: integer);
begin
  eq.lBand := b;    // get b band values

  BASS_FXGetParameters(fxEQ, @eq);
    eq.fGain := 10 - pos;
  BASS_FXSetParameters(fxEQ, @eq);
end;

// update the dsp phaser fx
procedure TfrmDSP.UpdatePHS();
begin
  BASS_FXGetParameters(fxPhaser, @phs);
    phs.fDryMix := tbDryMix.Position / 1000;
    phs.fWetMix := tbWetMix.Position / 1000;
    phs.fFeedback := tbFeedback.Position / 1000;
    phs.fRate := tbRate.Position / 10;
    phs.fRange := tbRange.Position / 10;
    phs.fFreq := tbFreq.Position / 10;
  BASS_FXSetParameters(fxPhaser, @phs);
end;

end.
