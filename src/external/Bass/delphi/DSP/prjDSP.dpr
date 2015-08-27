program prjDSP;

uses
  Forms,
  dsp in 'dsp.pas' {frmDSP},
  BASS_FX in '..\bass_fx.pas',
  BASS in '..\bass.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TfrmDSP, frmDSP);
  Application.Run;
end.
