object frmDSP: TfrmDSP
  Left = 195
  Top = 117
  BorderStyle = bsDialog
  Caption = 'BASS_FX dsp fx in action'
  ClientHeight = 366
  ClientWidth = 193
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox: TGroupBox
    Left = 0
    Top = 29
    Width = 193
    Height = 337
    TabOrder = 1
    object lbl1KHz: TLabel
      Left = 72
      Top = 104
      Width = 49
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '1 khz'
    end
    object lbl8KHz: TLabel
      Left = 136
      Top = 104
      Width = 49
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '8 khz'
    end
    object lblDryMix: TLabel
      Left = 8
      Top = 144
      Width = 57
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'DryMix'
    end
    object lblWetMix: TLabel
      Left = 8
      Top = 168
      Width = 57
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'WetMix'
    end
    object lblFeedback: TLabel
      Left = 8
      Top = 192
      Width = 57
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Feedback'
    end
    object lblDX: TLabel
      Left = 8
      Top = 296
      Width = 177
      Height = 13
      Hint = 'Click to set DX Samplerate to 44100Hz'
      Alignment = taCenter
      AutoSize = False
      Caption = 'DirectX Samplerate = 44100Hz'
      ParentShowHint = False
      ShowHint = True
      OnClick = lblDXClick
    end
    object lbl125Hz: TLabel
      Left = 8
      Top = 104
      Width = 49
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '125 hz'
    end
    object lblRate: TLabel
      Left = 8
      Top = 216
      Width = 57
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Rate'
    end
    object lblRange: TLabel
      Left = 8
      Top = 240
      Width = 57
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Range'
    end
    object lblFreq: TLabel
      Left = 8
      Top = 264
      Width = 57
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Freq'
    end
    object vsEQBASS: TScrollBar
      Left = 16
      Top = 32
      Width = 41
      Height = 65
      Kind = sbVertical
      Max = 19
      PageSize = 0
      Position = 10
      TabOrder = 1
      OnChange = vsEQBASSChange
    end
    object vsEQMID: TScrollBar
      Left = 72
      Top = 32
      Width = 49
      Height = 65
      Kind = sbVertical
      Max = 19
      PageSize = 0
      Position = 10
      TabOrder = 2
      OnChange = vsEQMIDChange
    end
    object vsEQTREBLE: TScrollBar
      Left = 136
      Top = 32
      Width = 41
      Height = 65
      Kind = sbVertical
      Max = 19
      PageSize = 0
      Position = 10
      TabOrder = 3
      OnChange = vsEQTREBLEChange
    end
    object chkPhaser: TCheckBox
      Left = 8
      Top = 120
      Width = 177
      Height = 17
      Caption = 'Phaser'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 4
      OnClick = chkPhaserClick
    end
    object tbDryMix: TTrackBar
      Left = 64
      Top = 144
      Width = 121
      Height = 17
      Max = 2000
      Min = -2000
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 999
      SelEnd = 0
      SelStart = 0
      TabOrder = 5
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbDryMixChange
    end
    object tbWetMix: TTrackBar
      Left = 64
      Top = 168
      Width = 121
      Height = 17
      Max = 2000
      Min = -2000
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = -999
      SelEnd = 0
      SelStart = 0
      TabOrder = 6
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbWetMixChange
    end
    object tbFeedback: TTrackBar
      Left = 64
      Top = 192
      Width = 121
      Height = 17
      Max = 1000
      Min = -1000
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = -60
      SelEnd = 0
      SelStart = 0
      TabOrder = 7
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbFeedbackChange
    end
    object tbDXRate: TTrackBar
      Left = 8
      Top = 312
      Width = 177
      Height = 17
      Max = 57330
      Min = 30870
      Orientation = trHorizontal
      PageSize = 441
      Frequency = 1
      Position = 44100
      SelEnd = 0
      SelStart = 0
      TabOrder = 8
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbDXRateChange
    end
    object chkEqualizer: TCheckBox
      Left = 8
      Top = 8
      Width = 177
      Height = 17
      Caption = 'Peaking Equalizer'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 0
      OnClick = chkEqualizerClick
    end
    object tbRate: TTrackBar
      Left = 64
      Top = 216
      Width = 121
      Height = 17
      Max = 100
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 2
      SelEnd = 0
      SelStart = 0
      TabOrder = 9
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbFeedbackChange
    end
    object tbRange: TTrackBar
      Left = 64
      Top = 240
      Width = 121
      Height = 17
      Max = 100
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 60
      SelEnd = 0
      SelStart = 0
      TabOrder = 10
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbFeedbackChange
    end
    object tbFreq: TTrackBar
      Left = 64
      Top = 264
      Width = 121
      Height = 17
      Max = 10000
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 1000
      SelEnd = 0
      SelStart = 0
      TabOrder = 11
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbFeedbackChange
    end
  end
  object btnOpen: TButton
    Left = 0
    Top = 0
    Width = 193
    Height = 33
    Caption = 'click here to open a file && play it'
    TabOrder = 0
    OnClick = btnOpenClick
  end
  object od: TOpenDialog
    Left = 160
  end
end
