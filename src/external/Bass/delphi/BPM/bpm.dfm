object frmBPM: TfrmBPM
  Left = 192
  Top = 114
  BorderStyle = bsDialog
  Caption = 'BASS_FX bpm with tempo & rate'
  ClientHeight = 345
  ClientWidth = 265
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
    Width = 265
    Height = 316
    TabOrder = 0
    object lblTempo: TLabel
      Left = 8
      Top = 16
      Width = 193
      Height = 13
      Hint = 'click to set Tempo to 0%'
      Alignment = taCenter
      AutoSize = False
      Caption = 'Tempo = 0%'
      ParentShowHint = False
      ShowHint = True
      OnClick = lblTempoClick
    end
    object lblSamplerate: TLabel
      Left = 16
      Top = 80
      Width = 193
      Height = 13
      Hint = 'click to set Samplerate to 44100Hz'
      Alignment = taCenter
      AutoSize = False
      Caption = 'Samplerate = 44100Hz'
      ParentShowHint = False
      ShowHint = True
      OnClick = lblSamplerateClick
    end
    object lblPosition: TLabel
      Left = 8
      Top = 272
      Width = 249
      Height = 13
      Hint = 'Click to set DX Samplerate to 44100Hz'
      Alignment = taCenter
      AutoSize = False
      Caption = 'Playing position: 00:00 / 00:00'
      ParentShowHint = False
      ShowHint = True
    end
    object lblVolume: TLabel
      Left = 216
      Top = 16
      Width = 35
      Height = 13
      Alignment = taCenter
      Caption = 'Volume'
    end
    object lblBPM: TLabel
      Left = 8
      Top = 136
      Width = 185
      Height = 16
      AutoSize = False
      Caption = 'BPM: 0.00'
      Font.Charset = HEBREW_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblProcess: TLabel
      Left = 8
      Top = 232
      Width = 193
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Detection process of a decoding BPM:'
    end
    object lblBeat: TLabel
      Left = 8
      Top = 160
      Width = 185
      Height = 16
      AutoSize = False
      Caption = 'Beat pos: 0.00'
      Font.Charset = HEBREW_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object tbVolume: TTrackBar
      Left = 208
      Top = 32
      Width = 49
      Height = 249
      Max = 100
      Orientation = trVertical
      ParentShowHint = False
      PageSize = 5
      Frequency = 5
      Position = 50
      SelEnd = 0
      SelStart = 0
      ShowHint = False
      TabOrder = 2
      ThumbLength = 25
      TickMarks = tmBoth
      TickStyle = tsAuto
      OnChange = tbVolumeChange
    end
    object tbTempo: TTrackBar
      Left = 8
      Top = 32
      Width = 193
      Height = 49
      Max = 30
      Min = -30
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 5
      Position = 0
      SelEnd = 0
      SelStart = 0
      TabOrder = 0
      ThumbLength = 25
      TickMarks = tmBoth
      TickStyle = tsAuto
      OnChange = tbTempoChange
    end
    object tbSamplerate: TTrackBar
      Left = 8
      Top = 104
      Width = 193
      Height = 33
      Max = 57330
      Min = 30870
      Orientation = trHorizontal
      PageSize = 441
      Frequency = 1
      Position = 44100
      SelEnd = 0
      SelStart = 0
      TabOrder = 1
      ThumbLength = 25
      TickMarks = tmBottomRight
      TickStyle = tsNone
      OnChange = tbSamplerateChange
    end
    object hsPosition: TScrollBar
      Left = 8
      Top = 288
      Width = 249
      Height = 17
      PageSize = 0
      TabOrder = 3
      OnChange = hsPositionChange
      OnScroll = hsPositionScroll
    end
    object chkBPMCallback: TCheckBox
      Left = 8
      Top = 208
      Width = 145
      Height = 17
      Caption = 'BPM by period of seconds:'
      TabOrder = 4
      OnClick = chkBPMCallbackClick
    end
    object ebPeriod: TEdit
      Left = 160
      Top = 206
      Width = 41
      Height = 21
      BiDiMode = bdRightToLeftNoAlign
      MaxLength = 5
      ParentBiDiMode = False
      TabOrder = 5
      Text = '10'
    end
    object pbProcess: TProgressBar
      Left = 8
      Top = 248
      Width = 193
      Height = 17
      Min = 0
      Max = 100
      TabOrder = 6
    end
    object chkBeat: TCheckBox
      Left = 8
      Top = 184
      Width = 193
      Height = 17
      Caption = 'Enable / Disable  showing Beat pos'
      TabOrder = 7
      OnClick = chkBeatClick
    end
  end
  object btnOpen: TButton
    Left = 0
    Top = 0
    Width = 265
    Height = 33
    Caption = 'click here to open a file && play it'
    TabOrder = 1
    OnClick = btnOpenClick
  end
  object od: TOpenDialog
    Left = 232
  end
end
