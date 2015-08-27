VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmDSP 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "BASS_FX dsp fx in action"
   ClientHeight    =   5505
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   2895
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   367
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   193
   StartUpPosition =   2  'CenterScreen
   Begin MSComDlg.CommonDialog CMD 
      Left            =   2400
      Top             =   0
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton cmdOpenFP 
      Caption         =   "click here to open a file && play it..."
      Height          =   495
      Left            =   0
      Style           =   1  'Graphical
      TabIndex        =   4
      Top             =   0
      Width           =   2895
   End
   Begin VB.Frame frameDSP 
      Height          =   5085
      Left            =   0
      TabIndex        =   0
      Top             =   420
      Width           =   2895
      Begin MSComctlLib.Slider sldEQ 
         Height          =   1095
         Index           =   0
         Left            =   120
         TabIndex        =   21
         Top             =   480
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   1931
         _Version        =   393216
         Orientation     =   1
         Min             =   -10
         TickStyle       =   2
         TickFrequency   =   0
      End
      Begin VB.CheckBox chkEqualizer 
         Caption         =   "Peaking Equalizer"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   177
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   120
         TabIndex        =   12
         Top             =   120
         Width           =   2655
      End
      Begin VB.CheckBox chkPhaser 
         Caption         =   "Phaser"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   177
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   120
         TabIndex        =   11
         Top             =   1920
         Width           =   2655
      End
      Begin MSComctlLib.Slider sldPhaser 
         Height          =   315
         Index           =   1
         Left            =   1080
         TabIndex        =   9
         Top             =   2640
         Width           =   1695
         _ExtentX        =   2990
         _ExtentY        =   556
         _Version        =   393216
         Min             =   -2000
         Max             =   2000
         SelStart        =   -999
         TickStyle       =   3
         Value           =   -999
      End
      Begin MSComctlLib.Slider sldPhaser 
         Height          =   315
         Index           =   2
         Left            =   1080
         TabIndex        =   10
         Top             =   3000
         Width           =   1695
         _ExtentX        =   2990
         _ExtentY        =   556
         _Version        =   393216
         Min             =   -1000
         Max             =   1000
         SelStart        =   -60
         TickStyle       =   3
         Value           =   -60
      End
      Begin MSComctlLib.Slider sldPhaser 
         Height          =   315
         Index           =   0
         Left            =   1080
         TabIndex        =   13
         Top             =   2280
         Width           =   1695
         _ExtentX        =   2990
         _ExtentY        =   556
         _Version        =   393216
         Min             =   -2000
         Max             =   2000
         SelStart        =   999
         TickStyle       =   3
         Value           =   999
      End
      Begin MSComctlLib.Slider sldDXRate 
         Height          =   315
         Left            =   120
         TabIndex        =   14
         Top             =   4680
         Width           =   2655
         _ExtentX        =   4683
         _ExtentY        =   556
         _Version        =   393216
         Min             =   30870
         Max             =   57330
         SelStart        =   44100
         TickStyle       =   3
         Value           =   44100
      End
      Begin MSComctlLib.Slider sldPhaser 
         Height          =   315
         Index           =   3
         Left            =   1080
         TabIndex        =   15
         Top             =   3360
         Width           =   1695
         _ExtentX        =   2990
         _ExtentY        =   556
         _Version        =   393216
         Max             =   100
         TickStyle       =   3
         Value           =   2
      End
      Begin MSComctlLib.Slider sldPhaser 
         Height          =   315
         Index           =   4
         Left            =   1080
         TabIndex        =   17
         Top             =   3720
         Width           =   1695
         _ExtentX        =   2990
         _ExtentY        =   556
         _Version        =   393216
         Max             =   100
         SelStart        =   60
         TickStyle       =   3
         Value           =   60
      End
      Begin MSComctlLib.Slider sldPhaser 
         Height          =   315
         Index           =   5
         Left            =   1080
         TabIndex        =   19
         Top             =   4080
         Width           =   1695
         _ExtentX        =   2990
         _ExtentY        =   556
         _Version        =   393216
         Max             =   10000
         SelStart        =   1000
         TickStyle       =   3
         Value           =   1000
      End
      Begin MSComctlLib.Slider sldEQ 
         Height          =   1095
         Index           =   1
         Left            =   1080
         TabIndex        =   22
         Top             =   480
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   1931
         _Version        =   393216
         Orientation     =   1
         Min             =   -10
         TickStyle       =   2
         TickFrequency   =   0
      End
      Begin MSComctlLib.Slider sldEQ 
         Height          =   1095
         Index           =   2
         Left            =   2040
         TabIndex        =   23
         Top             =   480
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   1931
         _Version        =   393216
         Orientation     =   1
         Min             =   -10
         TickStyle       =   2
         TickFrequency   =   0
      End
      Begin VB.Label lblPhaser 
         Alignment       =   2  'Center
         Caption         =   "Freq"
         Height          =   195
         Index           =   5
         Left            =   120
         TabIndex        =   20
         Top             =   4080
         Width           =   930
      End
      Begin VB.Label lblPhaser 
         Alignment       =   2  'Center
         Caption         =   "Range"
         Height          =   195
         Index           =   4
         Left            =   120
         TabIndex        =   18
         Top             =   3720
         Width           =   930
      End
      Begin VB.Label lblPhaser 
         Alignment       =   2  'Center
         Caption         =   "Rate"
         Height          =   195
         Index           =   1
         Left            =   120
         TabIndex        =   16
         Top             =   3360
         Width           =   930
      End
      Begin VB.Label lblPhaser 
         Alignment       =   2  'Center
         Caption         =   "Feedback"
         Height          =   195
         Index           =   3
         Left            =   120
         TabIndex        =   8
         Top             =   3000
         Width           =   930
      End
      Begin VB.Label lblPhaser 
         Alignment       =   2  'Center
         Caption         =   "WetMix"
         Height          =   195
         Index           =   2
         Left            =   120
         TabIndex        =   7
         Top             =   2640
         Width           =   930
      End
      Begin VB.Label lblPhaser 
         Alignment       =   2  'Center
         Caption         =   "DryMix"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   6
         Top             =   2280
         Width           =   930
      End
      Begin VB.Label lblDX 
         Alignment       =   2  'Center
         Caption         =   "DirectX Samplerate = 44100Hz"
         Height          =   195
         Left            =   120
         TabIndex        =   5
         ToolTipText     =   "Click to set DX Samplerate to 44100Hz"
         Top             =   4440
         Width           =   2655
      End
      Begin VB.Label lblEQ 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "125 hz"
         Height          =   195
         Index           =   0
         Left            =   240
         TabIndex        =   3
         Top             =   1560
         Width           =   480
      End
      Begin VB.Label lblEQ 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "1 khz"
         Height          =   195
         Index           =   1
         Left            =   1200
         TabIndex        =   2
         Top             =   1560
         Width           =   480
      End
      Begin VB.Label lblEQ 
         Alignment       =   2  'Center
         Caption         =   "8 khz"
         Height          =   195
         Index           =   2
         Left            =   2160
         TabIndex        =   1
         Top             =   1560
         Width           =   480
      End
   End
