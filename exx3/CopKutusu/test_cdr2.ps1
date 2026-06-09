$cdr = New-Object -ComObject CorelDRAW.Application
$doc = $cdr.ActiveDocument
$layer = $doc.ActiveLayer
$text = $layer.CreateArtisticText(0, 0, "Line 1Line 2Line 3")
$story = $text.Text.Story
$lines = $story.Lines
$line2 = $lines.Item(2)
$line2.Fill.UniformColor.HexValue = "#FF0000"
Write-Host "Color applied via HexValue"
