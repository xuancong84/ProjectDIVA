VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmReverse 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "BASS_FX reverse with tempo & dx8 fx"
   ClientHeight    =   3915
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   2535
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   261
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   169
   StartUpPosition =   2  'CenterScreen
   Begin MSComDlg.CommonDialog CMD 
      Left            =   2040
      Top             =   0
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton cmdOpenFP 
      Caption         =   "click here to open a file && play it"
      Height          =   495
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   2535
   End
   Begin VB.Frame frameReverse 
      Height          =   3495
      Left            =   0
      TabIndex        =   5
      Top             =   420
      Width           =   2535
      Begin VB.VScrollBar vsReverb 
         Height          =   975
         LargeChange     =   5
         Left            =   120
         Max             =   20
         TabIndex        =   3
         Top             =   1080
         Value           =   20
         Width           =   1125
      End
      Begin MSComctlLib.Slider sldVolume 
         Height          =   300
         Left            =   120
         TabIndex        =   1
         Top             =   480
         Width           =   1095
         _ExtentX        =   1931
         _ExtentY        =   529
         _Version        =   393216
         Max             =   100
         SelStart        =   50
         TickStyle       =   3
         TickFrequency   =   5
         Value           =   50
      End
      Begin MSComctlLib.Slider sldPosition 
         Height          =   330
         Left            =   120
         TabIndex        =   4
         Top             =   2400
         Width           =   2295
         _ExtentX        =   4048
         _ExtentY        =   582
         _Version        =   393216
         LargeChange     =   1
         Max             =   100
         SelStart        =   100
         TickStyle       =   3
         TickFrequency   =   5
         Value           =   100
      End
      Begin MSComctlLib.Slider sldTempo 
         Height          =   1575
         Left            =   1560
         TabIndex        =   2
         Top             =   480
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   2778
         _Version        =   393216
         Orientation     =   1
         LargeChange     =   1
         Min             =   -30
         Max             =   30
         TickStyle       =   2
         TickFrequency   =   10
      End
      Begin VB.CommandButton cmdDirection 
         Caption         =   "Playing Direction - Reverse"
         Height          =   495
         Left            =   120
         TabIndex        =   10
         Top             =   2880
         Width           =   2295
      End
      Begin VB.Label lblTempo 
         Alignment       =   2  'Center
         Caption         =   "Tempo = 0%"
         Height          =   195
         Left            =   1320
         TabIndex        =   8
         ToolTipText     =   "click to set Tempo to 0%"
         Top             =   240
         Width           =   1095
      End
      Begin VB.Label Label1 
         Alignment       =   2  'Center
         Caption         =   "DX8 Reverb"
         Height          =   195
         Left            =   120
         TabIndex        =   9
         Top             =   840
         Width           =   1125
      End
      Begin VB.Label lblPosition 
         Alignment       =   2  'Center
         Caption         =   "Playing position: 00:00 / 00:00"
         Height          =   195
         Left            =   120
         TabIndex        =   7
         Top             =   2160
         Width           =   2295
      End
      Begin VB.Label lblVolume 
         Alignment       =   2  'Center
         Caption         =   "Volume"
         Height          =   195
         Left            =   120
         TabIndex        =   6
         Top             =   240
         Width           =   1095
      End
   End
End
Attribute VB_Name = "frmReverse"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'================================================================================
' frmReverse.frm - Copyright (c) 2002-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
'                                                        [http://www.jobnik.org]
'                                                        [   bass_fx@jobnik.org]
'
' BASS_FX playing in reverse with tempo & dx8 fx
' * Requires: BASS 2.4 (available @ www.un4seen.com)
'================================================================================

Option Explicit

Dim chan As Long            ' reverse handle
Dim fx As Long              ' dx8 reverb handle

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

    ' check if DX8 features are available
    Dim bi As BASS_INFO
    Call BASS_GetInfo(bi)

    If (bi.dsver < 8) Then
        Call MsgBox("DirectX version 8 is not Installed!!!" & vbCrLf & _
                    "You won't be able to use any DX8 Effects!", vbExclamation, "DX8")
        vsReverb.Enabled = False
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call BASS_Free      ' free BASS & BASS_FX
End Sub

Private Sub cmdOpenFP_Click()
    On Local Error Resume Next    ' in case Cancel was pressed

    CMD.CancelError = True
    CMD.flags = cdlOFNExplorer Or cdlOFNFileMustExist Or cdlOFNHideReadOnly
    CMD.Filter = "Playable files|*.mo3;*.xm;*.mod;*.s3m;*.it;*.mtm;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif|All files|*.*"
    CMD.ShowOpen

    ' if cancel was pressed, exit the procedure
    If Err.Number = 32755 Then Exit Sub

    ' change path to default to avoid error, that BASS_FX.DLL isn't found in IDE mode.
    ChDrive App.Path
    ChDir App.Path

    ' free previous tempo, reverse & reverb handles
    Call BASS_StreamFree(chan)

#If 1 Then 'with FX flag

    chan = BASS_StreamCreateFile(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_STREAM_DECODE Or BASS_STREAM_PRESCAN Or BASS_SAMPLE_FX)
    If (chan = 0) Then chan = BASS_MusicLoad(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_MUSIC_DECODE Or BASS_MUSIC_RAMP Or BASS_MUSIC_PRESCAN Or BASS_SAMPLE_FX, 0)