End
Attribute VB_Name = "frmDSP"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'============================================================================
' frmDSP.frm - Copyright (c) 2002-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
'                                                    [http://www.jobnik.org]
'                                                    [   bass_fx@jobnik.org]
'
' BASS_FX dsp fx in action
' * Requires: BASS 2.4 (available @ www.un4seen.com)
'============================================================================

Option Explicit

Dim floatable As Long              ' floating-point channel support?
Dim chan As Long                   ' a handle
Dim fxEQ As Long, fxPhaser As Long ' dsp fx handles
Dim freq As Single                 ' sample rate
Dim oldfreq As Single              ' old sample rate

' display error dialogs
Sub Error_(ByVal es As String)
    Call MsgBox(es & vbCrLf & vbCrLf & "(error code: " & BASS_ErrorGetCode & ")", vbExclamation, "Error")
End Sub

Private Sub Form_Initialize()
    ' change and set the current path, to prevent from VB not finding BASS.DLL
    ChDrive App.Path
    ChDir App.Path
    
    ' check the correct BASS was loaded
    If (HiWord(BASS_GetVersion) <> BASSVERSION) Then
        Call MsgBox("An incorrect version of BASS.DLL was loaded (2.4 is required)", vbCritical)
        End
    End If

    ' check the correct BASS_FX was loaded
    If (HiWord(BASS_FX_GetVersion) <> BASSVERSION) Then
        Call MsgBox("An incorrect version of BASS_FX.DLL was loaded (2.4 is required)", vbCritical)
        End
    End If

    ' enable floating-point DSP
    Call BASS_SetConfig(BASS_CONFIG_FLOATDSP, BASSTRUE)

    ' setup output - default device, 44100hz, stereo, 16 bits
    If (BASS_Init(-1, 44100, 0, Me.hWnd, 0) = 0) Then
        Call Error_("Can't initialize device")
        End
    End If

    ' check for floating-point capability
    floatable = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, 0, 0)
    If (floatable) Then
        Call BASS_StreamFree(floatable)  ' woohoo!
        floatable = BASS_SAMPLE_FLOAT
    End If

    freq = 44100
    fxEQ = 0
    fxPhaser = 0
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call BASS_Free      'free BASS & BASS_FX
End Sub

