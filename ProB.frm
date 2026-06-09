Option Explicit
Dim cbStoneNames(1 To 50) As MSForms.ComboBox

Dim originalWidth As Double, originalHeight As Double, stepWidth As Double, stepHeight As Double, mouseMoveCounter As Integer
Dim Xi() As Object  ' Dictionary dizisi
Dim headerLabels() As Object
Dim Lsr As ShapeRange
Dim Kalip1 As ShapeRange
Dim selectedStoneRange As New ShapeRange
Dim colorBoxCount As Integer
Dim tableColumnWidths As Variant
Dim SrKUst As New ShapeRange
Dim SrKAlt As New ShapeRange
Dim bannerIndex As Integer
Dim bannerTimer As Double
Dim bannerTexts As Variant
Dim SrG As New ShapeRange
Dim srKalip As New ShapeRange
Dim srText As New ShapeRange
Dim KalipBilgisi As String
Dim srT(1 To 50) As Shape
Dim SrTNo As Integer

Private Sub Label466_Click()
    Dim cbName1 As MSForms.ComboBox
    Set cbName1 = fraList.Controls("cbStoneName1")
    MsgBox cbName1.Text
End Sub

Private Sub lblDesigner_Click()
    Dim DesignerName As String
    DesignerName = InputBox("Desinatör Adı", "Desinatör Adı", "")
    DesignerName = UCase(Left(DesignerName, 1)) & LCase(Mid(DesignerName, 2))
    
    If DesignerName = "" Then Exit Sub
    lblDesigner.Caption = Split(lblDesigner.Caption, ":")(0) & ": " & DesignerName
    
    SaveSetting "SizeFind1", "Preferences", "Designer", DesignerName
End Sub

Function ModelFirma() As String
    Dim dosyaKlasoru As String, dosyaAdi As String
    Dim firmaAdi As String, ModelNo As String, DesignerT As String

    dosyaKlasoru = ActiveDocument.FilePath
    dosyaAdi = ActiveDocument.FileName

    ' Firma adını belirle
    If InStr(dosyaKlasoru, "ARŞİV\") > 0 Then
        firmaAdi = Split(Split(dosyaKlasoru, "ARŞİV\")(1), "\")(0)
    Else
        firmaAdi = ""
    End If

    ' Model adını belirle
    If dosyaAdi <> "" Then
        ModelNo = Left(dosyaAdi, InStrRev(dosyaAdi, ".") - 1)
        ' Alt çizgi varsa en sondakini model olarak al
        If InStr(ModelNo, "_") > 0 Then
            ModelNo = Split(ModelNo, "_")(UBound(Split(ModelNo, "_")))
        End If
    Else
        ModelNo = ""
    End If
    
    DesignerT = Left(Trim(Split(lblDesigner.Caption, ": ")(1)), 2)
    If firmaAdi <> "" Then
        ModelFirma = firmaAdi & DesignerT & "_" & ModelNo
    Else
        ModelFirma = DesignerT & "_" & ModelNo
    End If
End Function

Private Sub btnCalculate_Click()
    Optimize True, "Hesap Tablosu Oluştur"
    
    ActiveDocument.Unit = cdrMillimeter
    fraCalculate.Controls.Clear

    Dim s As Shape, S1 As Shape, Swh As Shape
    Dim Sruuu As New ShapeRange
    
    Dim i As Integer, j As Integer, k As Integer
    Dim Ti As Integer, Tj As Integer
    Dim KalipT As Integer, RenkT As Integer
    Dim r As Integer, g As Integer, b As Integer
    
    Dim xPs As Double, yPs As Double
    
    Dim txt As String, metin As String, veri As String, bant As String, key As String
    Dim parca As Variant, satir As Variant, keys As Variant, t As Variant
    Dim TaslarB(0 To 100, 0 To 100, 0 To 100) As String
    
    Dim dict As Object

    Set dict = CreateObject("Scripting.Dictionary")

    ' --- VERİLERİ OKU ---
    For Each s In ActivePage.Shapes.FindShapes(Query:="@type = 'text:artistic'").ReverseRange
        txt = s.Text.Story
        If s.Text.Story.Size = 8.01 And InStr(txt, "X") > 0 And UBound(Split(txt, ":")) > 7 Then
            KalipT = KalipT + 1
            Set Swh = ActivePage.SelectShapesAtPoint(s.LeftX + 20, s.BottomY - 20, True)
            Set Sruuu = ActivePage.SelectShapesFromRectangle(s.RightX + 1, s.TopY, Swh.RightX, Swh.TopY + 1, True).Shapes.FindShapes(Query:="@type = 'text:artistic'").All

            metin = Replace(Replace(txt, vbCrLf, vbLf), vbCr, vbLf)

            For Each satir In Split(metin, vbLf)
                If InStr(satir, ":") > 0 Then
                    parca = Split(satir, ":")
                    If UBound(parca) >= 1 Then
                        Select Case True
                            Case InStr(LCase(parca(0)), "firma"): TaslarB(Ti, 0, 0) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "dosya"): TaslarB(Ti, 0, 1) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "kalıp adı"): TaslarB(Ti, 0, 2) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "renk"): TaslarB(Ti, 0, 5) = Trim(parca(1)): RenkT = RenkT + Trim(parca(1))
                            Case InStr(LCase(parca(0)), "kalıptaki"): TaslarB(Ti, 0, 6) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "toplam"): TaslarB(Ti, 0, 12) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "pres"): TaslarB(Ti, 0, 13) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "montaj"): TaslarB(Ti, 0, 14) = Trim(parca(1))
                            Case InStr(LCase(parca(0)), "ölçü"):
                                TaslarB(Ti, 0, 3) = Trim(Split(parca(1), "X")(0))
                                TaslarB(Ti, 0, 4) = Trim(Split(parca(1), "X")(1))
                            Case Else:
                                TaslarB(Ti, 0, 11) = Trim(Split(metin, vbLf)(UBound(Split(metin, vbLf))))
                        End Select
                    End If
                End If
            Next
            
            TaslarB(Ti, 0, 7) = TaslarB(Ti, 0, 3) * TaslarB(Ti, 0, 5)
            TaslarB(Ti, 0, 8) = TaslarB(Ti, 0, 7) * (TaslarB(Ti, 0, 12) / TaslarB(Ti, 0, 6))
            TaslarB(Ti, 0, 9) = CLng(TaslarB(Ti, 0, 12) / TaslarB(Ti, 0, 6))

            key = TaslarB(Ti, 0, 4)
            dict(key) = IIf(dict.Exists(key), CDbl(dict(key)) + CDbl(TaslarB(Ti, 0, 8)), CDbl(TaslarB(Ti, 0, 8)))

            Tj = 1
            For Each S1 In Sruuu.Shapes.All
                parca = Split(S1.Text.Story, "-")
                TaslarB(Ti, Tj, 0) = S1.Fill.UniformColor.HexValue
                TaslarB(Ti, Tj, 1) = Trim(parca(0))
                TaslarB(Ti, Tj, 2) = Trim(parca(1))
                Select Case LCase(Trim(parca(2)))
                    Case "double": TaslarB(Ti, Tj, 3) = "Dbl"
                    Case "mc": TaslarB(Ti, Tj, 3) = "Mc"
                    Case "rezene": TaslarB(Ti, Tj, 3) = "Rzn"
                    Case "octagon": TaslarB(Ti, Tj, 3) = "Oct"
                    Case "drop": TaslarB(Ti, Tj, 3) = "Drp"
                    Case "düz metal": TaslarB(Ti, Tj, 3) = "Düz"
                    Case "özel": TaslarB(Ti, Tj, 3) = "Özl"
                    Case Else: TaslarB(Ti, Tj, 3) = Trim(parca(2))
                End Select
                TaslarB(Ti, Tj, 4) = Split(Trim(parca(3)), " ")(0)
                TaslarB(Ti, Tj, 5) = Split(Trim(parca(3)), " ")(1)

                key = TaslarB(Ti, Tj, 1) & "_" & TaslarB(Ti, Tj, 2) & "_" & TaslarB(Ti, Tj, 3)
                dict(key) = IIf(dict.Exists(key), CInt(dict(key)) + CInt(TaslarB(Ti, Tj, 4)), TaslarB(Ti, Tj, 4))
                Tj = Tj + 1
            Next
            Ti = Ti + 1
        End If
    Next
    
    ' --- EKRANA YAZDIR ---
    For i = 0 To KalipT
        For j = 0 To 50
            veri = TaslarB(i, j, 1)
            If j = 0 Then
                xPs = 0
                For k = 0 To 9
                    veri = TaslarB(i, j, k)
                    If veri <> "" Then
                        With fraCalculate.Controls.Add("Forms.Label.1")
                            Select Case k
                                Case 3, 4: .Caption = CLng(TaslarB(i, j, k + 2))
                                Case 5, 6:
                                    .Caption = CLng(CLng(TaslarB(i, j, k + 2)) / 100)
                                    If .Caption = 0 Then .Caption = 1
                                Case Else: .Caption = TaslarB(i, j, k + 2)
                            End Select
                            .Move 3 + xPs, yPs, tableColumnWidths(k), 13
                            .ForeColor = IIf(k Mod 2 = 1, vbBlack, RGB(0, 0, 255))
                            .TextAlign = fmTextAlignLeft
                            .AutoSize = True
                        End With
                    End If
                    xPs = xPs + tableColumnWidths(k)
                Next
            ElseIf veri <> "" Then
                xPs = IIf((j - 1) Mod 2 = 0, 3, 230)
                With fraCalculate.Controls.Add("Forms.Label.1")
                    .Move xPs, 13 + yPs, 225, 13
                    .Caption = " " & TaslarB(i, j, 1) & "_" & TaslarB(i, j, 2) & "_" & TaslarB(i, j, 3) & "_" & TaslarB(i, j, 4) & " " & TaslarB(i, j, 5)
                    .BorderStyle = fmBorderStyleSingle
                    .Font.Size = 8
                    r = val("&H" & Mid(TaslarB(i, j, 0), 2, 2))
                    g = val("&H" & Mid(TaslarB(i, j, 0), 4, 2))
                    b = val("&H" & Mid(TaslarB(i, j, 0), 6, 2))
                    .BackColor = RGB(r, g, b)
                    .ForeColor = IIf(0.2126 * r + 0.7152 * g + 0.0722 * b > 100, vbBlack, vbWhite)
                End With
                If xPs = 230 Then yPs = yPs + 14
            End If
        Next
        yPs = yPs + 20
    Next
    yPs = yPs - 20

    ' --- TOPLAM HESAPLAMA ---
    keys = dict.keys
    bant = ""
    For i = LBound(keys) To UBound(keys)
        If InStr(keys(i), "_") = 0 Then
            bant = bant & keys(i) & "X" & CLng(dict(keys(i)) / 100) & "m  "
            keys(i) = ""
        End If
    Next
    '
    bant = "Toplam Kalıp :" & vbCrLf & KalipT & vbCrLf & vbCrLf & "Toplam Renk :" & vbCrLf & RenkT & vbCrLf & vbCrLf & "Toplam Bant :" & vbCrLf & bant & vbCrLf & vbCrLf & "Toplam Taş :" & vbCrLf

    For i = LBound(keys) To UBound(keys) - 1
        For j = i + 1 To UBound(keys)
            If keys(i) <> "" And keys(j) <> "" And dict(keys(i)) < dict(keys(j)) Then
                t = keys(i): keys(i) = keys(j): keys(j) = t
            End If
        Next
    Next

    For i = LBound(keys) To UBound(keys)
        If keys(i) <> "" Then
            bant = bant & Replace(keys(i), "_", " ") & "   " & dict(keys(i)) & " Adet / " & CLng(dict(keys(i)) / 144) & " Gross" & vbCrLf
        End If
    Next
    
    KalipBilgisi = bant
    With fraCalculate.Controls.Add("Forms.Label.1")
        .Caption = bant
        .Move 3, yPs, 500, 500
        .ForeColor = vbBlack
        .TextAlign = fmTextAlignLeft
        .Font.Bold = True
        .AutoSize = True
        yPs = yPs + .Height
    End With
    Optimize False, "Hesap Tablosu Oluştur"
    fraCalculate.ScrollBars = fmScrollBarsVertical
    fraCalculate.ScrollHeight = yPs + 30
    fraCalculate.ScrollTop = 0
End Sub

