$corel = New-Object -ComObject CorelDRAW.Application
if (-not $corel) {
    Write-Host "Failed to connect to CorelDRAW"
    exit
}

$doc = $corel.ActiveDocument
if (-not $doc) {
    Write-Host "No active document"
    exit
}

$sel = $corel.ActiveSelectionRange
if ($sel.Count -eq 0) {
    Write-Host "Nothing selected"
    exit
}

$outDir = "C:\Users\tubbi\Dropbox\Claude\HotfixPro\scratch"
$pltFile = Join-Path $outDir "test_export.plt"

Write-Host "Exporting to $pltFile"

$cdrHPGL = 1303
$cdrSelection = 1

try {
    $opt = $corel.CreateStructExportOptions()
    $flt = $doc.ExportEx($pltFile, $cdrHPGL, $cdrSelection, $opt)
    $flt.Finish()
    Write-Host "ExportEx finished successfully!"
    
    if (Test-Path $pltFile) {
        $size = (Get-Item $pltFile).Length
        Write-Host "File created: $pltFile (Size: $size bytes)"
    } else {
        Write-Host "ExportEx returned without error, but FILE WAS NOT CREATED."
    }
} catch {
    Write-Host "ExportEx failed: $_"
}
