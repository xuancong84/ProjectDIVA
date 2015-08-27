object frmReverse: TfrmReverse
  Left = 192
  Top = 114
  BorderStyle = bsDialog
  Caption = 'BASS_FX reverse with tempo & dx8 fx'
  ClientHeight = 249
  ClientWidth = 169
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
  object GroupBox1: TGroupBox
    Left = 0
    Top = 29
    Width = 169
    Height = 220
    TabOrder = 1
    object tempo_lab: TLabel
      Left = 97
      Top = 14
      Width = 59
      Height = 13
      Caption = 'Tempo = 0%'
      OnClick = tempo_labClick
    end
    object dx_lab: TLabel
      Left = 10
      Top = 53
      Width = 59
      Height = 13
      Caption = 'DX8 Reverb'
    end
    object vol_lab: TLabel
      Left = 21
      Top = 13
      Width = 35
      Height = 13
      Caption = 'Volume'
    end
    object pos_lab: TLabel
      Left = 6
      Top = 140
      Width = 155
      Height = 13
      Alignment = taCenter
      AutoSize = False
      Caption = 'Playing position: 00:00 / 00:00'
    end
    object tempo_bar: TTrackBar
      Left = 102
      Top = 30
      Width = 43
      Height = 107
      Max = 30
      Min = -30
      Orientation = trVertical
      Frequency = 10
      Position = 0
      SelEnd = 0
      SelStart = 0
      TabOrder = 0
      TickMarks = tmBoth
      TickStyle = tsAuto
      OnChange = tempo_barChange
    end
    object vol_bar: TTrackBar
      Left = 3
      Top = 28
      Width = 78
      Height = 17
      Hint = '50'
      Max = 100
      Orientation = trHorizontal
      ParentShowHint = False
      Frequency = 1
      Position = 50
      SelEnd = 0
      SelStart = 0
      ShowHint = True
      TabOrder = 1
      ThumbLength = 15
      TickMarks = tmBottomRight
      TickStyle = tsNone
      OnChange = vol_barChange
    end
    object reverb_bar: TScrollBar
      Left = 8
      Top = 72
      Width = 67
      Height = 57
      Kind = sbVertical
      Max = 20
      PageSize = 0
      Position = 20
      TabOrder = 2
      OnChange = reverb_barChange
    end
    object pos_bar: TScrollBar
      Left = 8
      Top = 160
      Width = 153
      Height = 17
      PageSize = 0
      Position = 100
      TabOrder = 3
      OnChange = pos_barChange
      OnScroll = pos_barScroll
    end
  end
  object open_but: TButton
    Left = 0
    Top = 0
    Width = 169
    Height = 33
    Caption = 'Click here to open a file && play it'
    TabOrder = 0
    OnClick = open_butClick
  end
  object btnDirection: TButton
    Left = 8
    Top = 216
    Width = 153
    Height = 25
    Caption = 'Playing Direction - Reverse'
    TabOrder = 2
    OnClick = btnDirectionClick
  end
  object OpenDialog1: TOpenDialog
    Left = 136
  end
end