Private Sub btnSplitTemplate_Click()
    ActiveDocument.Unit = cdrMillimeter
    If ActiveShape Is Nothing Then Exit Sub
    
    Optimize True, "Kalıpları Ayır"
    
    Dim s As Shape, t As Shape, tText As String, sizeT As String, TasTur As String
    Dim valSize As Double
    Dim dup1 As ShapeRange, i As Integer
    
    ' 1. Find bounding box of real stones in selection
    Dim desenMinX As Double, desenMaxX As Double, desenMinY As Double, desenMaxY As Double
    Dim hasStones As Boolean
    
    desenMinX = 999999
    desenMaxX = -999999
    desenMinY = 999999
    desenMaxY = -999999
    hasStones = False
    
    Dim sh As Shape, shFlat As ShapeRange
    Set shFlat = ActiveSelectionRange.Shapes.FindShapes()
    For Each sh In shFlat
        If sh.Type <> cdrTextShape Then
            Dim nameLower As String
            nameLower = LCase(sh.Name)
            Dim isCutObject As Boolean
            isCutObject = (InStr(nameLower, "kesim") > 0 Or InStr(nameLower, "kalip") > 0 Or InStr(nameLower, "ztrass") > 0)
            
            If Not isCutObject Then
                Dim w As Double, h As Double
                w = sh.SizeWidth
                h = sh.SizeHeight
                
                Dim fillType As Long
                fillType = sh.Fill.Type
                Dim fillCol As Long
                fillCol = -1
                If fillType = 1 Then
                    fillCol = sh.Fill.UniformColor.ColorRgb
                End If
                
                Dim isRealStone As Boolean
                isRealStone = (w < 10 And h < 10 And fillType = 1 And fillCol <> RGB(255, 255, 255) And fillCol <> 0)
                
                If isRealStone Then
                    If sh.LeftX < desenMinX Then desenMinX = sh.LeftX
                    If sh.RightX > desenMaxX Then desenMaxX = sh.RightX
                    If sh.BottomY < desenMinY Then desenMinY = sh.BottomY
                    If sh.TopY > desenMaxY Then desenMaxY = sh.TopY
                    hasStones = True
                End If
            End If
        End If
    Next sh
    
    If Not hasStones Then
        desenMinX = ActiveSelectionRange.LeftX
        desenMaxX = ActiveSelectionRange.RightX
        desenMinY = ActiveSelectionRange.BottomY
        desenMaxY = ActiveSelectionRange.TopY
    End If
    
    ' 2. Find pKalipKare and marker shapes on the active page
    Dim pKalipKare As Shape
    Dim kalipLeft As Double, kalipRight As Double, kalipBottom As Double, kalipTop As Double
    Dim markerShapes As New ShapeRange
    Dim pInfoText As Shape
    
    Dim maxArea As Double
    maxArea = 0
    
    For Each sh In ActivePage.Shapes.FindShapes()
        nameLower = LCase(sh.Name)
        isCutObject = (InStr(nameLower, "kesim") > 0 Or InStr(nameLower, "kalip") > 0 Or InStr(nameLower, "ztrass") > 0)
        
        If Not isCutObject And sh.Type = cdrRectangleShape Then
            w = sh.SizeWidth
            h = sh.SizeHeight
            If w > 20 And h > 20 Then
                If sh.Fill.Type = 0 Then
                    isCutObject = True
                End If
            End If
        End If
        
        If isCutObject Then
            If sh.LeftX <= desenMinX + 1.5 And sh.RightX >= desenMaxX - 1.5 And _
               sh.BottomY <= desenMinY + 1.5 And sh.TopY >= desenMaxY - 1.5 Then
                Dim area As Double
                area = sh.SizeWidth * sh.SizeHeight
                If area > maxArea Then
                    maxArea = area
                    Set pKalipKare = sh
                    kalipLeft = sh.LeftX
                    kalipRight = sh.RightX
                    kalipBottom = sh.BottomY
                    kalipTop = sh.TopY
                End If
            End If
        End If
    Next sh
    
    If Not pKalipKare Is Nothing Then
        Dim bestTextDist As Double
        bestTextDist = 999999
        
        For Each sh In ActivePage.Shapes.FindShapes()
            nameLower = LCase(sh.Name)
            Dim isInsideKalip As Boolean
            isInsideKalip = (sh.LeftX >= kalipLeft - 1.5 And sh.RightX <= kalipRight + 1.5 And _
                             sh.BottomY >= kalipBottom - 1.5 And sh.TopY <= kalipTop + 1.5)
            If isInsideKalip Then
                Dim isMarker As Boolean
                isMarker = (InStr(nameLower, "kalip ztrass.com") > 0 Or InStr(nameLower, "kesim") > 0 Or _
                            (sh.Type = cdrEllipseShape And InStr(nameLower, "ztrass") > 0))
                If isMarker And sh.StaticID <> pKalipKare.StaticID Then
                    markerShapes.Add sh
                End If
            Else
                If pInfoText Is Nothing And sh.Type = cdrTextShape Then
                    Dim textValLower As String
                    textValLower = LCase(sh.Text.Story.Text)
                    If InStr(textValLower, "ztrass.com") > 0 Then
                        Dim dx As Double, dy As Double, dist As Double
                        dx = sh.LeftX - kalipLeft
                        dy = sh.BottomY - kalipTop
                        dist = Sqr(dx * dx + dy * dy)
                        If dist < 150 And dist < bestTextDist Then
                            bestTextDist = dist
                            Set pInfoText = sh
                        End If
                    End If
                End If
            End If
        Next sh
    Else
        kalipLeft = ActiveSelectionRange.LeftX
        kalipTop = ActiveSelectionRange.TopY
    End If
    
    Dim dupSR As New ShapeRange
    dupSR.AddRange ActiveSelectionRange.Shapes.FindShapes()
    
    Dim idMap As Object
    Set idMap = CreateObject("Scripting.Dictionary")
    For Each sh In dupSR
        idMap(sh.StaticID) = True
    Next sh
    
    If Not pKalipKare Is Nothing Then
        If Not idMap.Exists(pKalipKare.StaticID) Then
            dupSR.Add pKalipKare
            idMap(pKalipKare.StaticID) = True
        End If
    End If
    
    For Each sh In markerShapes
        If Not idMap.Exists(sh.StaticID) Then
            dupSR.Add sh
            idMap(sh.StaticID) = True
        End If
    Next sh
    
    Dim kalipW As Double
    kalipW = dupSR.SizeWidth
    
    For i = 1 To SrTNo
        ' 3. Find original stone fill color to select corresponding text
        Dim grpCol As Long
        grpCol = -1
        If srT(i).Type = cdrGroupShape Then
            Dim firstSh As Shape
            For Each firstSh In srT(i).Shapes.FindShapes()
                If firstSh.Fill.Type = 1 Then
                    grpCol = firstSh.Fill.UniformColor.ColorRgb
                    Exit For
                End If
            Next firstSh
        Else
            If srT(i).Fill.Type = 1 Then
                grpCol = srT(i).Fill.UniformColor.ColorRgb
            End If
        End If

        ' TasTur
        Dim typeText As String
        typeText = Trim(fraList.Controls("cbStoneType" & i).Text)
        
        Dim r As Long, g As Long, b As Long
        r = 255: g = 0: b = 0 ' Varsayılan Kırmızı
        Select Case typeText
            Case "Mc", "Dbl"
                r = 255: g = 0: b = 0
            Case "Rzn"
                r = 0: g = 0: b = 255 ' Mavi
            Case "Oct"
                r = 255: g = 255: b = 0 ' Sarı
            Case "Drp"
                r = 0: g = 255: b = 255 ' Turkuaz
            Case "Düz"
                r = 0: g = 255: b = 0 ' Yeşil
            Case "Özl"
                r = 255: g = 0: b = 255 ' Magenta
        End Select

        ' Duplicate and move template components
        Dim specSR As New ShapeRange
        If Not pKalipKare Is Nothing Then specSR.Add pKalipKare
        specSR.AddRange markerShapes
        specSR.Add srT(i)
        
        ' Sayfa genelinden bu gruba ait renkli yazıyı bulup ekleyelim
        If grpCol <> -1 Then
            Dim pageSh As Shape
            For Each pageSh In ActivePage.Shapes.FindShapes(Type:=cdrTextShape)
                If pageSh.Fill.Type = 1 Then
                    If pageSh.Fill.UniformColor.ColorRgb = grpCol Then
                        ' Eğer bilgi metni ("ztrass.com" içeren) değilse bu gruba ait renkli yazıdır
                        If InStr(LCase(pageSh.Text.Story.Text), "ztrass.com") = 0 Then
                            specSR.Add pageSh
                        End If
                    End If
                End If
            Next pageSh
        End If
        
        Set dup1 = specSR.Duplicate
        dup1.Move (kalipW + 10) * i, 0
        
        ' Kopyalanan nesneleri renklendirelim
        Dim stoneSh As Shape
        For Each stoneSh In dup1
            Dim nameL As String
            nameL = LCase(stoneSh.Name)
            Dim isMarker As Boolean
            isMarker = (InStr(nameL, "kalip ztrass.com") > 0 Or InStr(nameL, "kesim") > 0 Or (stoneSh.Type = cdrEllipseShape And InStr(nameL, "ztrass") > 0))
            
            Dim isKalip As Boolean
            isKalip = False
            If Not pKalipKare Is Nothing Then
                If stoneSh.Type = cdrRectangleShape And stoneSh.Fill.Type = 0 And stoneSh.SizeWidth > 20 Then
                    isKalip = True
                End If
            End If
            
            If isKalip Then
                ' Kalıp karesinin çizgi rengini boyayalım
                stoneSh.Outline.Color.RGBAssign r, g, b
            ElseIf isMarker Then
                ' İşaret taşının dolgu rengini boyayalım
                stoneSh.Fill.UniformColor.RGBAssign r, g, b
            ElseIf stoneSh.Type = cdrTextShape Then
                ' Yazıyı taşın renginde boya
                stoneSh.Fill.UniformColor.RGBAssign r, g, b
            Else
                ' Taşları boyayalım (grup ise altındakileri de)
                If stoneSh.Type = cdrGroupShape Then
                    Dim subSh As Shape
                    For Each subSh In stoneSh.Shapes.FindShapes()
                        If subSh.Fill.Type = 1 Then
                            subSh.Fill.UniformColor.RGBAssign r, g, b
                        End If
                    Next subSh
                Else
                    If stoneSh.Fill.Type = 1 Then
                        stoneSh.Fill.UniformColor.RGBAssign r, g, b
                    End If
                End If
            End If
        Next stoneSh
        
        ' SizeT
        Dim rawInfo As String
        rawInfo = Trim(Me.Controls("lblStoneInfo" & i).Caption)
        sizeT = Split(rawInfo, "mm-")(0)
        valSize = val(Replace(sizeT, ",", "."))
        
        Select Case valSize
            Case 1.9 To 2.1: sizeT = "ss6"
            Case 2.7 To 2.9: sizeT = "ss10"
            Case 3.8 To 4.0: sizeT = "ss16"
            Case 4.6 To 4.8: sizeT = "ss20"
            Case 6.2 To 6.4: sizeT = "ss30"
            Case 7.2 To 7.4: sizeT = "ss34"
            Case 8.2 To 8.4: sizeT = "ss40"
            Case Else: sizeT = sizeT & "mm"
        End Select
        
        Select Case typeText
            Case "Dbl": TasTur = "Double"
            Case "Mc": TasTur = "Mc"
            Case "Rzn": TasTur = "Rezene"
            Case "Oct": TasTur = "Octagon"
            Case "Drp": TasTur = "Drop"
            Case "Düz": TasTur = "Düz Metal"
            Case "Özl": TasTur = "Özel"
            Case Else: TasTur = typeText
        End Select
        
        Dim stoneName As String
        stoneName = Trim(fraList.Controls("cbStoneName" & i).Text)
        
        tText = Left(txtFirma.Text, 3) & txtModelNo.Text & "  " & sizeT & "  " & stoneName & "_" & TasTur
        
        Set t = ActiveDocument.ActiveLayer.CreateArtisticText(dup1.LeftX + 5, dup1.TopY - 10, tText, cdrAfrikaans, , , 20, cdrTrue)
        t.Fill.UniformColor.RGBAssign r, g, b
        
        ' 4. Duplicate and position Info Text shape
        If Not pInfoText Is Nothing Then
            Dim dupText As Shape
            Set dupText = pInfoText.Duplicate
            dupText.Move (kalipW + 10) * i, 0
            
            ' Metni satırlara bölüp temizleyelim
            Dim txtVal As String
            txtVal = dupText.Text.Story.Text
            
            Dim lines() As String
            lines = Split(txtVal, vbCr)
            
            Dim newTxt As String
            newTxt = ""
            
            Dim lineIdx As Long
            For lineIdx = 0 To UBound(lines)
                Dim line As String
                line = Trim(lines(lineIdx))
                Dim keepLine As Boolean
                keepLine = True
                
                If InStr(line, "mm-") > 0 Or InStr(line, " - ") > 0 Then
                    If typeText <> "" Then
                        ' typeText: "Mc", "Dbl", "Rzn" vb.
                        If InStr(LCase(line), LCase(typeText)) = 0 Then
                            keepLine = False
                        End If
                    End If
                ElseIf InStr(line, "Renk") > 0 Then
                    line = "1 Renk"
                End If
                
                If keepLine And line <> "" Then
                    If newTxt <> "" Then newTxt = newTxt & vbCr
                    newTxt = newTxt & line
                End If
            Next lineIdx
            
            dupText.Text.Story.Text = newTxt
            
            Dim textHeight As Double
            textHeight = dupText.SizeHeight
            dupText.PositionX = kalipLeft + (kalipW + 10) * i + 2
            dupText.PositionY = kalipTop - 2 - textHeight
            dupText.Fill.UniformColor.RGBAssign r, g, b
        End If
    Next i
    
    Optimize False, "Kalıpları Ayır"
