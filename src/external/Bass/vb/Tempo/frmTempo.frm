VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmTempo 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "BASS_FX tempo / rate / pitch with dsp fx"
   ClientHeight    =   3795
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5415
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   253
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   361
   StartUpPosition =   2  'CenterScreen
   Begin MSComDlg.CommonDialog CMD 
      Left            =   4920
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
      Width           =   5415
   End
   Begin VB.Frame frameTempo 
      Height          =   3375
      Left            =   0
      TabIndex        =   1
      Top             =   420
      Width           =   5415
      Begin MSComctlLib.Slider sldEQ 
         Height          =   1215
         Index           =   0
         Left            =   240
         TabIndex        =   16
         Top             =   1080
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   2143
         _Version        =   393216
         Orientation     =   1
         Min             =   -10
         TickStyle       =   2
         TickFrequency   =   0
      End
      Begin MSComctlLib.Slider sldVolume 
         Height          =   300
         Left            =   120
         TabIndex        =   9
         Top             =   480
         Width           =   5175
         _ExtentX        =   9128
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
         TabIndex        =   10
         Top             =   2880
         Width           =   5175
         _ExtentX        =   9128
         _ExtentY        =   582
         _Version        =   393216
         LargeChange     =   1
         Max             =   100
         TickStyle       =   3
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldTempo 
         Height          =   300
         Index           =   0
         Left            =   2760
         TabIndex        =   11
         Top             =   1080
         Width           =   2535
         _ExtentX        =   4471
         _ExtentY        =   529
         _Version        =   393216
         LargeChange     =   1
         Min             =   -30
         Max             =   30
         TickStyle       =   3
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldTempo 
         Height          =   300
         Index           =   1
         Left            =   2760
         TabIndex        =   12
         Top             =   1680
         Width           =   2535
         _ExtentX        =   4471
         _ExtentY        =   529
         _Version        =   393216
         LargeChange     =   441
         SmallChange     =   441
         Min             =   30870
         Max             =   57330
         SelStart        =   44100
         TickStyle       =   3
         TickFrequency   =   5
         Value           =   44100
      End
      Begin MSComctlLib.Slider sldTempo 
         Height          =   300
         Index           =   2
         Left            =   2760
         TabIndex        =   13
         Top             =   2280
         Width           =   2535
         _ExtentX        =   4471
         _ExtentY        =   529
         _Version        =   393216
         LargeChange     =   1
         Min             =   -30
         Max             =   30
         TickStyle       =   3
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldEQ 
         Height          =   1215
         Index           =   1
         Left            =   1080
         TabIndex        =   17
         Top             =   1080
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   2143
         _Version        =   393216
         Orientation     =   1
         Min             =   -10
         TickStyle       =   2
         TickFrequency   =   0
      End
      Begin MSComctlLib.Slider sldEQ 
         Height          =   1215
         Index           =   2
         Left            =   1920
         TabIndex        =   18
         Top             =   1080
         Width           =   675
         _ExtentX        =   1191
         _ExtentY        =   2143
         _Version        =   393216
         Orientation     =   1
         Min             =   -10
         TickStyle       =   2
         TickFrequency   =   0
      End
      Begin VB.Label lblTempo 
         Alignment       =   2  'Center
         Caption         =   "Tempo = 0%"
         Height          =   195
         Left            =   2760
         TabIndex        =   14
         ToolTipText     =   "click to set Tempo to 0%"
         Top             =   840
         Width           =   2535
      End
      Begin VB.Label lblRate 
         Alignment       =   2  'Center
         Caption         =   "Samplerate = 44100Hz"
         Height          =   195
         Left            =   2760
         TabIndex        =   15
         ToolTipText     =   "click to set Samplerate to 44100Hz"
         Top             =   1440
         Width           =   2535
      End
      Begin VB.Label lblPitch 
         Alignment       =   2  'Center
         Caption         =   "Pitch Scaling = 0 semitones"
         Height          =   195
         Left            =   2760
         TabIndex        =   3
         ToolTipText     =   "click to set Pitch Scaling to 0 semitones"
         Top             =   2040
         Width           =   2535
      End
      Begin VB.Label lblPosition 
         Alignment       =   2  'Center
         Caption         =   "Playing position: 00:00 / 00:00"
         Height          =   195
         Left            =   120
         TabIndex        =   2
         Top             =   2640
         Width           =   5175
      End
      Begin VB.Label lblDX8EQ 
         AutoSize        =   -1  'True
         Caption         =   "-= DSP Peaking Equalizer =-"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   8.25
            Charset         =   177
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   195
         Left            =   120
         TabIndex        =   8
         Top             =   840
         Width           =   2415
      End
      Begin VB.Label lbl10khz 
         Alignment       =   2  'Center
         Caption         =   "8 khz"
         Height          =   195
         Left            =   1840
         TabIndex        =   7
         Top             =   2330
         Width           =   720
      End
      Begin VB.Label lbl1khz 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "1 khz"
         Height          =   195
         Left            =   1000
         TabIndex        =   6
         Top             =   2330
         Width           =   735
      End
      Begin VB.Label lbl125hz 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "125 hz"
         Height          =   195
         Left            =   160
         TabIndex        =   5
         Top             =   2325
         Width           =   720
      End
      Begin VB.Label lblVolume 
         Alignment       =   2  'Center
         AutoSize        =   -1  'True
         Caption         =   "Volume"
         Height          =   195
         Left            =   120
         TabIndex        =   4
         Top             =   240
         Width           =   5175
      End
   End
