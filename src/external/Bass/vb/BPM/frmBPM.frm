VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmBPM 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "BASS_FX bpm with tempo & rate "
   ClientHeight    =   4935
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3975
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4935
   ScaleWidth      =   3975
   StartUpPosition =   2  'CenterScreen
   Begin MSComDlg.CommonDialog CMD 
      Left            =   3480
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
      TabIndex        =   0
      Top             =   0
      Width           =   3975
   End
   Begin VB.Frame frameBPM 
      Height          =   4500
      Left            =   0
      TabIndex        =   1
      Top             =   440
      Width           =   3975
      Begin VB.CheckBox chkBeat 
         Caption         =   "Enable / Disable showing Beat pos"
         Height          =   255
         Left            =   240
         TabIndex        =   16
         Top             =   2520
         Width           =   2895
      End
      Begin VB.TextBox txtPeriod 
         Alignment       =   2  'Center
         Height          =   285
         Left            =   2520
         MaxLength       =   5
         TabIndex        =   10
         Text            =   "10"
         Top             =   2850
         Width           =   615
      End
      Begin VB.CheckBox chkBPMCallback 
         Caption         =   "BPM by period of seconds:"
         Height          =   255
         Left            =   240
         TabIndex        =   11
         Top             =   2880
         Width           =   2295
      End
      Begin MSComctlLib.Slider sldPosition 
         Height          =   330
         Left            =   120
         TabIndex        =   5
         Top             =   4080
         Width           =   3735
         _ExtentX        =   6588
         _ExtentY        =   582
         _Version        =   393216
         Max             =   100
         TickStyle       =   3
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldTempo 
         Height          =   585
         Index           =   0
         Left            =   120
         TabIndex        =   6
         Top             =   480
         Width           =   3015
         _ExtentX        =   5318
         _ExtentY        =   1032
         _Version        =   393216
         LargeChange     =   1
         Min             =   -30
         Max             =   30
         TickStyle       =   2
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldTempo 
         Height          =   420
         Index           =   1
         Left            =   120
         TabIndex        =   7
         Top             =   1560
         Width           =   3015
         _ExtentX        =   5318
         _ExtentY        =   741
         _Version        =   393216
         LargeChange     =   1000
         Min             =   30870
         Max             =   57330
         SelStart        =   44100
         TickStyle       =   3
         TickFrequency   =   5
         Value           =   44100
      End
      Begin MSComctlLib.ProgressBar pbProcess 
         Height          =   255
         Left            =   240
         TabIndex        =   12
         Top             =   3480
         Width           =   2895
         _ExtentX        =   5106
         _ExtentY        =   450
         _Version        =   393216
         Appearance      =   1
      End
      Begin MSComctlLib.Slider sldVolume 
         Height          =   3375
         Left            =   3240
         TabIndex        =   4
         Top             =   480
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   5953
         _Version        =   393216
         Orientation     =   1
         Max             =   100
         SelStart        =   50
         TickStyle       =   2
         TickFrequency   =   5
         Value           =   50
      End
      Begin VB.Label lblBeat 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "Beat pos: 0.00"
         BeginProperty Font 
            Name            =   "Tahoma"
            Size            =   9.75
            Charset         =   177
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Left            =   1560
         TabIndex        =   15
         Top             =   2040
         Width           =   1425
      End
      Begin VB.Label lblVolPitch 
         Alignment       =   2  'Center
         Caption         =   "Volume"
         Height          =   195
         Left            =   3240
         TabIndex        =   3
         Top             =   240
         Width           =   615
      End
      Begin VB.Label lblProcess 
         Alignment       =   2  'Center
         Caption         =   "Detection process of a decoding BPM:"
         Height          =   195
         Left            =   240
         TabIndex        =   14
         Top             =   3240
         Width           =   2895
      End
      Begin VB.Label lblBPM 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "BPM: 0.00"
         BeginProperty Font 
            Name            =   "Tahoma"
            Size            =   9.75
            Charset         =   177
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   240
         Left            =   240
         TabIndex        =   13
         Top             =   2040
         Width           =   975
      End
      Begin VB.Label lblTempo 
         Alignment       =   2  'Center
         Caption         =   "Tempo = 0%"
         Height          =   195
         Left            =   120
         TabIndex        =   8
         ToolTipText     =   "click to set Tempo to 0%"
         Top             =   240
         Width           =   3015
      End
      Begin VB.Label lblRate 
         Alignment       =   2  'Center
         Caption         =   "Samplerate = 44100Hz"
         Height          =   195
         Left            =   120
         TabIndex        =   9
         ToolTipText     =   "click to set Samplerate to 44100Hz"
         Top             =   1200
         Width           =   3015
      End
      Begin VB.Label lblPos 
         Alignment       =   2  'Center
         Caption         =   "Playing position: 00:00 / 00:00"
         Height          =   195
         Left            =   120
         TabIndex        =   2
         Top             =   3840
         Width           =   3735
      End
   End
End
Attribute VB_Name = "frmBPM"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'============================================================================
' frmBPM.frm - Copyright (c) 2003-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
'                                                    [http://www.jobnik.org]
'                                                    [   bass_fx@jobnik.org]
' Other source: modBPM.bas
'
' BASS_FX bpm with tempo & samplerate changers
' * Requires: BASS 2.4 (available @ www.un4seen.com)
'============================================================================

Option Explicit

Dim info As BASS_CHANNELINFO

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

    ' setup output - default device, 44100hz, stereo, 16 bits
    If (BASS_Init(-1, 44100, 0, Me.hWnd, 0) = 0) Then
        Call Error_("Can't initialize device")
        End
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call BASS_Free      ' free BASS & BASS_FX
End Sub

