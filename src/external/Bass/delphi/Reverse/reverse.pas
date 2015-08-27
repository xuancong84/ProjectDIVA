{======================================================================
 reverse.pas - Copyright (c) 2003
                        Alex Graham @ http://www.ingenious.demon.co.uk

 BASS_FX playing in reverse with tempo & dx8 fx
 * Requires: BASS 2.4 (available @ www.un4seen.com)
======================================================================}
unit reverse;

interface

uses
  Windows, Messages, SysUtils, Classes, Controls, Forms, Dialogs,
  StdCtrls, ComCtrls, BASS, BASS_FX;

type
  TfrmReverse = class(TForm)
    open_but: TButton;
    OpenDialog1: TOpenDialog;
    GroupBox1: TGroupBox;
    tempo_lab: TLabel;
    dx_lab: TLabel;
    vol_lab: TLabel;
    pos_lab: TLabel;
    tempo_bar: TTrackBar;
    vol_bar: TTrackBar;
    reverb_bar: TScrollBar;
    pos_bar: TScrollBar;
    btnDirection: TButton;
    procedure reverb_barChange(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure open_butClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure tempo_labClick(Sender: TObject);
    procedure tempo_barChange(Sender: TObject);
    procedure vol_barChange(Sender: TObject);
////////////
    procedure UpdatePositionLabel;
    procedure Error(msg: string);
    procedure pos_barScroll(Sender: TObject; ScrollCode: TScrollCode;
      var ScrollPos: Integer);
    procedure pos_barChange(Sender: TObject);
    procedure btnDirectionClick(Sender: TObject);
////////////
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmReverse: TfrmReverse;
  chan : HSTREAM;     // Reverse handle
  fx : HFX;           // dx8 reverb handle

implementation

{$R *.DFM}

// display error dialogs
procedure TFrmReverse.Error(msg: string);
var s: string;
begin
    s := msg + #13#10 + #13#10 + '(error code: ' + IntToStr(BASS_ErrorGetCode) + ')';

  MessageBox(handle, PChar(s), 'Error', MB_ICONEXCLAMATION);
end;

procedure TfrmReverse.reverb_barChange(Sender: TObject);
var
  p : BASS_DX8_REVERB;
  v : integer;
begin
  v := reverb_bar.Position;
  BASS_FXGetParameters(fx, @p);
    p.fReverbMix := -0.012 * v * v * v;
  BASS_FXSetParameters(fx, @p);
end;

procedure TfrmReverse.FormCreate(Sender: TObject);
var
  bi : BASS_INFO;
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
  If BASS_Init(-1, 44100, 0, frmReverse.handle, nil) = false then begin
    Error('Couldnt Initialize Digital Output');
    application.Terminate;
  end;

  // check if DX8 features are available
  BASS_GetInfo(bi);

  If (bi.dsver < 8) then begin
    application.MessageBox('DirectX version 8 is not installed!!!' + #13#10 + 'You won''t be able to use any DX8 Effects!', 'DX8 Error', MB_OK);
    reverb_bar.Enabled := false;
  end;
end;

procedure TfrmReverse.open_butClick(Sender: TObject);
begin

  OpenDialog1.Filename := '';
  OpenDialog1.Filter :=  'Playable files|*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif|All files|*.*';
  If opendialog1.Execute = false then exit;

  // free previous tempo, reverse & reverb handles
  BASS_StreamFree(chan);

{$DEFINE ENFXFLAG}

{$IFDEF ENFXFLAG} // with FX flag

    chan := BASS_StreamCreateFile(FALSE, pChar(OpenDialog1.filename), 0, 0, BASS_STREAM_DECODE Or BASS_STREAM_PRESCAN Or BASS_SAMPLE_FX);
    If (chan = 0) Then chan := BASS_MusicLoad(FALSE, pChar(OpenDialog1.filename), 0, 0, BASS_MUSIC_DECODE Or BASS_MUSIC_RAMP Or BASS_MUSIC_PRESCAN Or BASS_SAMPLE_FX, 0);

{$ELSE} // without FX flag

    chan := BASS_StreamCreateFile(FALSE, pChar(OpenDialog1.filename), 0, 0, BASS_STREAM_DECODE Or BASS_STREAM_PRESCAN);
    If (chan = 0) Then chan := BASS_MusicLoad(FALSE, pChar(OpenDialog1.filename), 0, 0, BASS_MUSIC_DECODE Or BASS_MUSIC_RAMP Or BASS_MUSIC_PRESCAN, 0);

{$ENDIF}

{$UNDEF ENFXFLAG}

  If (chan = 0) then begin
    open_but.Caption := 'click here to open a file && play it...';
    Error('Selected file couldnt be loaded!');
    Exit;
  end;

  // set the sliders max length according to file length
  pos_bar.Max := trunc(BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE)));
  pos_bar.Position := pos_bar.Max;

  // create new stream - decoded & reversed
  // 2 seconds decoding block as a decoding channel
  chan := BASS_FX_ReverseCreate(chan, 2, BASS_STREAM_DECODE Or BASS_FX_FREESOURCE);
  If (chan = 0) then begin
    open_but.Caption := 'click here to open a file && play it...';
    Error('Couldnt create a reversed stream!');
    BASS_StreamFree(chan);
    Exit;
  end;

  // create a new stream - decoded & resampled
  chan := BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP Or BASS_FX_FREESOURCE);
  If (chan = 0) then begin
    open_but.Caption := 'click here to open a file && play it...';
    Error('Couldnt create a resampled stream!');
    BASS_StreamFree(chan);
    Exit;
  end;

  // update the button to show the loaded filename
  open_but.Caption := ExtractFileName(pChar(opendialog1.Filename));

  // set dx8 reverb
  fx := BASS_ChannelSetFX(chan, BASS_FX_DX8_REVERB, 0);
  reverb_bar.OnChange(self);

  // set volume
  vol_bar.OnChange(self);

  // update the approximate time in seconds view
  UpdatePositionLabel;

  // play the new stream
  BASS_ChannelPlay(chan, FALSE);

  // set the tempo to optimal
  tempo_lab.OnClick(self);
