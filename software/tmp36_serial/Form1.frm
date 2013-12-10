VERSION 5.00
Object = "{648A5603-2C6E-101B-82B6-000000000014}#1.1#0"; "MSCOMM32.OCX"
Begin VB.Form Form1 
   Caption         =   "Current Temperature"
   ClientHeight    =   4980
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6975
   LinkTopic       =   "Form1"
   ScaleHeight     =   4980
   ScaleWidth      =   6975
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer Timer1 
      Interval        =   100
      Left            =   6360
      Top             =   840
   End
   Begin MSCommLib.MSComm MSComm1 
      Left            =   6240
      Top             =   120
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
      DTREnable       =   -1  'True
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      Caption         =   "Loading..."
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   72
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1605
      Left            =   120
      TabIndex        =   0
      Top             =   1440
      Width           =   6810
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim buf As String

Private Sub Form_Load()
    MSComm1.CommPort = 14
    MSComm1.PortOpen = True
End Sub

Private Sub Timer1_Timer()
    If MSComm1.InBufferCount > 0 Then
        buf = buf & MSComm1.Input
        If InStr(buf, vbCrLf) Then
            If InStr(Mid(buf, InStr(buf, vbCrLf) + 1), vbCrLf) Then
                temp = Mid(buf, InStr(buf, vbCrLf) + 2, InStr(Mid(buf, InStr(buf, vbCrLf) + 2), vbCrLf) - 1)
                Label1.Caption = temp & Chr(176) & "C"
                buf = ""
            End If
        End If
    End If
End Sub
