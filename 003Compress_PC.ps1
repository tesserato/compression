
$orig_path = "000_original_samples/"
$comp_path = "007_compressed_samples_PC/"
$dcmp_path = "008_decompressed_samples_PC/"
$rsut_path = "010_results_PC/PC.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }

$results = @()
foreach ($item in $items) {
  $item.Name
  $name = $item.Name.replace($item.Extension, "")
  $inwav = $orig_path + $item.Name
  $inpc = $orig_path + $name + ".pc"
  $orig_size = $item.Length / 1kb
  $orig_duration = [double](ffprobe -i $inwav -show_entries format=duration -v quiet -of csv="p=0") * 1000
  for ($i = 0; $i -le 10; $i++) {
    $q = $i / 10
    $comp_time = (Measure-Command { executables/x64_Release_Compress.exe $inwav -q $q | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    $out = $comp_path + $name + "-q=" + $q.ToString("N2") + "-.pc"
    Move-Item -Path $inpc -Destination $out -Force

    $line = [ordered] @{
      Sample                  = $item.Name.replace($item.Extension, "")
      q                       = $q
      "Original duration (ms)"= $orig_duration
      "Original size (KB)"    = $orig_size
      "Compression time (ms)" = $comp_time
      "Comp Time / Orig dur"  = $comp_time / $orig_duration
    }

    $comp_size = (Get-Item $out).Length / 1kb
    $decomp_time = (Measure-Command { executables/x64_Release_Compress.exe $out -a pc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    $line += [ordered] @{
      "Size (KB)"              = $comp_size
      "Rate"                   = $orig_size / $comp_size
      "Decompression time (ms)"= $decomp_time
      "Decomp Time / Orig dur" = $decomp_time / $orig_duration
    }

    $in = $comp_path + $name + "-q=" + $q.ToString("N2") + "-pc.wav"
    $out = $dcmp_path + $name + "-q=" + $q.ToString("N2") + ".wav"
    Move-Item -Path $in -Destination $out -Force

    $results += New-Object PSObject -Property $line
  }
}
$results | export-csv -Path $rsut_path -NoTypeInformation