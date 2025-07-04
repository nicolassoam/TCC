Attribute VB_Name = "Módulo1"
Sub freq_aviao()
    Application.ActiveSheet.Range("K1").Select
    
    Dim aviao As String
    aviao = Application.Selection.Value
    Application.ActiveSheet.Range("G1").Select
    
    Application.Selection.Value = "FREQUÊNCIA DO AVIÃO " + aviao
    Application.ActiveSheet.Range("G2").Select
    Application.Selection.Formula = "=SOMASE(C2:C20000, """ + aviao + """, D2:D20000)"
End Sub

Sub pass_aviao()
    Application.ActiveSheet.Range("K1").Select
    
    Dim aviao As String
    aviao = Application.Selection.Value
    Application.ActiveSheet.Range("H1").Select
   
    Application.Selection.Value = "PASSAGEIROS DO AVIÃO " + aviao
    Application.ActiveSheet.Range("H2").Select
    Application.Selection.Formula = "=SOMASE(C2:C20000, """ + aviao + """, E2:E20000)"
End Sub

Sub voos_ida()
    Application.ActiveSheet.Range("K1").Select
    
    Dim aviao As String
    aviao = Application.Selection.Value
    Application.ActiveSheet.Range("I1").Select

    Application.Selection.Value = "VOOS DE IDA " + aviao
    Application.ActiveSheet.Range("I2").Select
    Application.Selection.Formula = "=SOMASES(D2:D20000,A2:A20000, ""*SBGO"", C2:C20000,""" + aviao + """)"
End Sub