End
Attribute VB_Name = "frmTempo"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'==============================================================================
' frmTempo.frm - Copyright (c) 2003-2008 (: JOBnik! :) [Arthur Aminov, ISRAEL]
'                                                      [http://www.jobnik.org]
'                                                      [   bass_fx@jobnik.org]
'
' BASS_FX tempo / rate / pitch with dsp fx
' * Requires: BASS 2.4 (available @ www.un4seen.com)
'==============================================================================

Option Explicit

Dim chan As Long                ' tempo channel handle
Dim fxEQ As Long                ' dsp peaking eq handle
Dim eq As BASS_BFX_PEAKEQ       ' dsp peaking equalizer
Dim freq As Single              ' sample rate
Dim oldfreq As Single           ' old sample rate
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
        
    ' initialize - default device, 44100hz, stereo, 16 bits
    If (BASS_Init(-1, 44100, 0, Me.hWnd, 0) = 0) Then
        Call Error_("Can't initialize device")
        End
    End If
 
    freq = 44100
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

    ' free previous tempo & dsp handles
    Call BASS_StreamFree(chan) ' stream & music

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

    ' save the original sample rate
    freq = info.freq
    oldfreq = freq

    ' set MAX length to sldPosition Scroller in seconds
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

    ' set dsp eq to channel
    Call SetDSP_EQ(0, 2.5, 0, 125, 1000, 8000)

    ' update the Button to show the loaded file name
    cmdOpenFP.Caption = GetFileName(CMD.filename)

    ' set Volume
    Call sldVolume_Scroll
    
    ' set rate min/max values according to current frequency
    With sldTempo(1)
        .min = 0    'to avoit min > max
        .max = 1    'to avoid max < min
        .max = freq * 1.3
        .min = freq * 0.7
        .LargeChange = freq * 0.01      ' by 1%
    End With

    ' update the Rate lable tooltiptext
    lblRate.ToolTipText = "click to set Samplerate to " & freq & "Hz"
    
    ' update the approximate time in seconds view
    Call UpdatePositionLabel
    
    ' play new created stream
    Call BASS_ChannelPlay(chan, BASSFALSE)

    ' set tempo/rate/pitch to normal
    Call lblTempo_Click
    Call lblRate_Click
    Call lblPitch_Click
End Sub

Private Sub lblTempo_Click()
    sldTempo(0).value = 0
    Call sldTempo_Scroll(0)