End Sub

Private Sub Label524_Click()
    Dim dict As Object
    Set dict = CreateObject("Scripting.Dictionary")
    
    ' 2'li array ekleyelim
    dict("anahtar1") = Array(10, 20)
    dict("anahtar2") = Array("merhaba", "dünya")
    
    ' Kullanımı
    MsgBox dict("anahtar1")(0)  ' 10
    MsgBox dict("anahtar1")(1)  ' 20
    MsgBox dict("anahtar2")(0)  ' merhaba
    MsgBox dict("anahtar2")(1)  ' dünya
End Sub

Private Sub btnMachineTemplate_Click()
    If ActiveShape Is Nothing Then Exit Sub
    Optimize True, "Makine Kalıbı"
    ActiveDocument.Unit = cdrMillimeter

    Dim i As Integer, val As Double
    For i = 1 To 6
        val = CDbl(Me.Controls("MakS" & i).Text)
        If val <= 0 Then val = 1
        Me.Controls("MakS" & i).Text = CStr(val)
        SaveSetting "SizeFind1", "Preferences", "MakS" & i, val
    Next

    ' 1. Find bounding box of real stones in selection
    Dim desenMinX As Double, desenMaxX As Double, desenMinY As Double, desenMaxY As Double
    Dim hasStones As Boolean
    
    desenMinX = 999999
    desenMaxX = -999999
    desenMinY = 999999
    desenMaxY = -999999
    hasStones = False
    
    Dim sh As Shape, shFlat As ShapeRange
    Set shFlat = ActiveSelectionRange.Shapes.FindShapes()
    For Each sh In shFlat
        If sh.Type <> cdrTextShape Then
            Dim nameLower As String
            nameLower = LCase(sh.Name)
            Dim isCutObject As Boolean
            isCutObject = (InStr(nameLower, "kesim") > 0 Or InStr(nameLower, "kalip") > 0 Or InStr(nameLower, "ztrass") > 0)
            
            If Not isCutObject Then
                Dim w As Double, h As Double
                w = sh.SizeWidth
                h = sh.SizeHeight
                
                Dim fillType As Long
                fillType = sh.Fill.Type
                Dim fillCol As Long
                fillCol = -1
                If fillType = 1 Then
                    fillCol = sh.Fill.UniformColor.ColorRgb
                End If
                
                Dim isRealStone As Boolean
                isRealStone = (w < 10 And h < 10 And fillType = 1 And fillCol <> RGB(255, 255, 255) And fillCol <> 0)
                
                If isRealStone Then
                    If sh.LeftX < desenMinX Then desenMinX = sh.LeftX
                    If sh.RightX > desenMaxX Then desenMaxX = sh.RightX
                    If sh.BottomY < desenMinY Then desenMinY = sh.BottomY
                    If sh.TopY > desenMaxY Then desenMaxY = sh.TopY
                    hasStones = True
                End If
            End If
        End If
    Next sh
    
    If Not hasStones Then
        desenMinX = ActiveSelectionRange.LeftX
        desenMaxX = ActiveSelectionRange.RightX
        desenMinY = ActiveSelectionRange.BottomY
        desenMaxY = ActiveSelectionRange.TopY
    End If

    ' 2. Find pKalipKare and marker shapes on the active page
    Dim pKalipKare As Shape
    Dim kalipLeft As Double, kalipRight As Double, kalipBottom As Double, kalipTop As Double
    Dim markerShapes As New ShapeRange
    
    Dim maxArea As Double
    maxArea = 0
    
    For Each sh In ActivePage.Shapes.FindShapes()
        nameLower = LCase(sh.Name)
        isCutObject = (InStr(nameLower, "kesim") > 0 Or InStr(nameLower, "kalip") > 0 Or InStr(nameLower, "ztrass") > 0)
        
        If Not isCutObject And sh.Type = cdrRectangleShape Then
            w = sh.SizeWidth
            h = sh.SizeHeight
            If w > 20 And h > 20 Then
                If sh.Fill.Type = 0 Then
                    isCutObject = True
                End If
            End If
        End If
        
        If isCutObject Then
            If sh.LeftX <= desenMinX + 1.5 And sh.RightX >= desenMaxX - 1.5 And _
               sh.BottomY <= desenMinY + 1.5 And sh.TopY >= desenMaxY - 1.5 Then
                Dim area As Double
                area = sh.SizeWidth * sh.SizeHeight
                If area > maxArea Then
                    maxArea = area
                    Set pKalipKare = sh
                    kalipLeft = sh.LeftX
                    kalipRight = sh.RightX
                    kalipBottom = sh.BottomY
                    kalipTop = sh.TopY
                End If
            End If
        End If
    Next sh
    
    If Not pKalipKare Is Nothing Then
        For Each sh In ActivePage.Shapes.FindShapes()
            nameLower = LCase(sh.Name)
            Dim isInsideKalip As Boolean
            isInsideKalip = (sh.LeftX >= kalipLeft - 1.5 And sh.RightX <= kalipRight + 1.5 And _
                             sh.BottomY >= kalipBottom - 1.5 And sh.TopY <= kalipTop + 1.5)
            If isInsideKalip Then
                Dim isMarker As Boolean
                isMarker = (InStr(nameLower, "kalip ztrass.com") > 0 Or InStr(nameLower, "kesim") > 0 Or _
                            (sh.Type = cdrEllipseShape And InStr(nameLower, "ztrass") > 0))
                If isMarker And sh.StaticID <> pKalipKare.StaticID Then
                    markerShapes.Add sh
                End If
            End If
        Next sh
    End If

    ' Add mold square and markers to selection range if missing
    Dim idMap As Object
    Set idMap = CreateObject("Scripting.Dictionary")
    For Each sh In ActiveSelectionRange.Shapes.FindShapes()
        idMap(sh.StaticID) = True
    Next sh
    
    If Not pKalipKare Is Nothing Then
        If Not idMap.Exists(pKalipKare.StaticID) Then
            ActiveSelectionRange.Add pKalipKare
            idMap(pKalipKare.StaticID) = True
        End If
    End If
    
    For Each sh In markerShapes
        If Not idMap.Exists(sh.StaticID) Then
            ActiveSelectionRange.Add sh
            idMap(sh.StaticID) = True
        End If
    Next sh

    Dim sr As ShapeRange, s As Shape
    Set sr = ActiveSelectionRange.Shapes.FindShapes(Query:="@type <> 'text:artistic'").Duplicate
    sr.UngroupAll
    sr.Sort "@shape1.com.sizewidth>@shape2.com.sizewidth"
    
    Dim origH As Double
    origH = ActiveSelectionRange.SizeHeight
    sr.Move 0, -origH - 10

    Dim ust As String
    For i = 1 To 6
        ust = Replace(CStr(i + 1.9), ",", ".")
        For Each s In sr.Shapes.FindShapes(Query:="@type <> 'group' and @width > {" & i & " mm} and @width <= {" & ust & " mm} and @height > {" & i & " mm} and @height <= {" & ust & " mm}")
            nameLower = LCase(s.Name)
            Dim isKalip As Boolean
            isKalip = False
            If Not pKalipKare Is Nothing Then
                If s.StaticID = pKalipKare.StaticID Then isKalip = True
            End If
            If s.Type = cdrRectangleShape And s.Fill.Type = 0 And s.SizeWidth > 20 Then
                isKalip = True
            End If
            
            If Not isKalip And s.SizeWidth < 10 Then
                Dim newSz As Double
                newSz = CDbl(Me.Controls("MakS" & i).Text)
                
                Dim cx As Double, cy As Double
                cx = s.CenterX
                cy = s.CenterY
                
                ' Daireye dönüştürme ve boyutlandırma
                If s.Type <> cdrEllipseShape Then
                    Dim newEllipse As Shape
                    Set newEllipse = ActiveLayer.CreateEllipse2(cx, cy, newSz / 2, newSz / 2)
                    newEllipse.Fill.ApplyUniformFill CreateRGBColor(0, 0, 0)
                    newEllipse.Outline.SetProperties Width:=0.2, Color:=CreateRGBColor(0, 0, 0)
                    s.Delete
                Else
                    s.SetSize newSz, newSz
                End If
            End If
        Next
    Next

    Optimize False, "Makine Kalıbı"
End Sub

Sub btnToggleCompact_Click()
    fraSearch.Visible = Not fraSearch.Visible
    SB
    If fraSearch.Visible Then
        If Month(Date) > 7 Then
            lblBannerText.Caption = "Programı Güncelle. 05327099681"
            Me.Height = btnUpdate.Top + btnUpdate.Height + (fraBanner.Height * 1.4)
            MsgBox "Programın Süresi Doldu, Lütfen Güncelleyin. 05327099681"
            btnUpdate_Click
            Exit Sub
        End If
        
        Me.Width = originalWidth
        Me.Height = originalHeight
        btnToggleCompact.Height = 13
    Else
        'simge oldu
        originalWidth = Me.Width
        originalHeight = Me.Height
        Me.Width = btnToggleCompact.Left + btnToggleCompact.Width + (fraBanner.Width * 0.075)
        Me.Height = fraBanner.Top + fraBanner.Height + (fraBanner.Height * 1.25)
        btnToggleCompact.Height = fraBanner.Height - 1
    End If
    
    fraSize.Visible = fraSearch.Visible
    fraOutline.Visible = fraSearch.Visible
    SaveSetting "SizeFind1", "Preferences", "SizeFind_XY", Me.Top & "_" & Me.Left
End Sub

