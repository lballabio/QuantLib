Attribute VB_Name = "Module2"
Option Explicit

Function FaureBase2(n) As Double
'   Returns the equivalent first Faure sequence number
    Dim f As Double, sb As Double
    Dim i As Long, n1 As Long, n2 As Long
    n1 = n
    f = 0
    sb = 1 / 2
    Do While n1 > 0
        n2 = Int(n1 / 2)
        i = n1 - n2 * 2
        f = f + sb * i
        sb = sb / 2
        n1 = n2
    Loop
    FaureBase2 = f
End Function

Function Faure1Base2(n) As Double
'   Returns the equivalent first Faure sequence number
'   Uses FaureOct2Bin fn
    Dim nbase8, ai
    Dim f2 As Double
    Dim nbase2 As String
    Dim i As Integer, mx As Integer
    nbase8 = Oct(n)
    nbase2 = FaureOct2Bin(nbase8)
    mx = Len(nbase2)
    f2 = 0
    For i = 1 To mx
        ai = Val(Mid(nbase2, i, 1))
        f2 = f2 + ai / (2 ^ (mx + 1 - i))
    Next i
    Faure1Base2 = f2
End Function

Function FaureOct2Bin(nb8)
'   Converts Octal numbers to a Binary string
'   Uses MOct2Bin fn
    Dim temp As String
    Dim i As Integer, mx As Integer
    mx = Len(nb8)
    For i = 1 To mx
        temp = temp & MOct2Bin(Mid(nb8, i, 1))
    Next i
    FaureOct2Bin = temp
End Function

Function MOct2Bin(nb8)
'   Converts Octal single-digit number to Binary string
    Dim nb2 As String
    Select Case nb8
        Case 0
            nb2 = "000"
        Case 1
            nb2 = "001"
        Case 2
            nb2 = "010"
        Case 3
            nb2 = "011"
        Case 4
            nb2 = "100"
        Case 5
            nb2 = "101"
        Case 6
            nb2 = "110"
        Case 7
            nb2 = "111"
    End Select
    MOct2Bin = nb2
End Function

