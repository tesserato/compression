
$orig_path = "000_original_samples/"
$comp_path = "009_compressed_samples_codecs/"
# $dcmp_path = "008_decompressed_samples_PC/"
$rsut_path = "009_results_PC/Codecs.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }

$B = 8, 16, 32, 64, 128, 320

$results = @()
foreach ($item in $items) {  
  $item.Name
  $orig_size = $item.Length / 1kb
  $in = $orig_path + $item.Name

  foreach ($bitrate in $B) {
    # $bitrate = [Math]::Pow(2, $i)
    $out = $comp_path + $item.Name.replace($item.Extension, "") + "-" + $bitrate + ".mp3"

    $line = [ordered] @{
      Sample                  = $item.Name.replace($item.Extension, "")
      "Original size (KB)"    = $orig_size
      "Bit Rate"              = $bitrate
    }

    $comp_size = (Get-Item $out).Length / 1kb
    $line += @{
      "MP3 compression time (ms)" = (Measure-Command {lame --quiet -b $bitrate $in $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      "Size (KB)"              = $comp_size
      "Rate"                   = $orig_size / $comp_size
    }


    #   q                       = $q
    #   # qxy                     = $qxy
    #   "Original size (KB)"    = $orig_size
    #   "Compression time (ms)" = (Measure-Command { executables/x64_Release_Compress.exe $in -q $q | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    
    # $name = $item.Name.replace($item.Extension, "")
    # $in = $orig_path + $name + ".pc"
    # $out = $comp_path + $name + "-q=" + $q.ToString("N2") + "-.pc"
    # Move-Item -Path $in -Destination $out -Force

    # $comp_size = (Get-Item $out).Length / 1kb
    # $line += @{
    #   "size (KB)"              = $comp_size
    #   "rate"                   = $orig_size / $comp_size
    #   "Decompression time (ms" = (Measure-Command { executables/x64_Release_Compress.exe $out -a pc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    # }

    # $in = $comp_path + $name + "-q=" + $q.ToString("N2") + "-pc.wav"
    # $out = $dcmp_path + $name + "-q=" + $q.ToString("N2") + ".wav"
    # Move-Item -Path $in -Destination $out -Force

    $results += New-Object PSObject -Property $line
  }
}
$results | export-csv -Path $rsut_path -NoTypeInformation