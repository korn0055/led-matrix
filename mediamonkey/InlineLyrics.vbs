'
' MediaMonkey Script
'
' NAME: InlineLyrics 3.2
'
' AUTHOR: trixmoto (http://trixmoto.net)
' DATE : 29/07/2014
'
' INSTALL: Copy to Scripts directory and add the following to Scripts.ini 
'          Don't forget to remove comments (') and set the order appropriately
'
' FIXES: Fixed sync delay when seeking in a track
'
' [InlineLyrics]
' FileName=InlineLyrics.vbs
' ProcName=InlineLyrics
' Order=12
' DisplayName=&Inline Lyrics
' Description=Show timed lyrics in a progress bar
' Language=VBScript
' ScriptType=2 
'

Option Explicit

  Public lastTxt
  lastTxt = "none1"
Public MSComm1
Set MSComm1 = Nothing

Sub InlineLyrics
  'check for LyricTimer running  
  
  'MsgBox "CallMe was called by the action defined in the Scripts.ini file."
  Dim form : Set form = SDB.Objects("LyricTimer")
  If Not (form is Nothing) Then
    If form.Common.Visible Then 
      Exit Sub
    End If
  End If
  
  'create display
  Dim line : Set line = SDB.Progress
  line.Text = "InlineLyrics by Trixmoto (http://trixmoto.net)"
  Set SDB.Objects("InlineLyricsLine") = line
  
  'check for current song
  Dim itm : Set itm = SDB.Player.CurrentSong
  If itm Is Nothing Then
    Call Error("No current song in player")
    Exit Sub
  End If    

  'check for lyrics
  Dim lrc : lrc = itm.Lyrics
  If lrc = "" Then
    Call Error("No lyrics in current song")
    Exit Sub
  End If
  
  'remove header
  lrc = deheader(lrc)
  If lrc = "" Then
    Call Error("No timestamps in lyrics")
    Exit Sub    
  End If

  'removed enhanced timestamps  
  lrc = simplify(lrc)
  If lrc = "" Then
    Call Error("No lyric lines found")
    Exit Sub    
  End If
 
  'create data
  Dim dat : Set dat = CreateObject("Scripting.Dictionary")
  dat.Item("idx") = itm.ID
  dat.Item("tot") = itm.SongLength  
  dat.Item("lst") = itm.SongLength
  dat.Item("pos") = 0
  dat.Item("tsp") = 0
dat.Item("last") = "none"	
  Dim i : i = 0
  Dim arr : arr = split(lrc,VBCrLf)
  For i = 0 To UBound(arr)
    dat.Item("#"&i) = arr(i)
  Next
  dat.Item("max") = UBound(arr)
  Set SDB.Objects("InlineLyricsData") = dat

Dim oTrack
Set oTrack = SDB.Player.CurrentSong
Set MSComm1=CreateObject("MSCOMMLib.MSComm")
MSComm1.Settings = "9600,n,8,1"
MSComm1.CommPort = 5
MSComm1.InBufferCount = 0
'MSComm1.Handshaking = NoHandshaking
MSComm1.DTREnable = false
MSComm1.PortOpen = True
SDB.Tools.Sleep(2000)
   If Err Then
      MsgBox "COM" & MSComm1.CommPort & ": not opened, Sorry!"
      Exit Sub
   End If
MSComm1.Output= oTrack.ArtistName & " - " & oTrack.Title & CHR(13)  
SDB.Tools.Sleep(1000)


  'start loop
  Dim tmr : Set tmr = SDB.CreateTimer(100)
  Set SDB.Objects("InlineLyricsTimer") = tmr
  Call Script.RegisterEvent(tmr,"OnTimer","MainLoop") 
End Sub

Sub MainLoop(Tmr)
  'check for display
  Dim lin : Set lin = SDB.Objects("InlineLyricsLine")
  If lin Is Nothing Then
    Call Script.UnregisterEvents(Tmr)
    Call SDB.MessageBox("InlineLyrics: The progress bar has been lost",mtError,Array(mbOk))
    Exit Sub
  End If  
  If lin.Terminate Then
    Call Script.UnregisterEvents(Tmr)
    Call Error("Cancelled by user")
    Exit Sub
  End If

  'check for data  
  Dim dat : Set dat = SDB.Objects("InlineLyricsData")
  If dat Is Nothing Then
    Call Script.UnregisterEvents(Tmr)
    Call Error("Data has been lost")
    Exit Sub
  End If

  'check for song change  
  If Not SDB.Player.CurrentSong.ID = Int(dat.Item("idx")) Then
    Call Script.UnregisterEvents(Tmr)
    Exit Sub
  End If
  
  'check for player stopped
  If Not SDB.Player.isPlaying Then
    Call Script.UnregisterEvents(Tmr)
    Call Error("Player stopped")
    Exit Sub
  End If
  
  'check for end of song
  If SDB.Player.PlaybackTime+100 > Clng(dat.Item("tot")) Then
    Call Script.UnregisterEvents(Tmr)
    Exit Sub
  End If
  
  'check for LyricTimer
  Dim form : Set form = SDB.Objects("LyricTimer")
  If Not (form is Nothing) Then
    If form.Common.Visible Then 
      Call Script.UnregisterEvents(Tmr)
      Exit Sub
    End If
  End If

  'check for position decreased
  Dim tsp : tsp = Int(dat.Item("tsp"))
  Dim pos : pos = Int(dat.Item("pos"))
  Dim lst : lst = Clng(dat.Item("lst"))
  If SDB.Player.PlaybackTime < lst Then 
    pos = 0
    tsp = gettime(dat.Item("#"&pos))
  End If
   
  'check for next line
  Dim max : max = Int(dat.Item("max"))
  If pos <= max Then
    While (pos <= max) And (tsp < SDB.Player.PlaybackTime+100)
      pos = pos+1
      If pos <= max Then 
        tsp = gettime(dat.Item("#"&pos))
      End If
    WEnd
    If pos > 0 Then
      lin.Text = getline(dat.Item("#"&(pos-1)))

	If StrComp(dat.Item("last"), lin.Text) <> 0 Then
	  dat.Item("last") = lin.Text	  
	  If Not (MSComm1 is Nothing) Then
		MSComm1.Output= lin.Text & CHR(13)  
		End If
		'	  MsgBox("current = " & lin.Text & "  last = " & dat.Item("last"))
	End If
    End If
    lst = SDB.Player.PlaybackTime
  End If
  
  'save data
  dat.Item("pos") = pos
  dat.Item("lst") = lst
  dat.Item("tsp") = tsp
End Sub

Sub Error(txt)
  'check for display
  Dim lin : Set lin = SDB.Objects("InlineLyricsLine")
  If lin Is Nothing Then
    Call SDB.MessageBox("InlineLyrics: "&txt,mtError,Array(mbOk))
    Exit Sub
  End If  

  'display error for short time
  lin.Text = "InlineLyrics: "&txt
  Dim tmr : Set tmr = SDB.CreateTimer(5000)
  Set SDB.Objects("InlineLyricsTimer") = tmr  
  Call Script.RegisterEvent(tmr,"OnTimer","ClearUp") 
End Sub

Sub ClearUp(Tmr)
  Call Script.UnregisterEvents(Tmr)
  Set SDB.Objects("InlineLyricsTimer") = Nothing
  Set SDB.Objects("InlineLyricsData") = Nothing  
  Set SDB.Objects("InlineLyricsLine") = Nothing
If Not (MSComm1 is Nothing) Then
MSComm1.PortOpen = False
Set MSComm1 = Nothing
End If
End Sub

Function deheader(txt)
  Dim pos : pos = InStr(txt,"[0")
  If pos = 0 Then
    deheader = ""
  Else
    deheader = Mid(txt,pos)
  End If
End Function

Function simplify(txt)
  Dim a : a = InStr(txt,"<")
  If a = 0 Then
    simplify = txt
    Exit Function
  End If
  
  Dim b : b = InStr(txt,">")
  Dim l : l = Len(txt)
  Dim c : c = Mid(txt,1,a-1)
  If b = 0 Then 
    b = a
  End If
  Dim d : d = Mid(txt,b+1,l-b)
  txt = c&d
  a = InStr(txt,"<")
  If a > 0 Then 
    txt = simplify(txt)
  End If
  simplify = txt
End Function

Function gettime(txt)
  gettime = 0
  If Left(txt,1) = "[" Then
    Dim temp,min,sec,mil
    temp = Mid(txt,2,2)
    If isNumeric(temp) Then 
      min = Clng(temp)
    End if
    temp = Mid(txt,5,2)
    If isNumeric(temp) Then 
      sec = Clng(temp)
    End If
    temp = Mid(txt,8,2)
    If isNumeric(temp) Then 
      mil = Clng(temp)  
    End If
    gettime = min*60000 + sec*1000 + mil*1    
  End If
End Function

Function getline(txt)
  getline = Mid(txt,11,Len(txt)-10)
End Function

Sub Install()
  Dim inip : inip = SDB.ApplicationPath&"Scripts\Scripts.ini"
  Dim inif : Set inif = SDB.Tools.IniFileByPath(inip)
  If Not (inif Is Nothing) Then
    inif.StringValue("InlineLyrics","Filename") = "InlineLyrics.vbs"
    inif.StringValue("InlineLyrics","Procname") = "InlineLyrics"
    inif.StringValue("InlineLyrics","Order") = "12"
    inif.StringValue("InlineLyrics","DisplayName") = "Inline Lyrics"
    inif.StringValue("InlineLyrics","Description") = "Show timed lyrics in a progress bar"
    inif.StringValue("InlineLyrics","Language") = "VBScript"
    inif.StringValue("InlineLyrics","ScriptType") = "2"
    SDB.RefreshScriptItems
  End If
End Sub