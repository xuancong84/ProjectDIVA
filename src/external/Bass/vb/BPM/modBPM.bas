Attribute VB_Name = "modBPM"
'============================================================================
' modBPM.bas - Copyright (c) 2003-2012 (: JOBnik! :) [Arthur Aminov, ISRAEL]
'                                                    [http://www.jobnik.org]
'                                                    [   bass_fx@jobnik.org]
' Other source: frmBPM.frm
'
' BASS_FX bpm with tempo & samplerate changers
' * Requires: BASS 2.4 (available @ www.un4seen.com)
'============================================================================

Option Explicit

' Declare an API Timer
Public Const TIME_ONESHOT = 0       ' program timer for single event
Public Declare Function timeSetEvent Lib "winmm.dll" (ByVal uDelay As Long, ByVal uResolution As Long, ByVal lpFunction As Long, ByVal dwUser As Long, ByVal uFlags As Long) As Long
Public Declare Function timeKillEvent Lib "winmm.dll" (ByVal uID As Long) As Long

Public chan As Long                 ' a tempo handle
Public hBPM As Long                 ' decoding bpm handle
Public orgBPM As Single             ' original bpm

' show the approximate position in MM:SS format
Public Sub UpdatePositionLabel()
    Dim totalsec As Single, posec As Single

    If (BASS_FX_TempoGetRateRatio(chan)) Then
        With frmBPM
            totalsec = .sldPosition.max / BASS_FX_TempoGetRateRatio(chan)
            posec = .sldPosition.value / BASS_FX_TempoGetRateRatio(chan)
    
            .sldPosition.Text = Format(Int(posec) \ 60, "00") & ":" & Format(Int(posec) Mod 60, "00") & " / " & _
                                Format(Int(totalsec) \ 60, "00") & ":" & Format(Int(totalsec) Mod 60, "00")
            .lblPos.Caption = "Playing position: " & .sldPosition.Text
        End With
    End If
End Sub

Public Function GetNewBPM(ByVal hBPM As Long) As Single
    GetNewBPM = BASS_FX_BPM_Translate(hBPM, BASS_FX_TempoGetRateRatio(chan) * 100#, BASS_FX_BPM_TRAN_PERCENT2)
    
    ' or you could do it this way too :)
    ' GetNewBPM = orgBPM * BASS_FX_TempoGetRateRatio(chan)
End Function

Public Sub DecodingBPM(ByVal newStream As Boolean, ByVal startSec As Double, ByVal endSec As Double)
    If newStream Then
        ' Open file for bpm decoding detection
        hBPM = BASS_StreamCreateFile(BASSFALSE, StrPtr(frmBPM.CMD.filename), 0, 0, BASS_STREAM_DECODE)
        If (hBPM = 0) Then hBPM = BASS_MusicLoad(BASSFALSE, StrPtr(frmBPM.CMD.filename), 0, 0, BASS_MUSIC_DECODE Or BASS_MUSIC_PRESCAN, 0)
    End If

    orgBPM = BASS_FX_BPM_DecodeGet(hBPM, startSec, endSec, 0, BASS_FX_BPM_BKGRND Or BASS_FX_BPM_MULT2 Or BASS_FX_FREESOURCE, AddressOf GetBPM_Process, 0)

    ' don't bother to update the BPM view if it's zero
    If (orgBPM) Then frmBPM.lblBPM.Caption = "BPM: " & Format(GetNewBPM(hBPM), "0.00") ' update the bpm view
End Sub

'------------------------------------------
'----------- CALLBACK FUNCTIONS -----------
'------------------------------------------

' get the bpm after period of time
Public Sub GetBPM_Callback(ByVal handle As Long, ByVal bpm As Single, ByVal user As Long)
    ' don't bother to update the BPM view if it's zero
    If (bpm) Then frmBPM.lblBPM.Caption = "BPM: " & Format(GetNewBPM(handle), "0.00")
End Sub

' get the bpm process detection in percents of a decoding channel
Public Sub GetBPM_Process(ByVal chan As Long, ByVal per As Single, ByVal user As Long)
    ' update the progress bar
    frmBPM.pbProcess.value = per
End Sub

' get the beat position in seconds CALLBACK
Public Sub GetBeatPos_Callback(ByVal handle As Long, ByVal beatpos As Double, ByVal user As Long)
    Dim curpos As Double
    curpos = BASS_ChannelBytes2Seconds(handle, BASS_ChannelGetPosition(handle, BASS_POS_BYTE))
    Call timeSetEvent((beatpos - curpos) * 1000, 0, AddressOf beatTimerProc, user, TIME_ONESHOT)
End Sub

' beat timer proc CALLBACK
Public Sub beatTimerProc(ByVal uTimerID As Long, ByVal uMsg As Long, ByVal dwUser As Long, ByVal dw1 As Long, ByVal dw2 As Long)
    If (BASS_FX_TempoGetRateRatio(chan)) Then
        frmBPM.lblBeat.Caption = "Beat pos: " & Format(BASS_ChannelBytes2Seconds(chan, BASS_ChannelGetPosition(chan, BASS_POS_BYTE)) / BASS_FX_TempoGetRateRatio(chan), "0.00")
    End If
    Call timeKillEvent(uTimerID)
End Sub