#Else   'without FX flag

    chan = BASS_StreamCreateFile(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_STREAM_DECODE Or BASS_STREAM_PRESCAN)
    If (chan = 0) Then chan = BASS_MusicLoad(BASSFALSE, StrPtr(CMD.filename), 0, 0, BASS_MUSIC_DECODE Or BASS_MUSIC_RAMP Or BASS_MUSIC_PRESCAN, 0)

#End If

    If chan = 0 Then
        cmdOpenFP.Caption = "click here to open a file && play it..."
        Call Error_("Selected file couldn't be loaded!")
        Exit Sub
    End If

    ' create new stream - decoded & reversed :)
    ' 2 seconds decoding block as a decoding channel
    chan = BASS_FX_ReverseCreate(chan, 2#, BASS_STREAM_DECODE Or BASS_FX_FREESOURCE)
    If (chan = 0) Then
        cmdOpenFP.Caption = "click here to open a file && play it..."
        Call Error_("Couldn't create a reversed stream!")
        Call BASS_StreamFree(chan)
        Exit Sub
    End If
    
    ' create new stream - decoded & resampled :)
    chan = BASS_FX_TempoCreate(chan, BASS_SAMPLE_LOOP Or BASS_FX_FREESOURCE)
    If (chan = 0) Then
        cmdOpenFP.Caption = "click here to open a file && play it..."
        Call Error_("Couldn't create a resampled stream!")
        Call BASS_StreamFree(chan)
        Exit Sub
    End If
    
    ' update the button to show the loaded file name
    cmdOpenFP.Caption = GetFileName(CMD.filename)

    ' set dx8 reverb
    fx = BASS_ChannelSetFX(chan, BASS_FX_DX8_REVERB, 0)
    Call vsReverb_Scroll

    ' set Volume
    Call sldVolume_Scroll

    ' set sldPosition max length
    sldPosition.max = BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetLength(chan, BASS_POS_BYTE))
    sldPosition.value = sldPosition.max

    ' update the approximate time in seconds view
    Call UpdatePositionLabel

    ' play the new stream
    Call BASS_ChannelPlay(chan, BASSFALSE)

    ' set the tempo to optimal
    Call lblTempo_Click
End Sub

Private Sub lblTempo_Click()
    sldTempo.value = 0
    sldTempo_Scroll
End Sub

Private Sub sldPosition_Change()
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub
    Call BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, sldPosition.value), BASS_POS_BYTE)
End Sub

Private Sub sldPosition_Scroll()
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub
    Call UpdatePositionLabel
End Sub

Private Sub sldTempo_Scroll()
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub

    sldTempo.Text = sldTempo.value * -1 & "%"
    lblTempo.Caption = "Tempo = " & sldTempo.value * -1 & "%"
    Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, sldTempo.value * -1)

    ' update the approximate time in seconds view
    Call UpdatePositionLabel
End Sub

Private Sub sldVolume_Scroll()
    Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, sldVolume.value / 100)
    sldVolume.Text = sldVolume.value & "%"
End Sub

Private Sub vsReverb_Change()
    Call vsReverb_Scroll
End Sub

' update dx8 reverb
Private Sub vsReverb_Scroll()
    Dim p As BASS_DX8_REVERB, v As Integer

    v = vsReverb.value
     
    Call BASS_FXGetParameters(fx, p)
        p.fReverbMix = -0.012 * v * v * v
    Call BASS_FXSetParameters(fx, p)
End Sub

Private Sub cmdDirection_Click()
    Dim srcChan As Long, dir As Single

    srcChan = BASS_FX_TempoGetSource(chan)
    Call BASS_ChannelGetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, dir)

    If (dir < 0) Then
        Call BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_FORWARD)
        cmdDirection.Caption = "Playing Direction - Forward"
    Else
        Call BASS_ChannelSetAttribute(srcChan, BASS_ATTRIB_REVERSE_DIR, BASS_FX_RVS_REVERSE)
        cmdDirection.Caption = "Playing Direction - Reverse"
    End If
End Sub

' show the approximate position in MM:SS format
Public Sub UpdatePositionLabel()
    Dim totalsec As Single, posec As Single

    If (BASS_FX_TempoGetRateRatio(chan)) Then
        totalsec = sldPosition.max / BASS_FX_TempoGetRateRatio(chan)
        posec = sldPosition.value / BASS_FX_TempoGetRateRatio(chan)

        sldPosition.Text = Format(Int(posec) \ 60, "00") & ":" & Format(Int(posec) Mod 60, "00") & " / " & _
                            Format(Int(totalsec) \ 60, "00") & ":" & Format(Int(totalsec) Mod 60, "00")
        lblPosition.Caption = "Playing position: " & sldPosition.Text
    End If
End Sub

'--------------------
' useful function :)
'--------------------

' get file name from file path
Public Function GetFileName(ByVal fp As String) As String
    GetFileName = Mid(fp, InStrRev(fp, "\") + 1)
End Function
