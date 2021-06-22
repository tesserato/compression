
$orig_path = "000_original_samples/"
$comp_path = "005_compressed_samples_PC_qxy/"
$dcmp_path = "006_decompressed_samples_PC_qxy/"
$rsut_path = "010_results_PC/PC_qxy.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }

$results = @()
foreach ($item in $items) {
  $item.Name
  $inwav = $orig_path + $item.Name
  $inpc = $orig_path + $item.Name.replace($item.Extension, "") + ".pc"
  $orig_size = $item.Length / 1kb
  $orig_duration = [double](ffprobe -i $in -show_entries format=duration -v quiet -of csv="p=0") * 1000

  for ($i = 1; $i -le 5; $i++) {
    $q = $i / 5
    for ($j = 0; $j -le 5; $j++) {
      $qxy = $j / 5
      $comp_time = (Measure-Command { executables/x64_Release_Compress.exe $inwav -q $q -qxy $qxy | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      $line = [ordered] @{
        Sample                  = $item.Name.replace($item.Extension, "")
        q                       = $q.ToString("N2")
        qxy                     = $qxy.ToString("N2")
        "Original size (KB)"    = $orig_size
        "Compression time (ms)" = $comp_time
        "Comp Time / Orig dur"  = $comp_time / $orig_duration
      }
      $name = $item.Name.replace($item.Extension, "")
      $out = $comp_path + $name + "-q=" + $q.ToString("N2") + "-qxy=" + $qxy.ToString("N2") + ".pc"
      Move-Item -Path $inpc -Destination $out -Force
      $comp_size = (Get-Item $out).Length / 1kb
      $decomp_time = (Measure-Command { executables/x64_Release_Compress.exe $out -a .pc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      $line += [ordered] @{
        "size (KB)"              = $comp_size
        "rate"                   = $orig_size / $comp_size
        "Decompression time (ms" = $decomp_time
        "Decomp Time / Orig dur" = $decomp_time / $orig_duration
      }

      $in  = $comp_path + $name + "-q=" + $q.ToString("N2") + "-qxy=" + $qxy.ToString("N2") + ".pc.wav"
      $out = $dcmp_path + $name + "-q=" + $q.ToString("N2") + "-qxy=" + $qxy.ToString("N2") + ".wav"
      Move-Item -Path $in -Destination $out -Force

      $results += New-Object PSObject -Property $line
    }
  }
}
$results | export-csv -Path $rsut_path -NoTypeInformation