Private Sub cmdOpenFP_Click()
    On Local Error Resume Next    ' in case Cancel was pressed

    With CMD
        .CancelError = True
        .flags = cdlOFNExplorer Or cdlOFNFileMustExist Or cdlOFNHideReadOnly
        .Filter = "Playable files|*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif|All files|*.*"
        .ShowOpen
    End With

    ' if cancel was pressed, exit the procedure
    If Err.Number = 32755 Then Exit Sub

    ' change path to default to avoid error, that BASS_FX.DLL isn't found in IDE mode.
    ChDrive App.Path
    ChDir App.Path

    ' free decode bpm & stream
    Call BASS_FX_BPM_Free(hBPM)

    ' free tempo, stream, music & bpm/beat callbacks
    Call BASS_StreamFree(chan)

    ' create decode channel
    chan = BASS_StreamCreateFile(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_STREAM_DECODE)

    ' check for MOD
    If chan = 0 Then chan = BASS_MusicLoad(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_MUSIC_RAMP Or BASS_MUSIC_PRESCAN Or BASS_MUSIC_DECODE, 0)

    If chan = 0 Then
        cmdOpenFP.Caption = "click here to open a file && play it..."
        Call Error_("Selected file couldn't be loaded!")
        Exit Sub
    End If

    ' get channel info
    Call BASS_ChannelGetInfo(chan, info)

    ' set max length to sldPosition scroller
    sldPosition.max = BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE))
    sldPosition.value = 0

    ' create a new stream - decoded & resampled :)
    chan = BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP Or BASS_FX_FREESOURCE)

    If (chan = 0) Then
        cmdOpenFP.Caption = "click here to open a file && play it..."
        Call Error_("Couldn't create a resampled stream!")
        Call BASS_StreamFree(chan)
        Call BASS_MusicFree(chan)
        Exit Sub
    End If

    ' update the button to show the loaded file name
    cmdOpenFP.Caption = GetFileName(CMD.filename)

    ' set volume
    Call sldVolume_Scroll

    ' set rate min/max values according to current frequency
    With sldTempo(1)
        .min = 0    ' to avoit min > max
        .max = 1    ' to avoid max < min
        .max = info.freq * 1.3
        .min = info.freq * 0.7
        .LargeChange = info.freq * 0.01      ' by 1%
    End With

    ' update the Rate label ToolTipText
    lblRate.ToolTipText = "click to set Samplerate to " & info.freq & "Hz"

    ' set tempo/rate to normal
    Call lblTempo_Click
    Call lblRate_Click

    ' update the approximate time in seconds view
    Call UpdatePositionLabel

    ' play new created stream
    Call BASS_ChannelPlay(chan, BASSFALSE)

    ' set the callback bpm and beat
    Call chkBPMCallback_Click
    Call chkBeat_Click

    ' get the bpm of 30 seconds from the start
    Call DecodingBPM(True, 0, 30)
End Sub

Private Sub lblTempo_Click()
    sldTempo(0).value = 0
    Call sldTempo_Scroll(0)
End Sub

Private Sub lblRate_Click()
    sldTempo(1).value = IIf(info.freq, info.freq, 44100)
    Call sldTempo_Scroll(1)
End Sub

Private Sub sldPosition_Change()
    If ((BASS_ChannelIsActive(chan) = 0)) Then Exit Sub

    Call BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, sldPosition.value), BASS_POS_BYTE)

    ' get the bpm of last txtPeriod seconds
    Call DecodingBPM(False, sldPosition.value - Val(txtPeriod.Text), sldPosition.value)
End Sub

Private Sub sldPosition_Scroll()
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub
    Call UpdatePositionLabel
End Sub

Private Sub sldTempo_Scroll(index As Integer)
    Select Case (index)
        Case 0:
            ' set new tempo
            Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, sldTempo(0).value)

            lblTempo.Caption = "Tempo = " & sldTempo(0).value & "%"
            sldTempo(0).Text = sldTempo(0).value & "%"
        Case 1:
            ' set new sample rate
            Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_FREQ, sldTempo(1).value)

            lblRate.Caption = "Samplerate = " & sldTempo(1).value & "Hz"
            sldTempo(1).Text = sldTempo(1).value & "Hz"
    End Select

    ' update the bpm view
    lblBPM.Caption = "BPM: " & Format(GetNewBPM(IIf(chkBPMCallback.value = vbChecked, chan, hBPM)), "0.00")

    ' update the approximate time in seconds view
    Call UpdatePositionLabel
End Sub

Private Sub sldVolume_Scroll()
    Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, (100 - sldVolume.value) / 100)
    sldVolume.Text = 100 - sldVolume.value & "%"
End Sub

Private Sub chkBPMCallback_Click()
    If (chkBPMCallback.value = vbChecked) Then
        Call BASS_FX_BPM_CallbackSet(chan, AddressOf GetBPM_Callback, Val(txtPeriod.Text), 0, BASS_FX_BPM_MULT2, 0)
    Else
        Call BASS_FX_BPM_Free(chan)
    End If
End Sub

Private Sub chkBeat_Click()
    If (chkBeat.value = vbChecked) Then
        Call BASS_FX_BPM_BeatCallbackSet(chan, AddressOf GetBeatPos_Callback, 0)
    Else
        Call BASS_FX_BPM_BeatFree(chan)
    End If
End Sub

'--------------------
' useful function :)
'--------------------

' get file name from file path
Public Function GetFileName(ByVal fp As String) As String
    GetFileName = Mid(fp, InStrRev(fp, "\") + 1)
End Function