End Sub

Private Sub lblRate_Click()
    sldTempo(1).value = freq
    Call sldTempo_Scroll(1)
End Sub

Private Sub lblPitch_Click()
    sldTempo(2).value = 0
    Call sldTempo_Scroll(2)
End Sub

Private Sub sldPosition_Change()
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub
    
    Call BASS_ChannelSetPosition(chan, BASS_ChannelSeconds2Bytes(chan, sldPosition.value), BASS_POS_BYTE)

    ' update the approximate time in seconds view
    Call UpdatePositionLabel
End Sub

Private Sub sldPosition_Scroll()
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub
    ' update the approximate time in seconds view
    Call UpdatePositionLabel
End Sub

Private Sub sldTempo_Scroll(index As Integer)
    If (BASS_ChannelIsActive(chan) = 0) Then Exit Sub

    Select Case (index)
        Case 0:
            Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO, sldTempo(0).value)
            lblTempo.Caption = "Tempo = " & sldTempo(0).value & "%"
            sldTempo(0).Text = sldTempo(0).value & "%"
        Case 1:
            Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_FREQ, sldTempo(1).value)
            lblRate.Caption = "Samplerate = " & sldTempo(1).value & "Hz"
            sldTempo(1).Text = sldTempo(1).value & "Hz"
            
            ' update all bands fCenters after changing samplerate
            Dim i As Integer
            For i = 0 To 2
                eq.lBand = i
                Call BASS_FXGetParameters(fxEQ, eq)
                    eq.fCenter = eq.fCenter * sldTempo(1).value / oldfreq
                Call BASS_FXSetParameters(fxEQ, eq)
            Next i
            oldfreq = sldTempo(1).value
        Case 2:
            Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_TEMPO_PITCH, sldTempo(2).value)
            lblPitch.Caption = "Pitch Scaling = " & sldTempo(2).value & " semitones"
    End Select
    ' update the approximate time in seconds view
    Call UpdatePositionLabel
End Sub

Private Sub sldVolume_Scroll()
    Call BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, sldVolume.value / 100)
    sldVolume.Text = sldVolume.value & "%"
End Sub

Private Sub sldEQ_Change(index As Integer)
    Call UpdateFX(index)
End Sub

Private Sub sldEQ_Scroll(index As Integer)
    Call UpdateFX(index)
    sldEQ(index).Text = sldEQ(index).value * -1
End Sub

' set dsp peaking eq
Public Sub SetDSP_EQ(ByVal fGain As Single, ByVal fBandwidth As Single, ByVal fQ As Single, ByVal fCenter_Bass As Single, ByVal fCenter_Mid As Single, ByVal fCenter_Treble As Single)
    ' set peaking equalizer effect with no bands
    fxEQ = BASS_ChannelSetFX(chan, BASS_FX_BFX_PEAKEQ, 0)

    eq.fGain = fGain
    eq.fQ = fQ
    eq.fBandwidth = fBandwidth
    eq.lChannel = BASS_BFX_CHANALL

    ' create 1st band for bass
    eq.lBand = 0
    eq.fCenter = fCenter_Bass
    Call BASS_FXSetParameters(fxEQ, eq)

    ' create 2nd band for mid
    eq.lBand = 1
    eq.fCenter = fCenter_Mid
    Call BASS_FXSetParameters(fxEQ, eq)

    ' create 3rd band for treble
    eq.lBand = 2
    eq.fCenter = fCenter_Treble
    Call BASS_FXSetParameters(fxEQ, eq)

    ' update dsp eq
    Call UpdateFX(0)
    Call UpdateFX(1)
    Call UpdateFX(2)
End Sub

' update dsp peaking eq
Public Sub UpdateFX(ByVal b As Integer)
    eq.lBand = b    ' get values of the selected band
    Call BASS_FXGetParameters(fxEQ, eq)
        eq.fGain = sldEQ(b).value * -1
    Call BASS_FXSetParameters(fxEQ, eq)
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