Private Sub btnUpdate_Click()
    ' Mega.nz linkini ve GMS klasörünü aç
    Shell "cmd /c start https://tubbiya.infinityfreeapp.com", vbHide
    On Error Resume Next
    Shell "explorer.exe """ & Application.GMSManager.UserGMSPath & """", vbNormalFocus
    If Err.Number <> 0 Then MsgBox "GMS klasörü açılamadı.", vbExclamation
End Sub

Sub SB()
    ' Renk değiştir
    fraBanner.BackColor = IIf(fraBanner.BackColor = vbBlack, &H8000000F, vbBlack)
    lblBannerText.ForeColor = IIf(lblBannerText.ForeColor = vbWhite, vbBlack, vbWhite)

    ' İlk defa başlatma
    If IsEmpty(bannerTexts) Then
        bannerTexts = Array( _
            "Hotfix Pro - 0532 709 96 81", _
            "60x60 Büyük Tarayıcı", _
            "Bilgisayarınız Yavaş mı?", _
            "Taş Deseni Tasarımı & Çalışması", _
            "Özel Desinatörlük Eğitimi", _
            "Lazer Makina Kurulumu", _
            "Yapay Zeka Destekli Otomasyon", _
            "Yapay Zeka Sistemleri Kurulumu", _
            "Otomasyon Programları Geliştirme", _
            "Program Geliştirmek İçin Mesaj Atın" _
        )
        bannerTimer = Timer - 2
    End If
    
    ' Süre dolmadıysa çık
    If Timer < bannerTimer Then Exit Sub
    If Timer - bannerTimer < 0 Then Exit Sub ' Gece yarısı için

    ' Mesajı göster
    lblBannerText.Caption = bannerTexts(bannerIndex)
    Me.Caption = "Hotfix Pro  +90532 709 96 81"
    
    ' Sonraki mesaj için index güncelle
    bannerIndex = (bannerIndex + 1) Mod (UBound(bannerTexts) + 1)
    SaveSetting "SizeFind1", "Preferences", "bannerIndex", bannerIndex
    ' Zamanlayıcıyı güncelle
    bannerTimer = Timer + 5
End Sub

Private Sub Label439_Click()
     ActivePage.Shapes.FindShapes(Query:="@type <> 'group' and @fill.type = 'none' and @colors.find('black')").CreateSelection
End Sub

Private Sub lblStoneInfo1_Click(): ccB 1: End Sub
Private Sub lblStoneInfo2_Click(): ccB 2: End Sub
Private Sub lblStoneInfo3_Click(): ccB 3: End Sub
Private Sub lblStoneInfo4_Click(): ccB 4: End Sub
Private Sub lblStoneInfo5_Click(): ccB 5: End Sub
Private Sub lblStoneInfo6_Click(): ccB 6: End Sub
Private Sub lblStoneInfo7_Click(): ccB 7: End Sub
Private Sub lblStoneInfo8_Click(): ccB 8: End Sub
Private Sub lblStoneInfo9_Click(): ccB 9: End Sub
Private Sub lblStoneInfo10_Click(): ccB 10: End Sub

Private Sub ccB(ByRef hedef As Long)
    selectedStoneRange(hedef).CreateSelection
End Sub

Function Yy(ByVal sayi As Double) As Variant
    Yy = CDbl(Format(sayi, "0.#"))
End Function

Private Sub UserForm_MouseMove(ByVal Button As Integer, ByVal shift As Integer, ByVal x As Single, ByVal y As Single)
    mouseMoveCounter = mouseMoveCounter + 1
    If mouseMoveCounter > 20 Then
        SaveSetting "SizeFind1", "Preferences", "SizeFind_XY", Me.Top & "_" & Me.Left
        mouseMoveCounter = 0
    End If
End Sub

Private Sub btnClearSizes_Click(): txtWidth.Text = 0: txtHeight.Text = 0: End Sub
Private Sub txtWidth_Change()
    TemizleVeDuzelt Me.txtWidth
    If txtWidth.Text <> "" Then txtHeight.Text = txtWidth.Text
End Sub

Private Sub txtHeight_Change(): TemizleVeDuzelt Me.txtHeight: End Sub

Private Sub SaveLoadUi(mode As String)
    Dim ctrl As Object, isimler As String
    If mode = "load" Then
        Dim allNames As String: allNames = GetSetting("SizeFind1", "Preferences", "fraSearch", "") & GetSetting("SizeFind1", "Preferences", "ExportF", "")
        For Each ctrl In Me.Controls
            If TypeName(ctrl) = "CheckBox" Then ctrl.Value = (InStr(allNames, ctrl.Name) > 0)
        Next
    Else
        For Each ctrl In Controls(mode).Controls
            If TypeName(ctrl) = "CheckBox" Then
                If ctrl.Value Then isimler = isimler & ctrl.Name & ","
            End If
        Next
        SaveSetting "SizeFind1", "Preferences", mode, isimler
    End If
End Sub

Private Sub chkSize_Click(): btnSizeOp.Caption = "=": End Sub
Private Sub btnSizeOp_Click()
    If chkSize.Value Then btnSizeOp.Caption = IIf(btnSizeOp.Caption = ">", "<", ">")
End Sub

Private Sub UserForm_Initialize()
    ' Orijinal genişlik ve yükseklikleri tasarım anındaki değerler olarak sakla (Simge moduna geçmeden önce)
    originalWidth = Me.Width
    originalHeight = Me.Height

    lblDesigner.Caption = Split(lblDesigner.Caption, ":")(0) & ": " & GetSetting("SizeFind1", "Preferences", "Designer", "")
    
    colorBoxCount = fraList.Controls.Count
    Dim reklamNo As Integer: reklamNo = 80
    
    Dim reklamC As Integer: reklamC = GetSetting("SizeFind1", "Preferences", "reklamC", 0)
    If reklamC >= reklamNo Then
        SaveSetting "SizeFind1", "Preferences", "reklamC", reklamC + 1
    End If

    fraSearch.BackColor = RGB(255, 200, 200): fraSize.BackColor = RGB(200, 200, 255):  ExportF.BackColor = RGB(150, 255, 150)

    Sec3a.List = Array("10", "20", "30", "40", "50", "60", "70", "80", "90"): Sec3a.ListIndex = 0
    
    Dim Cord As Variant: Cord = Split(GetSetting("SizeFind1", "Preferences", "SizeFind_XY", "0_0"), "_")
    SaveLoadUi "load"

    stepWidth = originalWidth * 0.085
    stepHeight = originalHeight * 0.115
    
    fraList.Height = 12
    
    ' Kontrolleri ilk açılışta simge durumuna göre gizle
    fraSearch.Visible = False
    fraSize.Visible = False
    fraOutline.Visible = False
    
    With Me
        .Caption = "Hotfix Pro  +90532 709 96 81"
        .Width = btnToggleCompact.Left + btnToggleCompact.Width + (fraBanner.Width * 0.075)
        .Height = fraBanner.Top + fraBanner.Height + (fraBanner.Height * 1.25)
        .StartUpPosition = 0
        .Top = Cord(0): .Left = Cord(1)
    End With
    lblBannerText.Caption = "Hotfix Pro"
    btnToggleCompact.Height = fraBanner.Height - 1
    
    Dim c As Object
    For Each c In fraCalculate.Controls
        If Left(c.Name, 1) = "S" Then c.Visible = False
    Next
    
    btnCalculate.Top = 6
    btnCalculate.Left = fraCalculate.Left + fraCalculate.Width - btnCalculate.Width - 1
    fraCalculate.Top = btnCalculate.Top + 15
    fraCalculate.Height = 238
    Dim xPs As Integer, i As Integer
    
    tableColumnWidths = Array(35, 25, 25, 30, 35, 45, 45, 30, 25, 70)
    xPs = fraCalculate.Left + 3
    ReDim headerLabels(UBound(tableColumnWidths))
    For i = 0 To UBound(tableColumnWidths)
        Set headerLabels(i) = Me.Controls.Add("Forms.Label.1")
        With headerLabels(i)
            .Caption = Split("Kalıp,En,Boy,Renk,Kal. iç,1Bant(m),Bant(m),Dizim,Tür,Tarih", ",")(i)
            .Left = xPs
            .Top = fraCalculate.Top - 13
            .Width = tableColumnWidths(i)
            .Font.Bold = True
            .AutoSize = True
            .ForeColor = IIf(i Mod 2 = 1, RGB(0, 0, 0), RGB(0, 0, 255))
        End With
        xPs = xPs + tableColumnWidths(i)
    Next
    
    txtKareMM.Text = GetSetting("SizeFind1", "Preferences", "KareMM", "10")
    txtTasMM.Text = GetSetting("SizeFind1", "Preferences", "TasMM", "10")
    bannerIndex = GetSetting("SizeFind1", "Preferences", "bannerIndex", 0)
    If bannerIndex > 0 Then bannerIndex = bannerIndex - 1
    bannerIndex = 0
    
    Dim val As String
    For i = 1 To 6
        val = GetSetting("SizeFind1", "Preferences", "MakS" & i, "")
        If val = "" Then val = Array("0,55", "0,8", "1,2", "1,5", "2", "2")(i - 1)
        Me.Controls("MakS" & i).Text = val
    Next i
End Sub

Sub Find()
    btnSearch_Click
End Sub

Private Sub btnSearch_Click()
    ActiveDocument.Unit = cdrMillimeter
    
    Dim x As Double, y As Double
    Dim q As String, s As Shape, sr As New ShapeRange, SrC As New ShapeRange
    Set s = ActiveShape
    If s Is Nothing Then MsgBox "Lütfen bir obje seçin.": Exit Sub

    If ActiveSelectionRange.Count = 1 Then
        
    Else
        Set SrC = ActiveSelectionRange
        If ActiveDocument.GetUserClick(x, y, 0, -1, Snap:=False, CursorShape:=cdrCursorPickOvertarget) Then Exit Sub
        With ActivePage.SelectShapesAtPoint(x, y, False)
           If .Shapes.Count = 0 Then Beep: Exit Sub
        End With
        Set s = ActiveShape
    End If

    Optimize True, "Ztrass Benzerleri Bul"
    ' İç Renk Kontrolü
    If chkFillColor.Value Then
        If s.Fill.Type = cdrUniformFill Then
            q = "(@fill.color = '" & s.Fill.UniformColor.ToString & "')"
        ElseIf s.Fill.Type = cdrNoFill Then
            q = "(@fill.type = 'none')"
        End If
    End If
    
    ' Kontur Renk Kontrolü
    If chkOutlineColor.Value Then
        If s.Outline.Type = cdrOutline Then
            If q <> "" Then q = q & " and "
            q = q & "(@outline.color = '" & s.Outline.Color.ToString & "')"
        ElseIf s.Outline.Type = cdrNoOutline Then
            If q <> "" Then q = q & " and "
            q = q & "(@outline.type = 'none')"
        End If
    End If
    
    ' Kontur Kalınlığı (Width)
    If chkOutlineWidth.Value Then
        If s.Outline.Type = cdrOutline Then
            If q <> "" Then q = q & " and "
            q = q & "(@outline.width = {" & Replace(s.Outline.Width, ",", ".") & " mm})"
        ElseIf s.Outline.Type = cdrNoOutline Then
            If q <> "" Then q = q & " and "
            q = q & "(@outline.type = 'none')"
        End If
    End If

    ' Genişlik Kontrolü
    Dim op As String, sSize As String
    Select Case btnSizeOp.Caption
        Case "="
            op = "(@width > {" & Replace(s.SizeWidth - 0.06, ",", ".") & " mm} and @width < {" & Replace(s.SizeWidth + 0.06, ",", ".") & " mm})"
        Case ">"
            sSize = Replace(s.SizeWidth - 0.01, ",", ".")
            op = "(@width >= {" & sSize & " mm})"
        Case "<"
            sSize = Replace(s.SizeWidth + 0.01, ",", ".")
            op = "(@width <= {" & sSize & " mm})"
    End Select

    If chkSize.Value Then q = q & IIf(q <> "", " and ", "") & op

    If Not chkType.Value And q = "" Then
        MsgBox "Lütfen en az bir kriter seçin."
        Optimize False, "Ztrass Benzerleri Bul"
        Exit Sub
    End If
    
    If ActiveSelectionRange.Count = 1 Then
        With ActivePage.Shapes.All
            If Not chkInsideGroup.Value Then .RemoveRange .FindAnyOfType(cdrGroupShape)
            .Shapes.FindShapes(Query:=q, Type:=IIf(chkType.Value, s.Type, cdrNoShape)).CreateSelection
        End With
    Else
        With SrC
            .Shapes.FindShapes(Query:=q, Type:=IIf(chkType.Value, s.Type, cdrNoShape)).CreateSelection
        End With
    End If
    SaveLoadUi "fraSearch" 'Save
    Optimize False, "Ztrass Benzerleri Bul"
End Sub

Private Sub SizeFF_Click(): ResizeShapes CDbl(txtWidth.Text), CDbl(txtHeight.Text): End Sub
Private Sub S1_Click(): ResizeShapes 2, 2: End Sub
Private Sub S2_Click(): ResizeShapes 2.1, 2.1: End Sub
Private Sub S3_Click(): ResizeShapes 2.2, 2.2: End Sub
Private Sub S4_Click(): ResizeShapes 2.3, 2.3: End Sub
Private Sub S5_Click(): ResizeShapes 3, 3: End Sub
Private Sub S6_Click(): ResizeShapes 3.1, 3.1: End Sub
Private Sub S7_Click(): ResizeShapes 3.2, 3.2: End Sub
Private Sub S8_Click(): ResizeShapes 3.3, 3.3: End Sub

Private Function ResizeShapes(ByVal w As Double, ByVal h As Double)
    If ActiveShape Is Nothing Then MsgBox "Lütfen bir obje seçin.": Exit Function
    Optimize True, "Seçilileri Boyutlandır"
    ActiveDocument.Unit = cdrMillimeter
    ActiveDocument.ReferencePoint = cdrCenter
    Dim s As Shape, sCr As Shape, sr As ShapeRange, VSR As New ShapeRange, x As Double, y As Double, N As Double, ww As Double, hh As Double
    Dim sL1 As Double, sL2 As Double

    Dim x1 As Double, y1 As Double, x2 As Double, y2 As Double
    Dim origx As Double, origy As Double, origw As Double, origh As Double
    
    Dim isCircleOnly As Boolean
    isCircleOnly = False
    On Error Resume Next
    isCircleOnly = chkCircleOnly.Value
    On Error GoTo 0
    
    If isCircleOnly Then h = w
    Set sr = ActiveSelection.Shapes.FindShapes()
    
    Set sCr = ActiveLayer.CreateEllipse2(0, 0, 1)

    For Each s In sr
        If isCircleOnly Then 'daireyi yeniden olustur
            s.GetPosition x, y
            With sCr.TreeNode.GetCopy
                With .VirtualShape
                    .Fill.UniformColor.CopyAssign s.Fill.UniformColor
                    .Outline.SetProperties s.Outline.Width, , s.Outline.Color
                    .RotationAngle = s.RotationAngle
                    .SetPosition x, y
                    .SetSize w, h
                End With
                .LinkAsChildOf s.Layer.TreeNode
                VSR.Add .VirtualShape
            End With
        Else
            Dim a As Double
 
            If s.Type = cdrEllipseShape Then
                s.SetSize w, h
                
            ElseIf s.Type = cdrRectangleShape Then
                N = s.RotationAngle
                If N = 0 Then
                    s.SetSize w, h
                Else
                    s.Rotate -N
                    s.SetSize w, h
                    s.Rotate N
                End If
 
            ElseIf s.Type = cdrCurveShape Then 'serbest çizimse
                sL1 = s.Curve.Nodes(1).Segment.Length
                sL2 = s.Curve.Nodes(2).Segment.Length
                
                If Abs(sL1 - Sqr((s.Curve.Nodes(1).Segment.EndNode.PositionX - s.Curve.Nodes(1).Segment.StartNode.PositionX) ^ 2 + (s.Curve.Nodes(1).Segment.EndNode.PositionY - s.Curve.Nodes(1).Segment.StartNode.PositionY) ^ 2)) < 0.01 Then
                    s.Curve.Nodes(1).GetPosition x1, y1
                    s.Curve.Nodes(2).GetPosition x2, y2
                    a = IIf(x2 = x1, -90, -Atn((y2 - y1) / (x2 - x1)) * 180 / 3.14159265358979)
                    
                    s.Rotate a
                    s.SetSize w, h
                    s.Rotate -a
                
                ElseIf Abs(sL1 - sL2) < 0.1 Then
                    s.SetSize w, h
                    
                Else
                    s.SetSize w, h
                    
                End If
            End If
        End If
    Next
    Optimize False, "Seçilileri Boyutlandır"
    sCr.Delete
    If isCircleOnly Then
        sr.Delete
        ActiveDocument.LogCreateShapeRange VSR
    End If
    
    Application.ActiveWindow.Activate
End Function

Private Sub btnExportLazer_Click()
    Dim FileName As String, defaultPath As String, fileExt As String
    Dim filter As String
    
    If ActiveShape Is Nothing Then MsgBox "Lütfen bir obje seçin.": Exit Sub
    
    defaultPath = GetSetting("SizeFind1", "Preferences", "SavePath2", Environ("USERPROFILE") & "\Desktop\.plt")
    
    If LCase(Right(defaultPath, 4)) = ".plt" Then
        filter = "Plt Lazer Files (*.plt)|*.plt|Dxf Lazer Files (*.dxf)|*.dxf"
        defaultPath = Left(defaultPath, InStrRev(defaultPath, "\"))
    Else
        filter = "Dxf Lazer Files (*.dxf)|*.dxf|Plt Lazer Files (*.plt)|*.plt"
        defaultPath = Left(defaultPath, InStrRev(defaultPath, "\"))
    End If
    
    FileName = CorelScriptTools.GetFileBox(filter, "Dosya Kaydet", 1, defaultPath & ModelFirma)
    If FileName = "" Then Exit Sub
    
    SaveLoadUi "ExportF" 'Save
    SaveSetting "SizeFind1", "Preferences", "SavePath2", FileName
    fileExt = LCase(Mid(FileName, InStrRev(FileName, ".") + 1))

      Dim ayirAktif As Boolean
      ayirAktif = False
      Dim ctrl As Object
      Set ctrl = Nothing
      
      On Error Resume Next
      Set ctrl = Me.Controls("chkExportSplit")
      On Error GoTo 0
      
      If ctrl Is Nothing Then
          Dim c As Object, listMsg As String
          listMsg = ""
          On Error Resume Next
          For Each c In Me.Controls
              If InStr(LCase(c.Name), "ayir") > 0 Or InStr(LCase(c.Name), "ayır") > 0 Then
                  listMsg = listMsg & c.Name & " (" & TypeName(c) & "=" & c.Value & "), "
              End If
          Next c
          On Error GoTo 0
          MsgBox "chkExportSplit checkbox'ı formda bulunamadı! Benzer isimli kontroller: " & listMsg
      Else
          ayirAktif = CBool(ctrl.Value)
      End If
      
      Optimize True, "ExportLazer"
     
    Dim baseSel As ShapeRange
    Set baseSel = ActiveSelectionRange
    
    If Not ayirAktif Then
        ExportSingleFile baseSel, FileName, fileExt
    Else
        Dim ortakSekiller As New ShapeRange
        Dim enDistakiKutu As Shape
        Dim s As Shape
        Dim maxW As Double: maxW = 0
        Dim isBlackOutline As Boolean
        Dim isBlackFill As Boolean
        Dim isNoFill As Boolean
        
        For Each s In baseSel
            isBlackOutline = False
            isBlackFill = False
            isNoFill = True
            
            On Error Resume Next
            If s.Outline.Color.RGBRed < 15 And s.Outline.Color.RGBGreen < 15 And s.Outline.Color.RGBBlue < 15 Then
                isBlackOutline = True
            End If
            If s.Fill.Type = cdrUniformFill Then
                isNoFill = False
                If s.Fill.UniformColor.RGBRed < 15 And s.Fill.UniformColor.RGBGreen < 15 And s.Fill.UniformColor.RGBBlue < 15 Then
                    isBlackFill = True
                End If
            End If
            On Error GoTo 0
            
            If isBlackOutline And (isBlackFill Or isNoFill) Then
                ortakSekiller.Add s
            ElseIf (Not isBlackOutline) And isBlackFill Then
                ortakSekiller.Add s
            End If
            
            If s.SizeWidth > maxW Then
                maxW = s.SizeWidth
                Set enDistakiKutu = s
            End If
        Next s
        
        If Not enDistakiKutu Is Nothing Then
            If Not ShapeRangeContains(ortakSekiller, enDistakiKutu) Then
                ortakSekiller.Add enDistakiKutu
            End If
        End If
        
        Dim renkliSekiller As New ShapeRange
        For Each s In baseSel
            If Not ShapeRangeContains(ortakSekiller, s) Then
                renkliSekiller.Add s
            End If
        Next s
        
        Dim renkSözlüğü As Object
        Set renkSözlüğü = CreateObject("Scripting.Dictionary")
        Dim renkHex As String
        
        For Each s In renkliSekiller
            renkHex = "#000000"
            On Error Resume Next
            If s.Fill.Type = cdrUniformFill Then
                renkHex = s.Fill.UniformColor.HexValue
            End If
            If renkHex = "#000000" Or s.Fill.Type <> cdrUniformFill Then
                If Not s.Outline Is Nothing Then
                    renkHex = s.Outline.Color.HexValue
                End If
            End If
            On Error GoTo 0
            
            If renkHex <> "#000000" Then
                If Not renkSözlüğü.Exists(renkHex) Then
                    renkSözlüğü.Add renkHex, New ShapeRange
                End If
                renkSözlüğü(renkHex).Add s
            End If
        Next s
        
        Dim keys As Variant
        keys = renkSözlüğü.keys
        Dim i As Integer
        Dim baseDir As String, baseNameOnly As String
        baseDir = Left(FileName, InStrRev(FileName, "\"))
        baseNameOnly = Mid(FileName, InStrRev(FileName, "\") + 1)
        baseNameOnly = Left(baseNameOnly, InStrRev(baseNameOnly, ".") - 1)
        
        If renkSözlüğü.Count = 0 Then
            ExportSingleFile baseSel, FileName, fileExt
        Else
            Dim exportRange As ShapeRange
            Dim outPath As String
            For i = LBound(keys) To UBound(keys)
                Set exportRange = New ShapeRange
                Dim renkGrubu As ShapeRange
                Set renkGrubu = renkSözlüğü(keys(i))
                Dim j As Long
                For j = 1 To renkGrubu.Count
                    exportRange.Add renkGrubu(j)
                Next j
                For j = 1 To ortakSekiller.Count
                    exportRange.Add ortakSekiller(j)
                Next j
                
                ActiveDocument.ClearSelection
                exportRange.CreateSelection
                
                outPath = baseDir & baseNameOnly & "_" & Replace(keys(i), "#", "") & "." & fileExt
                ExportSingleFile exportRange, outPath, fileExt
            Next i
        End If
    End If
    
    baseSel.CreateSelection
    Optimize False, "ExportLazer"

    If chkExportJpg Then
        FileName = Left(FileName, InStrRev(FileName, ".")) & "jpg"
        Dim expopt2 As StructExportOptions, expflt2 As ExportFilter
        Set expopt2 = CreateStructExportOptions: expopt2.UseColorProfile = False
        Set expflt2 = ActiveDocument.ExportBitmap(FileName, cdrJPEG, cdrSelection, cdrRGBColorImage, 0, 0, 150, 150, cdrNormalAntiAliasing, False, False, False, False, cdrCompressionZIP)
        With expflt2
            .Compression = 20
            .Optimized = True
            .Smoothing = 0
            .SubFormat = 1
            .Progressive = True
            .Finish
        End With
    End If
End Sub

Private Sub ExportSingleFile(ByVal sr As ShapeRange, ByVal path As String, ByVal ext As String)
    Dim expopt As StructExportOptions, expflt As ExportFilter
    Set expopt = CreateStructExportOptions: expopt.UseColorProfile = False
    
    Select Case ext
        Case "plt"
            Set expflt = ActiveDocument.ExportEx(path, cdrHPGL, cdrSelection, expopt)
            With expflt
                .PenLibIndex = 0
                .FitToPage = False
                .ScaleFactor = 100
                .PageWidth = 8.5
                .PageHeight = 11
                .FillType = 0
                .FillSpacing = 0.005
                .FillAngle = 0
                .HatchAngle = 90
                .CurveResolution = 0.0003
                .RemoveHiddenLines = False
                .AutomaticWeld = False
                .ExcludeWVC = False
                .PlotterUnits = 1008
                .PlotterOrigin = 0
                .Finish
            End With
        Case "dxf"
            Set expflt = ActiveDocument.ExportEx(path, cdrDXF, cdrSelection, expopt)
            With expflt
                .BitmapType = 0
                .TextAsCurves = True
                .Version = 1
                .Units = 3
                .FillUnmapped = True
                .FillColor = 0
                .Finish
            End With
    End Select
End Sub

Sub btnSave12_Click()
    If ActiveDocument Is Nothing Then Exit Sub
    If ActiveDocument.FilePath = "" Then Exit Sub
    
    Dim s As StructSaveAsOptions
    Set s = CreateStructSaveAsOptions
    With s
     .EmbedVBAProject = False
     .filter = cdrCDR
     .IncludeCMXData = False
     .Range = cdrAllPages
     .EmbedICCProfile = False
     .Version = cdrVersion12
    End With
    ActiveDocument.SaveAs ActiveDocument.FullFileName, s
End Sub

Sub KalipKare_Click(): MainS "KalipKare": End Sub
Sub KalipTas_Click(): MainS "KalipTas": End Sub

Private Sub Aci_Click(): MainS "Aci": End Sub
Private Sub itok_Click(): MainS "ik": End Sub
Private Sub ktoi_Click(): MainS "ki": End Sub
Private Sub Ayir_Click(): MainS "Ayir": End Sub
Private Sub resim_Click(): MainS "Resim": End Sub

Private Sub Sec1_Click(): MainS "SecCizimAlti": End Sub
Private Sub Sec2_Click(): MainS "SecKonturAlti": End Sub
Private Sub Sec3_Click(): MainS "SecYuzde": End Sub

Private Sub Listele_Click(): Kalip "Listele": End Sub
Private Sub KalipAyir_Click(): Kalip "KalipAyir": End Sub

Private Sub KalipBilgiEkle_Click(): MainS "KalipBilgiEkle": End Sub
Private Sub DenYakinB_Click(): MainS "DenYakin": End Sub

Public Sub MainS(ByVal secim As String)
    If ActiveShape Is Nothing Then Exit Sub
    On Error Resume Next
    
    Dim undoName As String
    Select Case secim
        Case "ik": undoName = "Dolgu Rengini Çizgiye Kopyala"
        Case "ki": undoName = "Çizgi Rengini Dolguya Kopyala"
        Case "SecKonturAlti": undoName = "Kontur Altını Seç"
        Case "SecCizimAlti": undoName = "Çizim Altını Seç"
        Case "KalipBilgiEkle": undoName = "Kalıba Bilgi Ekle"
        Case "DenYakin": undoName = "En Yakınları Seç"
        Case "KalipKare": undoName = "Kalıp Karesi Ekle"
        Case "KalipTas": undoName = "İşaret Taşı Ekle"
        Case "Aci": undoName = "Açı Düzelt"
        Case "Ayir": undoName = "Grupları Ayır"
        Case "Resim": undoName = "Resim Optimizasyonu"
        Case "SecYuzde": undoName = "Yüzdeye Göre Seç"
        Case Else: undoName = "Ztrass İşlem"
    End Select
    
    ActiveDocument.Unit = cdrMillimeter
    Optimize True, undoName
    Dim iSr As New ShapeRange
    Set iSr = ActiveSelectionRange
    iSr.RemoveRange iSr.FindAnyOfType(cdrBlendGroupShape, cdrGroupShape, cdrGuidelineShape, cdrBitmapShape)
    Select Case secim
        Case "ik"
            RenkKopyala iSr, True
        Case "ki"
            RenkKopyala iSr, False
            
        Case "SecKonturAlti"
            SecXM iSr, "kontur"
            
        Case "SecCizimAlti"
            SecXM iSr, "ic"
            
        Case "KalipBilgiEkle"
            If srKalip.Count = 0 Then
                Optimize False, undoName
                Exit Sub
            End If
            CallByName Me, secim & "M", VbMethod, srKalip, SrG
        Case Else
             CallByName Me, secim & "M", VbMethod, iSr
    End Select

    Optimize False, undoName
    Exit Sub
ErrHandler:
    Optimize False, undoName
    MsgBox "Hata oluştu: " & Err.Description
End Sub

Sub KalipKareM(ByVal sr As ShapeRange)
    Dim kare As Shape, padding As Double, altY As Double
    padding = val(txtKareMM.Text)

    Set kare = ActiveLayer.CreateRectangle(sr.LeftX - padding, sr.BottomY - padding, sr.RightX + padding, sr.TopY + padding)
    altY = kare.BottomY

    With kare
        .Fill.ApplyNoFill
        .Outline.Color.CMYKAssign 0, 0, 0, 100
        .OrderToBack
        .Name = "kalip ztrass.com"
        .CreateSelection

        Select Case .SizeHeight
            Case Is < 240: .SizeHeight = 240
            Case Is < 320: .SizeHeight = 320
            Case Is < 400: .SizeHeight = 400
        End Select

        .BottomY = altY
    End With

    SaveSetting "SizeFind1", "Preferences", "KareMM", txtKareMM.Text
End Sub

Sub KalipTasM(ByVal sr As ShapeRange)
    Set sr = ActivePage.SelectShapesFromRectangle(sr.LeftX - 5, sr.BottomY - 5, sr.RightX + 5, sr.TopY + 5, True).Shapes.FindShapes.All
    Dim D1 As Shape, D2 As Shape, Sw As Double
    Dim kare As Shape: Set kare = sr.Shapes.FindShapes(Query:="@type <> 'group' and @width > {" & CInt(sr.SizeWidth - 5) & " mm}").FirstShape
    If kare Is Nothing Then Exit Sub
    
    sr.Sort "@shape1.com.sizewidth<@shape2.com.sizewidth"
    If sr.Shapes.FindShapes(Query:="@type <> 'group' and @width < {15 mm}").Count = 0 Then Exit Sub
    Sw = sr.Shapes.FindShapes(Query:="@type <> 'group' and @width < {15 mm}").FirstShape.SizeWidth / 2
    Set D1 = ActiveLayer.CreateEllipse2(sr.LeftX + CDbl(txtTasMM.Text), sr.BottomY + CDbl(txtTasMM.Text), Sw)
    Set D2 = ActiveLayer.CreateEllipse2(sr.RightX - CDbl(txtTasMM.Text), sr.BottomY + CDbl(txtTasMM.Text), Sw)
    D1.Name = "kalip ztrass.com"
    D2.Name = "kalip ztrass.com"
    D1.Fill.ApplyNoFill
    D2.Fill.ApplyNoFill
    D1.Outline.Color.CMYKAssign 0, 0, 0, 100
    D2.Outline.Color.CMYKAssign 0, 0, 0, 100
    SaveSetting "SizeFind1", "Preferences", "TasMM", txtTasMM.Text
End Sub

Sub AciM(ByVal sr As ShapeRange)
    Dim x1 As Double, y1 As Double, x2 As Double, y2 As Double, a As Double, Aci As Double
    On Error Resume Next
    
    sr.FirstShape.Curve.Nodes.First.GetPosition x1, y1
    sr.FirstShape.Curve.Nodes.Last.GetPosition x2, y2
    a = -IIf(x2 = x1, 90, Atn((y2 - y1) / (x2 - x1)) * 180 / 3.14159265)
    Aci = InputBox("Açı Değeri", "Açı Değeri", a)
    On Error GoTo 0
    
    Dim x As Double, y As Double, w As Double, h As Double
    sr.FirstShape.GetBoundingBox x, y, w, h
    ActivePage.SelectShapesFromRectangle(x, y + h, x + w, y, True).Shapes.FindShapes.All.Rotate a
    sr.CreateSelection
End Sub

Sub RenkKopyala(ByVal sr As ShapeRange, ByVal fillToOutline As Boolean)
   Dim sr2 As ShapeRange, col As New Color
   
   If fillToOutline Then
       Set sr = sr.Shapes.FindShapes(Query:="@fill.type = 'uniform'")
       Do While sr.Count > 0
           Set sr2 = sr.Shapes.FindShapes(Query:="(@fill.color = '" & sr.FirstShape.Fill.UniformColor.ToString & "')")
           sr.RemoveRange sr2
           col.CopyAssign sr2.FirstShape.Fill.UniformColor
           sr2.SetOutlineProperties , , col
       Loop
   Else
       Set sr = sr.Shapes.FindShapes(Query:="@outline.color <> null")
       Do While sr.Count > 0
           Set sr2 = sr.Shapes.FindShapes(Query:="(@outline.color = '" & sr.FirstShape.Outline.Color.ToString & "')")
           sr.RemoveRange sr2
           col.CopyAssign sr2.FirstShape.Outline.Color
           sr2.ApplyUniformFill col
       Loop
   End If
End Sub

Sub AyirM(ByVal sr As ShapeRange)
    Dim s As Shape, toplam As Long
    Set sr = ActiveSelectionRange
    Do
        toplam = sr.Count
        sr.UngroupAll
        sr.BreakApart
        sr.UngroupAll
        sr.BreakApart
    Loop While sr.Count <> toplam
End Sub

Sub resimM(ByVal sr As ShapeRange)
    Dim s As Shape
    Set sr = ActiveSelectionRange.Shapes.FindShapes(, cdrBitmapShape)
    For Each s In sr
        s.Style.StringAssign "{""fill"":{""type"":""0"",""overprint"":""0"",""winding"":""0""},""outline"":{""overprint"":""0"",""screenSpec"":""0,0,45000000,60,0"",""angle"":""0"",""dashDotSpec"":""0"",""leftArrow"":""|0"",""shareArrow"":""0"",""rightArrow"":""|0"",""justification"":""0"",""rightArrowAttributes"":""0|0|0|0|0|0|0"",""overlapArrow"":""0"",""scaleWithObject"":""0"",""width"":""0"",""color"":""RGB255,USER,0,0,0,100,00000000-0000-0000-0000-000000000000"",""miterLimit"":""5"",""aspect"":""100"",""matrix"":""1,0,0,0,1,0"",""behindFill"":""0"",""endCaps"":""0"",""leftArrowAttributes"":""0|0|0|0|0|0|0"",""dotLength"":""0""},""transparency"":{""mode"":""12""}}"
    Next
End Sub

Private Sub SecXM(ByVal sr As ShapeRange, kontrolTipi As String)
    Dim s As Shape, sF As Shape, sonuc As New ShapeRange
    
    If sr.FirstShape.Type <= 6 Or sr.FirstShape.Type = 26 Then
        Set s = sr.CustomCommand("Boundary", "CreateBoundary")
    ElseIf sr.FirstShape.Curve.Closed Then
        Set s = sr.CustomCommand("Boundary", "CreateBoundary")
    Else
        If kontrolTipi = "ic" Then Exit Sub
        Set s = sr.FirstShape
    End If
    
    Set sr = ActivePage.SelectShapesFromRectangle(sr.LeftX, sr.TopY, sr.RightX, sr.BottomY, True).Shapes.FindShapes(Query:="@width < {20 mm}").All
    
    sr.RemoveRange sr.FindAnyOfType(cdrGroupShape, cdrGuidelineShape, cdrBlendGroupShape, cdrContourGroupShape)
    For Each sF In sr
        If sF.StaticID <> s.StaticID Then
            Select Case kontrolTipi
                Case "kontur"
                    If sF.DisplayCurve.IntersectsWith(s.DisplayCurve) Then sonuc.Add sF
                Case "ic"
                    If s.Curve.IsPointInside(sF.PositionX, sF.PositionY) Then sonuc.Add sF
            End Select
        End If
    Next sF
    If s.Curve.Closed Then s.Delete
    sonuc.CreateSelection
End Sub

Sub SecYuzdeM(ByVal sr As ShapeRange)
   Dim srSelected As New ShapeRange, s As Shape
   Dim selectCount As Integer

   Set sr = ActiveSelectionRange.Shapes.FindShapes(Query:="@type <> 'group'").All
   If sr.Count = 0 Then MsgBox "Lütfen bir şekil seçin.", vbOKOnly: Exit Sub
   If Not IsNumeric(Sec3a.Text) Or Sec3a.Text = "" Then MsgBox "Lütfen geçerli bir yüzde değeri girin.", vbOKOnly: Exit Sub
   
   selectCount = Round(sr.Count * CDbl(Sec3a.Text) * 0.01)
   If selectCount = 0 Then selectCount = 1
   
   Randomize
   Do While srSelected.Count < selectCount
        Set s = sr.Shapes(Int(Rnd * sr.Count) + 1)
        If srSelected.Shapes.FindShape(s.StaticID) Is Nothing Then srSelected.Add s
   Loop
   
   ActiveDocument.ClearSelection
   srSelected.CreateSelection
End Sub

Sub Kalip(ByVal secc As String)
    If ActiveShape Is Nothing Then Exit Sub
    On Error Resume Next
    ActiveDocument.Unit = cdrMillimeter
    Optimize True, secc
    
    Dim c As Object
    Dim i As Integer, j As Integer, toplam As Integer, sRenk As Integer
    Dim r As Integer, g As Integer, b As Integer
    Dim hexColor As String

    Dim s As Shape
    Dim sr As ShapeRange, srT As ShapeRange, SrC As New ShapeRange, SrD As Shape, SrD2 As New ShapeRange, SrF As New ShapeRange
    Dim metin As String
    Dim parca As Variant
    Dim satir As Variant
    Dim RST As Variant, RST2 As Variant, RSN As Variant
    
    Set SrF = ActiveSelectionRange
    
    Set sr = ActivePage.SelectShapesFromRectangle( _
                    ActiveSelectionRange.LeftX, ActiveSelectionRange.TopY, _
                    ActiveSelectionRange.RightX, ActiveSelectionRange.BottomY, True _
                 ).Shapes.All
    
    sr.UngroupAll
    sr.AddRange ActivePage.SelectShapesFromRectangle( _
                sr.LeftX, sr.TopY + 5, _
                sr.RightX, sr.BottomY, True _
             ).Shapes.All
            
    sr.UngroupAll
    sr.AddRange ActivePage.SelectShapesFromRectangle( _
                sr.LeftX, sr.TopY, _
                sr.RightX, sr.BottomY, True _
             ).Shapes.All
             
    sr.UngroupAll

    Set srText = New ShapeRange
    For Each s In sr.Shapes.FindShapes(Query:="@type = 'text:artistic'")
        If s.Text.Story.Size = 8.01 Then
            srText.Add s
            If s.SizeHeight > 5 Then metin = s.Text.Story
        End If
    Next
    
    sr.RemoveRange srText
         
    metin = Replace(Replace(metin, vbCrLf, vbLf), vbCr, vbLf)
    For Each satir In Split(metin, vbLf)
        If InStr(satir, ":") > 0 Then
            parca = Split(satir, ":")
            If UBound(parca) >= 1 Then
                Select Case True
                    Case InStr(LCase(parca(0)), "firma"):     txtFirma.Text = Trim(parca(1))
                    Case InStr(LCase(parca(0)), "dosya"):     txtDosya.Text = Trim(parca(1))
                    Case InStr(LCase(parca(0)), "kalıp adı"): txtKalip.Text = Trim(parca(1))
                    Case InStr(LCase(parca(0)), "pres"):      txtPres.Text = Trim(parca(1))
                    Case InStr(LCase(parca(0)), "montaj"):    txtMontaj.Text = Trim(parca(1))
                    Case InStr(LCase(parca(0)), "kalıptaki"): txtKalipAdet.Text = Trim(parca(1))
                    Case InStr(LCase(parca(0)), "toplam"):    txtToplamAdet.Text = Trim(parca(1))
                End Select
            End If
        End If
    Next

    Do
        toplam = sr.Count
        sr.UngroupAll
        sr.BreakApart
    Loop While sr.Count <> toplam

    Set srKalip = sr.Shapes.FindShapes(Name:="kalip ztrass.com")
    srKalip.AddRange sr.Shapes.FindShapes(Name:="Kesim")
    srKalip.AddRange sr.Shapes.FindShapes(Query:="@width > {" & CInt(ActiveSelectionRange.SizeWidth - 5) & " mm}")
    srKalip.AddRange sr.Shapes.FindShapes(Query:= _
        "@fill.type = 'none' and " & _
        "(@outline.color.cmyk.k > 90 or " & _
        "(@outline.color.rgb.r < 10 and @outline.color.rgb.g < 10 and @outline.color.rgb.b < 10))" _
    )

    sr.RemoveRange srKalip
    Set SrG = New ShapeRange

    RSN = Array("Crystal", "Amethyst", "Black Diamond", "Hyacinth", "Aquamarine", "Sapphire", "Peridot", "Light Sapphire", "Sm. Topaz", "Pink", "Light Siam", "Rainbow", "Jet Black", "Light Amethyst", "Emerald", "Jonquil", "Siam", "Topaz", "CobaLt", "Hematite", "Gold Hematite", "Ab Crystal", "Olive", "Rose Pink", "Fuchsia", "Blue Hematite", "Light Brown", "Rose Pink", "Cosmojet", "Light Topaz", "Montana", "Violet", "Light Peach", "Light Colorado Topaz", "Silver Hematite", "Citrine", "Blue Zircon", "Aquamarine Ab", "Topaz Ab", "Jet Ab", "Sapphire Ab", "Siam Ab", "Hyacinth Ab", "Peridot Ab", "Nude", "Light Black Diamond")
    RST = Array("Dbl", "Mc", "Rzn", "Oct", "Drp", "Düz", "Özl")
    RST2 = Array("Double", "Mc", "Rezene", "Octagon", "Drop", "Düz Metal", "Özel")

    If secc = "Listele" Then
        For Each c In fraList.Controls
            c.Visible = False
        Next c
    End If

    Do While sr.Count > 0
        Dim isNoFill As Boolean
        isNoFill = True
        On Error Resume Next
        If sr.FirstShape.Fill.Type = cdrUniformFill Then isNoFill = False
        On Error GoTo 0
        
        If isNoFill Then
            Set SrC = sr.Shapes.FindShapes(Query:="@outline.color = '" & sr.FirstShape.Outline.Color.ToString & "'")
            hexColor = sr.FirstShape.Outline.Color.HexValue
            If hexColor = "" Then hexColor = "000000"
        Else
            Set SrC = sr.Shapes.FindShapes(Query:="@fill.color = '" & sr.FirstShape.Fill.UniformColor.ToString & "'")
            hexColor = sr.FirstShape.Fill.UniformColor.HexValue
            If hexColor = "" Then hexColor = "000000"
        End If
        
        sr.RemoveRange SrC
        If secc = "Listele" Then
            chkDetailList.Enabled = True
            hexColor = Replace(hexColor, "#", "")
            r = val("&H" & Mid(hexColor, 1, 2))
            g = val("&H" & Mid(hexColor, 3, 2))
            b = val("&H" & Mid(hexColor, 5, 2))
    
            Dim cbName As MSForms.ComboBox
            Dim cbType As MSForms.ComboBox
            Dim idx As Integer
    
            idx = SrG.Count + 1
            With Me.Controls("lblStoneInfo" & idx)
                .Caption = " " & Format(SrC.FirstShape.SizeWidth, "0.#") & "mm-" & SrC.Count
                .Visible = True
                .BackColor = RGB(r, g, b)
                .ForeColor = IIf(0.2126 * r + 0.7152 * g + 0.0722 * b > 100, vbBlack, vbWhite)
                .TextAlign = fmTextAlignLeft
                .BorderStyle = fmBorderStyleSingle
            End With
            
            Dim sText As Variant
            sText = Empty
            
            For Each s In srText
                If s.Fill.UniformColor.HexValue = "#" & hexColor Then
                    sText = Split(s.Text.Story, "-")
                End If
            Next

            Set cbName = fraList.Controls.Add("Forms.ComboBox.1", "cbStoneName" & idx, True)
            With cbName
                For j = 0 To UBound(RSN)
                    .AddItem RSN(j)
                Next j
                .Text = ""
                .Visible = False
            End With
                
            Set cbType = fraList.Controls.Add("Forms.ComboBox.1", "cbStoneType" & idx, True)
            With cbType
                For j = 0 To UBound(RST)
                    .AddItem RST(j)
                Next j
                .Visible = False
                .ListIndex = 0
            End With

            If IsEmpty(sText) Then
                Dim defName As String, defType As String
                defName = ""
                defType = "Dbl"
                
                If r = 255 And g = 153 And b = 204 Then ' Rose Pink
                    defName = "Rose Pink"
                    defType = "Mc"
                ElseIf r = 220 And g = 20 And b = 60 Then ' Siam Ab
                    defName = "Siam Ab"
                    defType = "Rzn"
                ElseIf r = 224 And g = 255 And b = 255 Then ' Ab Crystal
                    defName = "Ab Crystal"
                    defType = "Oct"
                ElseIf r = 165 And g = 42 And b = 42 Then ' Light Brown
                    defName = "Light Brown"
                    defType = "Drp"
                ElseIf (r = 30 And g = 30 And b = 30) Or (r = 0 And g = 0 And b = 0) Then ' Jet Black
                    defName = "Jet Black"
                    defType = "Düz"
                ElseIf r = 255 And g = 140 And b = 0 Then ' Rainbow
                    defName = "Rainbow"
                    defType = "Dbl"
                ElseIf (r = 245 And g = 245 And b = 245) Or (r = 255 And g = 255 And b = 255) Then ' Crystal
                    defName = "Crystal"
                    defType = "Özl"
                End If
                
                fraList.Controls("cbStoneName" & idx).Text = defName
                fraList.Controls("cbStoneType" & idx).Text = Trim(defType)
            Else
                fraList.Controls("cbStoneName" & idx).Text = sText(1)
                For i = 0 To UBound(RST)
                    If RST2(i) = sText(2) Then
                        fraList.Controls("cbStoneType" & idx).Text = Trim(RST(i))
                    End If
                Next
            End If

        ElseIf secc = "KalipAyir" And srText.Count > 0 Then
            Set SrD2 = New ShapeRange
            SrD2.AddRange SrC.Duplicate((srKalip.SizeWidth + 20) * (SrG.Count + 1), 0)
            SrD2.ApplyUniformFill CreateRGBColor(255, 0, 0)
            SrD2.AddRange srKalip.Duplicate((srKalip.SizeWidth + 20) * (SrG.Count + 1), 0)
            
            For Each s In srText
                If s.Fill.UniformColor.HexValue = hexColor Then
                    Set SrD = s.Duplicate
                    SrD.SetPosition SrD2.LeftX + 2, SrD2.TopY - 2
                    SrD.Fill.UniformColor.RGBAssign 0, 255, 0
                    SrD2.Add SrD
                End If
            Next
            SrD2.Group.Name = "Lazer" & idx
        End If
        SrG.Add SrC.Group
    Loop

    fraList.Caption = SrG.Count & " Renk  En: " & Yy(SrG.SizeWidth / 10) & " Cm  Boy: " & Yy(SrG.SizeHeight / 10) & " Cm"
    
    ' Şablon ayırma için srT ve SrTNo dolduruluyor
    SrTNo = SrG.Count
    For i = 1 To SrTNo
        Set srT(i) = SrG.Shapes(i)
    Next i
    
    chkDetailList_Click
    SrF.CreateSelection
    
    Optimize False, secc
    Exit Sub
ErrHandler:
    Optimize False, secc
    MsgBox "Hata oluştu: " & Err.Description
End Sub

Private Sub chkDetailList_Click()
    Dim i As Integer
    For i = 1 To 50
        If Me.Controls("lblStoneInfo" & i).Visible = False Then
            Exit For
        ElseIf chkDetailList.Value Then
            With Me.Controls("lblStoneInfo" & i)
                .Top = 10 + (i - 1) * 20
                .Left = 3
                .Width = 65
                .Height = 16
                .Visible = True
                .Font.Size = 8
            End With
            With Me.Controls("cbStoneName" & i)
                .Top = 10 + (i - 1) * 20
                .Left = 71
                .Width = 97
                .Height = 16
                .Visible = True
                .Font.Size = 8
            End With
            With Me.Controls("cbStoneType" & i)
                .Top = 10 + (i - 1) * 20
                .Left = 170
                .Width = 40
                .Height = 16
                .Visible = True
                .Font.Size = 8
                fraList.Height = .Top + .Height + .Height
                .ListWidth = 120
            End With
        Else
            With Me.Controls("lblStoneInfo" & i)
                .Top = 10 + Int((i - 1) / 3) * 15
                .Left = 3 + (((fraList.Width / 3) - 5 + 3) * ((i - 1) Mod 3))
                .Width = (fraList.Width / 3) - 5
                .Height = 12
                .Visible = True
                .Font.Size = 8
                fraList.Height = .Top + .Height + .Height
            End With

            Me.Controls("cbStoneName" & i).Visible = False
            Me.Controls("cbStoneType" & i).Visible = False
        End If
    Next i
    Meh
End Sub

Sub KalipBilgiEkleM(ByVal KrKalip As ShapeRange, sr As ShapeRange)
    Dim tasTuri As Integer, idx As Integer, SatirK As Integer
    Dim posX As Double, posY As Double, SutunK As Double
    Dim tasAdi As String, tasSize As String, tasAdet As String, tasTurText As String, sizeText As String, ustBilgi As String
    Dim tasBilgi() As String
    
    Dim tiA As New ShapeRange, Kt As New ShapeRange, TextM As New ShapeRange
    Dim s As Shape
    srText.Delete
    
    ' 1. Oncelikle desen sinirlarini hesaplayalim
    Dim desenMinX As Double, desenMaxX As Double
    Dim desenMinY As Double, desenMaxY As Double
    Dim hasStones As Boolean
    
    desenMinX = 999999
    desenMaxX = -999999
    desenMinY = 999999
    desenMaxY = -999999
    hasStones = False
    
    For Each s In sr
        Dim w As Double, h As Double
        w = s.SizeWidth
        h = s.SizeHeight
        
        Dim isNoFill As Boolean
        isNoFill = True
        On Error Resume Next
        If s.Fill.Type = cdrUniformFill Then isNoFill = False
        On Error GoTo 0
        
        Dim fillCol As Long
        fillCol = -1
        If Not isNoFill Then fillCol = s.Fill.UniformColor.RGBValue
        
        ' Gerçek desen taşı filtresi (boyut < 10mm, dolgulu, siyah/beyaz olmayan)
        If w < 10 And h < 10 And Not isNoFill And fillCol <> RGB(255, 255, 255) And fillCol <> RGB(0, 0, 0) Then
            If s.LeftX < desenMinX Then desenMinX = s.LeftX
            If s.RightX > desenMaxX Then desenMaxX = s.RightX
            If s.BottomY < desenMinY Then desenMinY = s.BottomY
            If s.TopY > desenMaxY Then desenMaxY = s.TopY
            hasStones = True
        End If
    Next s
    
    ' Eger hic tas yoksa varsayilan secim sinirlarini alalim
    If Not hasStones Then
        desenMinX = sr.LeftX
        desenMaxX = sr.RightX
        desenMinY = sr.BottomY
        desenMaxY = sr.TopY
    End If
    
    ' 2. Sayfadaki tum sekilleri tarayip, desen sinirlarini icine alan en buyuk kalip karesini bulalim
    Dim kalipLeft As Double, kalipTop As Double
    Dim maxArea As Double
    Dim bestKalip As Shape
    Dim allShapes As ShapeRange
    Set allShapes = ActivePage.Shapes.FindShapes
    
    maxArea = 0
    For Each s In allShapes
        Dim nameLower As String
        nameLower = LCase(s.Name)
        
        Dim isKalipObj As Boolean
        isKalipObj = (InStr(nameLower, "kesim") > 0 Or InStr(nameLower, "kalip") > 0 Or InStr(nameLower, "ztrass") > 0)
        
        ' Adi bos olsa bile dolgusuz ve buyuk bir dikdortgense kalip kabul edelim
        If Not isKalipObj And s.Type = cdrRectangleShape And s.SizeWidth > 20 And s.SizeHeight > 20 Then
            Dim fillT As Long
            fillT = 0
            On Error Resume Next
            fillT = s.Fill.Type
            On Error GoTo 0
            If fillT = 0 Then isKalipObj = True
        End If
        
        If isKalipObj Then
            ' Bu sekil desen sınırlarını kapsıyor mu (geometrik enclosing)?
            If s.LeftX <= desenMinX + 1 And s.RightX >= desenMaxX - 1 And _
               s.BottomY <= desenMinY + 1 And s.TopY >= desenMaxY - 1 Then
               
                Dim area As Double
                area = s.SizeWidth * s.SizeHeight
                If area > maxArea Then
                    maxArea = area
                    Set bestKalip = s
                End If
            End If
        End If
    Next s
    
    If Not bestKalip Is Nothing Then
        kalipLeft = bestKalip.LeftX
        kalipTop = bestKalip.TopY
    Else
        kalipLeft = KrKalip.LeftX
        kalipTop = KrKalip.TopY
    End If
    
    ustBilgi = "Firma Adı: " & txtFirma.Text & vbCrLf & lblDosya.Caption & " " & txtDosya.Text & vbCrLf & lblKalip.Caption & " " & txtKalip.Text & vbCrLf & _
               "Kalıp Ölçüsü : " & Yy(sr.SizeWidth / 10) & " X " & Yy(sr.SizeHeight / 10) & vbCrLf

    If Trim(txtPres.Text) <> "0" Then ustBilgi = ustBilgi & lblPres.Caption & " " & txtPres.Text & vbCrLf
    If Trim(txtMontaj.Text) <> "0" Then ustBilgi = ustBilgi & lblMontaj.Caption & " " & txtMontaj.Text & vbCrLf

    ustBilgi = ustBilgi & "Renk Sayısı : " & sr.Count & vbCrLf & lblKalipAdet.Caption & " " & txtKalipAdet.Text & vbCrLf & lblToplamAdet.Caption & " " & txtToplamAdet.Text & vbCrLf & _
               Format(Now, "dd.MM.yyyy HH:mm")

    TextM.Add ActiveLayer.CreateArtisticText(kalipLeft, kalipTop + (UBound(Split(ustBilgi, vbCrLf)) * 3.3), ustBilgi, , , , 8.01, cdrTrue)
    
    ' Bilgi metnini kalıbın sol üstünün tam 5 mm üzerine yerleştirelim
    Dim textHeight As Double
    textHeight = TextM(1).SizeHeight
    TextM(1).PositionX = kalipLeft
    TextM(1).PositionY = kalipTop + 5 + textHeight

    idx = 1
    SatirK = 1

    For Each s In sr
        If Not Me.Controls("lblStoneInfo" & idx).Visible Then GoTo DevamEt

        tasBilgi = Split(Me.Controls("lblStoneInfo" & idx).Caption, "mm-")
        If UBound(tasBilgi) < 1 Then GoTo DevamEt

        tasSize = Split(tasBilgi(0), ",")(0)
        tasAdet = tasBilgi(1)
        tasAdi = Me.Controls("cbStoneName" & idx).Text
        tasTuri = Me.Controls("cbStoneType" & idx).ListIndex
        If tasTuri < 0 Or tasTuri > 6 Then tasTuri = 6

        tasTurText = Array("Double", "Mc", "Rezene", "Octagon", "Drop", "Düz Metal", "Özel")(tasTuri)
        sizeText = Array("", "ss4", "ss6", "ss10", "ss16", "ss20", "ss30", "ss34", "ss40", "ss50", "ss60", tasSize & "mm")(IIf(tasTuri < 2, val(tasSize), 11))

        If idx = 1 Then
            tiA.Add ActiveLayer.CreateArtisticText(kalipLeft + 2, (kalipTop - 2) - (SatirK * 2.1), sizeText & "-" & tasAdi & "-" & tasTurText, , , , 8.01, cdrFalse)
            SutunK = TextM.RightX + 10
            Kt.Add ActiveLayer.CreateArtisticText(SutunK, TextM.BottomY, sizeText & "-" & tasAdi & "-" & tasTurText & "-" & tasAdet & " Adet", , , , 8.01, cdrFalse)
        Else
            tiA.Add ActiveLayer.CreateArtisticText(tiA.LastShape.RightX + 4, (kalipTop - 2) - (SatirK * 2.1), sizeText & "-" & tasAdi & "-" & tasTurText & "-" & tasAdet & " Adet", , , , 8.01, cdrFalse)
            if tiA.RightX > KrKalip.RightX Then
                SatirK = SatirK + 1
                tiA.LastShape.Delete
                tiA.Add ActiveLayer.CreateArtisticText(kalipLeft + 2, (kalipTop - 2) - (SatirK * 2.1), sizeText & "-" & tasAdi & "-" & tasTurText, , , , 8.01, cdrFalse)
            End If

            Kt.Add ActiveLayer.CreateArtisticText(SutunK, Kt.LastShape.TopY + 1.1, sizeText & "-" & tasAdi & "-" & tasTurText & "-" & tasAdet & " Adet", , , , 8.01, cdrFalse)

            If Kt.TopY > TextM.TopY Then
                Kt.LastShape.Delete
                SutunK = Kt.RightX + 2
                Kt.Add ActiveLayer.CreateArtisticText(SutunK, TextM.BottomY, sizeText & "-" & tasAdi & "-" & tasTurText & "-" & tasAdet & " Adet", , , , 8.01, cdrFalse)
            End If
        End If

        If s.Fill.Type = cdrUniformFill Then
            tiA.LastShape.Fill.UniformColor = s.Fill.UniformColor
            Kt.LastShape.Fill.UniformColor = s.Fill.UniformColor
        Else
            tiA.LastShape.Fill.UniformColor = s.Shapes.First.Outline.Color
            Kt.LastShape.Fill.UniformColor = s.Shapes.First.Outline.Color
        End If

DevamEt:
        idx = idx + 1
    Next
End Sub

Sub DenYakinM(ByVal sr As ShapeRange)
    Set SrKAlt = New ShapeRange
    Set SrKUst = New ShapeRange

    Dim secR As New ShapeRange
    Dim S1 As Shape, S2 As Shape, sTest As Shape
    Dim i As Integer, tolerans As Double
    tolerans = CDbl(txtCloseDistance.Text)
    Set sr = ActiveSelectionRange.Shapes.FindShapes(Query:="@type <> 'group' and @width < {15 mm}").All
    If sr.Count < 2 Then Exit Sub
    
    For i = 1 To sr.Count
        Set S1 = sr(i)
        Dim j As Integer
        For j = i + 1 To sr.Count
            Set S2 = sr(j)

            If chkCloseArea.Value And S1.DisplayCurve.Area <> S2.DisplayCurve.Area Then Exit For
            
            If Sqr((S1.CenterX - S2.CenterX) ^ 2 + (S1.CenterY - S2.CenterY) ^ 2) <= _
                (S1.SizeWidth + S2.SizeWidth) / 2 + tolerans Then
                
                SrKAlt.Add S1
                SrKUst.Add S2
            End If
        Next j
    Next i
    
    chkCloseLower.Caption = Split(chkCloseLower.Caption, ":")(0) & ":" & SrKAlt.Count
    chkCloseUpper.Caption = Split(chkCloseUpper.Caption, ":")(0) & ":" & SrKUst.Count
    btnCloseSelect.Enabled = False

    If chkCloseLower.Value And chkCloseUpper.Value Then
        Set sr = ActiveDocument.CreateShapeRangeFromArray(SrKUst, SrKAlt)
    ElseIf chkCloseLower.Value Then
        Set sr = SrKAlt
    ElseIf chkCloseUpper.Value Then
        Set sr = SrKUst
    End If
    
    If Not sr Is Nothing Then
        If optCloseSelect.Value Then
            sr.CreateSelection
        ElseIf optCloseDelete.Value Then
            sr.Delete
        End If
        btnCloseSelect.Enabled = True
    End If
End Sub

Private Sub DaenYakinMaa(ByVal sr As ShapeRange)
    Dim secR As New ShapeRange
    Dim S1 As Shape, S2 As Shape, sTest As Shape
    Dim i As Integer, tolerans As Double

    tolerans = CDbl(txtCloseDistance.Text)
    Set sr = ActiveSelectionRange.Shapes.FindShapes(Query:="@type <> 'group' and @width < {15 mm}").All
    If sr.Count < 2 Then Exit Sub

    For i = 1 To sr.Count
        Set S1 = sr(i)
        Dim S1ZatenSecili As Boolean: S1ZatenSecili = False
        
        For Each sTest In secR
            If S1.StaticID = sTest.StaticID Then
                S1ZatenSecili = True
                Exit For
            End If
        Next sTest
        
        If S1ZatenSecili Then Exit For

        Dim j As Integer
        For j = i + 1 To sr.Count
            Set S2 = sr(j)

            If chkCloseArea.Value And S1.DisplayCurve.Area <> S2.DisplayCurve.Area Then Exit For

            If Sqr((S1.CenterX - S2.CenterX) ^ 2 + (S1.CenterY - S2.CenterY) ^ 2) <= _
                (S1.SizeWidth + S2.SizeWidth) / 2 + tolerans Then

                If chkCloseLower.Value Then secR.Add S1
                If chkCloseUpper.Value Then secR.Add S2
                Exit For
            End If
        Next j
    Next i

    If secR.Count = 0 Then
        btnCloseSelect.Enabled = False
        MsgBox "Kesişme Yok."
    Else
        btnCloseSelect.Enabled = True
        secR.CreateSelection
    End If
End Sub

Private Sub chkCloseLower_Click()
    If Not chkCloseLower.Value And Not chkCloseUpper.Value Then chkCloseUpper.Value = True
End Sub

Private Sub chkCloseUpper_Click()
    If Not chkCloseLower.Value And Not chkCloseUpper.Value Then chkCloseLower.Value = True
End Sub

Private Sub chkPanelTemplate_Click(): PanelGoster: End Sub
Private Sub chkPanelCalculate_Click(): PanelGoster: End Sub

Private Sub PanelGoster()
    If fraTemplate.Visible Then
        chkPanelTemplate.Value = False
    ElseIf fraCalculate.Visible Then
        chkPanelCalculate.Value = False
    End If
    fraTemplate.Visible = chkPanelTemplate.Value
    fraTemplateInfo.Visible = chkPanelTemplate.Value
    btnCalculate.Visible = chkPanelCalculate.Value
    fraCalculate.Visible = chkPanelCalculate.Value
    Meh
End Sub

Private Sub txtToplamAdet_Change(): TemizleVeDuzelt Me.txtToplamAdet: End Sub
Private Sub txtKalipAdet_Change(): TemizleVeDuzelt Me.txtKalipAdet: End Sub
Private Sub txtMontaj_Change(): TemizleVeDuzelt Me.txtMontaj: End Sub
Private Sub txtPres_Change(): TemizleVeDuzelt Me.txtPres: End Sub

Private Sub txtKareMM_Change(): TemizleVeDuzelt Me.txtKareMM: End Sub
Private Sub txtTasMM_Change(): TemizleVeDuzelt Me.txtTasMM: End Sub
Private Sub txtCloseDistance_Change(): TemizleVeDuzelt Me.txtCloseDistance: End Sub

Private Sub MakS1_Change(): TemizleVeDuzelt Me.MakS1: End Sub
Private Sub MakS2_Change(): TemizleVeDuzelt Me.MakS2: End Sub
Private Sub MakS3_Change(): TemizleVeDuzelt Me.MakS3: End Sub
Private Sub MakS4_Change(): TemizleVeDuzelt Me.MakS4: End Sub
Private Sub MakS5_Change(): TemizleVeDuzelt Me.MakS5: End Sub
Private Sub MakS6_Change(): TemizleVeDuzelt Me.MakS6: End Sub

Private Sub TemizleVeDuzelt(ByRef kutu As MSForms.TextBox)
    Dim txt As String, i As Integer, ch As String, sonuc As String
    Dim virgulVar As Boolean: virgulVar = False

    txt = Replace(kutu.Text, ".", ",") ' Noktayı virgüle çevir

    For i = 1 To Len(txt)
        ch = Mid(txt, i, 1)
        
        If ch Like "[0-9]" Then
            sonuc = sonuc & ch
        ElseIf ch = "," Then
            if Not virgulVar and Len(sonuc) > 0 Then
                sonuc = sonuc & ","
                virgulVar = True
            End If
        End If
    Next

    kutu.Text = sonuc
End Sub

Private Function ShapeRangeContains(ByVal sr As ShapeRange, ByVal targetShape As Shape) As Boolean
    Dim s As Shape
    ShapeRangeContains = False
    For Each s In sr
        If s.StaticID = targetShape.StaticID Then
            ShapeRangeContains = True
            Exit Function
        End If
    Next s
End Function

Sub Meh()
    Dim ctl As Object
    Dim enAlt As Double, enSag As Double

    For Each ctl In Me.Controls
        If TypeName(ctl) = "Frame" And ctl.Visible Then
            If ctl.Top + ctl.Height > enAlt Then enAlt = ctl.Top + ctl.Height
            If ctl.Left + ctl.Width > enSag Then enSag = ctl.Left + ctl.Width
        End If
    Next
    Me.Width = enSag + (fraBanner.Width * 0.075)
    Me.Height = enAlt + (fraBanner.Height * 1.4)
End Sub
