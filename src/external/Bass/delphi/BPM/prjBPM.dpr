program prjBPM;

uses
  Forms,
  bpm in 'bpm.pas' {frmBPM},
  BASS_FX in '..\bass_fx.pas',
  BASS in '..\bass.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TfrmBPM, frmBPM);
  Application.Run;
end.
