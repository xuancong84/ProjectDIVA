program prjRev;

uses
  Forms,
  reverse in 'reverse.pas' {frmReverse},
  BASS_FX in '..\bass_fx.pas',
  BASS in '..\bass.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TfrmReverse, frmReverse);
  Application.Run;
end.
