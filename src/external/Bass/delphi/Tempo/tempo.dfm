object frmTempo: TfrmTempo
  Left = 192
  Top = 114
  BorderStyle = bsDialog
  Caption = 'BASS_FX tempo / rate / pitch with dsp fx'
  ClientHeight = 249
  ClientWidth = 361
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
    Width = 361
    Height = 220
    TabOrder = 0
    object lbl125Hz: TLabel
      Left = 16
      Top = 155
      Width = 49
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '125 hz'
    end
    object lbl1KHz: TLabel
      Left = 80
      Top = 155
      Width = 49
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '1 khz'
    end
    object lbl8KHz: TLabel
      Left = 144
      Top = 155
      Width = 49
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '8 khz'
    end
    object lblTempo: TLabel
      Left = 200
      Top = 56
      Width = 153
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
      Left = 200
      Top = 96
      Width = 153
      Height = 13
      Hint = 'click to set Samplerate to 44100Hz'
      Alignment = taCenter
      AutoSize = False
      Caption = 'Samplerate = 44100Hz'
      ParentShowHint = False
      ShowHint = True
      OnClick = lblSamplerateClick
    end
    object lblPitch: TLabel
      Left = 200
      Top = 136
      Width = 153
      Height = 13
      Hint = 'click to set Pitch Scaling to 0 semitones'
      Alignment = taCenter
      AutoSize = False
      Caption = 'Pitch Scaling = 0 semitones'
      ParentShowHint = False
      ShowHint = True
      OnClick = lblPitchClick
    end
    object lblPosition: TLabel
      Left = 8
      Top = 176
      Width = 345
      Height = 13
      Hint = 'Click to set DX Samplerate to 44100Hz'
      Alignment = taCenter
      AutoSize = False
      Caption = 'Playing position: 00:00 / 00:00'
      ParentShowHint = False
      ShowHint = True
    end
    object lblVolume: TLabel
      Left = 8
      Top = 16
      Width = 345
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Volume'
    end
    object lblDSPEQ: TLabel
      Left = 16
      Top = 56
      Width = 161
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = '-= DSP Peaking Equalizer =-'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object vsEQBASS: TScrollBar
      Left = 16
      Top = 72
      Width = 49
      Height = 81
      Kind = sbVertical
      Max = 19
      PageSize = 0
      Position = 10
      TabOrder = 0
      OnChange = vsEQBASSChange
    end
    object vsEQMID: TScrollBar
      Left = 80
      Top = 72
      Width = 49
      Height = 81
      Kind = sbVertical
      Max = 19
      PageSize = 0
      Position = 10
      TabOrder = 1
      OnChange = vsEQMIDChange
    end
    object vsEQTREBLE: TScrollBar
      Left = 144
      Top = 72
      Width = 49
      Height = 81
      Kind = sbVertical
      Max = 19
      PageSize = 0
      Position = 10
      TabOrder = 2
      OnChange = vsEQTREBLEChange
    end
    object tbTempo: TTrackBar
      Left = 200
      Top = 72
      Width = 153
      Height = 17
      Max = 30
      Min = -30
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 0
      SelEnd = 0
      SelStart = 0
      TabOrder = 3
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbTempoChange
    end
    object tbPitch: TTrackBar
      Left = 200
      Top = 152
      Width = 153
      Height = 17
      Max = 30
      Min = -30
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 0
      SelEnd = 0
      SelStart = 0
      TabOrder = 4
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbPitchChange
    end
    object tbSamplerate: TTrackBar
      Left = 200
      Top = 112
      Width = 153
      Height = 17
      Max = 57330
      Min = 30870
      Orientation = trHorizontal
      PageSize = 441
      Frequency = 1
      Position = 44100
      SelEnd = 0
      SelStart = 0
      TabOrder = 5
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbSamplerateChange
    end
    object tbVolume: TTrackBar
      Left = 8
      Top = 32
      Width = 345
      Height = 17
      Max = 100
      Orientation = trHorizontal
      PageSize = 1
      Frequency = 1
      Position = 50
      SelEnd = 0
      SelStart = 0
      TabOrder = 6
      ThumbLength = 15
      TickMarks = tmBoth
      TickStyle = tsNone
      OnChange = tbVolumeChange
    end
    object hsPosition: TScrollBar
      Left = 8
      Top = 192
      Width = 345
      Height = 17
      PageSize = 0
      TabOrder = 7
      OnChange = hsPositionChange
      OnScroll = hsPositionScroll
    end
  end
  object btnOpen: TButton
    Left = 0
    Top = 0
    Width = 361
    Height = 33
    Caption = 'click here to open a file && play it'
    TabOrder = 1
    OnClick = btnOpenClick
  end
  object od: TOpenDialog
    Left = 328
  end
end
