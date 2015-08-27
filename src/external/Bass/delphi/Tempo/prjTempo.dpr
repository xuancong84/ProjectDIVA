program prjTempo;

uses
  Forms,
  tempo in 'tempo.pas' {frmTempo},
  BASS_FX in '..\bass_fx.pas',
  BASS in '..\bass.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TfrmTempo, frmTempo);
  Application.Run;
end.
