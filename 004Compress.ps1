
$orig_path = "000_original_samples/"
$comp_path = "007_compressed_samples_PC/"
$dcmp_path = "008_decompressed_samples_PC/"
$rsut_path = "009_results_PC/PC.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }

$results = @()
foreach ($item in $items) {
  $item.Name
  for ($i = 0; $i -le 8; $i++) {
    $q = $i / 10
    $in = $orig_path + $item.Name
    $orig_size = $item.Length / 1kb
    $line = [ordered] @{
      Sample                  = $item.Name.replace($item.Extension, "")
      q                       = $q
      # qxy                     = $qxy
      "Original size (KB)"    = $orig_size
      "Compression time (ms)" = (Measure-Command { executables/x64_Release_Compress.exe $in -q $q | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }
    $name = $item.Name.replace($item.Extension, "")
    $in = $orig_path + $name + ".pc"
    $out = $comp_path + $name + "-q=" + $q.ToString("N2") + "-.pc"
    Move-Item -Path $in -Destination $out -Force

    $comp_size = (Get-Item $out).Length / 1kb
    $line += @{
      "size (KB)"              = $comp_size
      "rate"                   = $orig_size / $comp_size
      "Decompression time (ms" = (Measure-Command { executables/x64_Release_Compress.exe $out -a pc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $comp_path + $name + "-q=" + $q.ToString("N2") + "-pc.wav"
    $out = $dcmp_path + $name + "-q=" + $q.ToString("N2") + ".wav"
    Move-Item -Path $in -Destination $out -Force

    $results += New-Object PSObject -Property $line
  }
}
$results | export-csv -Path $rsut_path -NoTypeInformation