end;

procedure TfrmReverse.FormDestroy(Sender: TObject);
begin
 BASS_Free;    // free BASS & BASS_FX
end;

procedure TfrmReverse.tempo_labClick(Sender: TObject);
begin
  tempo_bar.Position := 0;
  tempo_bar.OnChange(self);
end;

procedure TfrmReverse.tempo_barChange(Sender: TObject);
begin
  If (BASS_ChannelIsActive(chan) = 0) then exit;
  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, tempo_bar.position * -1);
  tempo_lab.Caption := 'Tempo = ' + IntToStr(tempo_bar.position * -1) + '%';

  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmReverse.vol_barChange(Sender: TObject);
begin
  BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, vol_bar.position / 100);
  vol_bar.Hint := IntToStr(vol_bar.position) + '%';
end;

// show the approximate position in MM:SS format
procedure TfrmReverse.UpdatePositionLabel();
var totalsec, posec: FLOAT;
begin
  if(BASS_FX_TempoGetRateRatio(chan)>0) then begin
    totalsec := pos_bar.max / BASS_FX_TempoGetRateRatio(chan);
    posec := pos_bar.position / BASS_FX_TempoGetRateRatio(chan);

    pos_lab.Caption := 'Playing position: ' +
          format('%2.2d:%2.2d', [trunc(posec) div 60, trunc(posec) mod 60]) + ' / ' +
          format('%2.2d:%2.2d', [trunc(totalsec) div 60, trunc(totalsec) mod 60]);
  end;
end;

procedure TfrmReverse.pos_barScroll(Sender: TObject; ScrollCode: TScrollCode; var ScrollPos: Integer);
begin
  If (BASS_ChannelIsActive(chan) = 0) then exit;
  if ScrollCode = scEndScroll then begin
    BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, pos_bar.position), BASS_POS_BYTE);
  end;
  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmReverse.pos_barChange(Sender: TObject);
begin
  If (BASS_ChannelIsActive(chan) = 0) then exit;
  // update the approximate time in seconds view
  UpdatePositionLabel;
end;

procedure TfrmReverse.btnDirectionClick(Sender: TObject);
var
  srcChan: DWORD;
  dir: FLOAT;
begin
  srcChan := BASS_FX_TempoGetSource(chan);
  BASS_ChannelGetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, dir);

  If (dir < 0) then begin
    BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_FORWARD);
    btnDirection.Caption := 'Playing Direction - Forward'
  end else begin
    BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_REVERSE);
    btnDirection.Caption := 'Playing Direction - Reverse';
  end;
end;

end.