Private Sub cmdOpenFP_Click()
    On Local Error Resume Next    ' in case Cancel was pressed

    With CMD
        .CancelError = True
        .flags = cdlOFNExplorer Or cdlOFNFileMustExist
        .Filter = "Playable files|*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif|All files|*.*"
        .ShowOpen
    End With

    ' if cancel was pressed, exit the procedure
    If Err.Number = 32755 Then Exit Sub

    ' change path to default to avoid error, that BASS_FX.DLL isn't found in IDE mode.
    ChDrive App.Path
    ChDir App.Path

    ' free previous streams/music/dsps handles
    Call BASS_StreamFree(chan)
    Call BASS_MusicFree(chan)

    ' check for a stream
    chan = BASS_StreamCreateFile(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_SAMPLE_LOOP Or floatable)

    ' check for MOD
    If chan = 0 Then chan = BASS_MusicLoad(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_MUSIC_LOOP Or BASS_MUSIC_RAMP Or floatable, 0)

    If chan = 0 Then
        cmdOpenFP.Caption = "click here to open a file && play it..."
        Call Error_("Selected file couldn't be loaded!")
        Exit Sub
    End If

    ' update the Button to show the loaded file name
    cmdOpenFP.Caption = GetFileName(CMD.filename)

    ' set dsp effects
    Call chkEqualizer_Click
    Call chkPhaser_Click

    ' set the dx sample rate
    Call BASS_ChannelGetAttribute(chan, BASS_ATTRIB_FREQ, freq)
    oldfreq = freq

    With sldDXRate
        .min = 0    'to avoit min > max
        .max = 1    'to avoid max < min
        .max = freq * 1.3
        .min = freq * 0.7
        .LargeChange = freq * 0.01      ' by 1%
    End With
    
    ' update lblDX ToolTipText
    lblDX.ToolTipText = "Click to set DX Samplerate to " & freq & "Hz"

    ' plat it!
    Call BASS_ChannelPlay(chan, BASSFALSE)
    
    ' update the dx sample rate to normal
    Call lblDX_Click
End Sub

Private Sub chkPhaser_Click()
    If chkPhaser.value = vbChecked Then
        fxPhaser = BASS_ChannelSetFX(chan, BASS_FX_BFX_PHASER, 1)
        Dim i As Integer
        For i = 0 To sldPhaser.Count
            Call sldPhaser_Scroll(i)
        Next i
    Else
        Call BASS_ChannelRemoveFX(chan, fxPhaser)
    End If
End Sub

Private Sub chkEqualizer_Click()
  If chkEqualizer.value = vbChecked Then

    Dim eq As BASS_BFX_PEAKEQ

    ' set peaking equalizer effect with no bands
    fxEQ = BASS_ChannelSetFX(chan, BASS_FX_BFX_PEAKEQ, 0)

    eq.fBandwidth = 2.5
    eq.fQ = 0#
    eq.fGain = 0#
    eq.lChannel = BASS_BFX_CHANALL

    ' create 1st band for bass
    eq.lBand = 0
    eq.fCenter = 125
    Call BASS_FXSetParameters(fxEQ, eq)
    
    ' create 2nd band for mid
    eq.lBand = 1
    eq.fCenter = 1000
    Call BASS_FXSetParameters(fxEQ, eq)
    
    ' create 3rd band for treble
    eq.lBand = 2
    eq.fCenter = 8000
    Call BASS_FXSetParameters(fxEQ, eq)

    ' update dsp eq
    Call sldEQ_Change(0)
    Call sldEQ_Change(1)
    Call sldEQ_Change(2)
  Else
    Call BASS_ChannelRemoveFX(chan, fxEQ)
  End If
End Sub

Private Sub sldDXRate_Scroll()
    If BASS_ChannelIsActive(chan) = 0 Then Exit Sub

    Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_FREQ, sldDXRate.value)

    lblDX.Caption = "DirectX Samplerate = " & sldDXRate.value & "Hz"

    ' update all bands fCenters after changing samplerate
    Dim eq As BASS_BFX_PEAKEQ, i As Integer
    For i = 0 To 2
        eq.lBand = i
        Call BASS_FXGetParameters(fxEQ, eq)
            eq.fCenter = eq.fCenter * sldDXRate.value / oldfreq
        Call BASS_FXSetParameters(fxEQ, eq)
    Next i
    oldfreq = sldDXRate.value
End Sub

Private Sub sldPhaser_Scroll(index As Integer)
    Dim phs As BASS_BFX_PHASER

    Call BASS_FXGetParameters(fxPhaser, phs)
        With sldPhaser(index)
            Select Case index
                Case 0: phs.fDryMix = .value / 1000
                Case 1: phs.fWetMix = .value / 1000
                Case 2: phs.fFeedback = .value / 1000
                Case 3: phs.fRate = .value / 10
                Case 4: phs.fRange = .value / 10
                Case 5: phs.fFreq = .value / 10
            End Select
        End With
    Call BASS_FXSetParameters(fxPhaser, phs)
End Sub

Private Sub sldEQ_Change(index As Integer)
    Dim eq As BASS_BFX_PEAKEQ

    eq.lBand = index    ' Band values you would like to get

    Call BASS_FXGetParameters(fxEQ, eq)
        eq.fGain = sldEQ(index).value * -1
    Call BASS_FXSetParameters(fxEQ, eq)
End Sub

Private Sub sldEQ_Scroll(index As Integer)
    Call sldEQ_Change(index)
    sldEQ(index).Text = sldEQ(index).value * -1
End Sub

Private Sub lblDX_Click()
    sldDXRate.value = freq
    Call sldDXRate_Scroll
End Sub

'--------------------
' useful function :)
'--------------------

'get file name from file path
Public Function GetFileName(ByVal fp As String) As String
    GetFileName = Mid(fp, InStrRev(fp, "\") + 1)
End Function
