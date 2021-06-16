
$orig_path = "000_original_samples/"
$comp_path = "005_compressed_samples_PC_qxy/"
$dcmp_path = "006_decompressed_samples_PC_qxy/"
$rsut_path = "009_results_PC/PC_qxy.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }

$results = @()
foreach ($item in $items) {
  "\n"
  $item.Name
  for ($i = 1; $i -le 5; $i++) {
    $q = $i / 5
    for ($j = 0; $j -le 5; $j++) {
      $qxy = $j / 5
      $in = $orig_path + $item.Name
      $orig_size = $item.Length / 1kb

      $line = [ordered] @{
        Sample                  = $item.Name.replace($item.Extension, "")
        q                       = $q
        qxy                     = $qxy
        "Original size (KB)"    = $orig_size
        "Compression time (ms)" = (Measure-Command { executables/x64_Release_Compress.exe $in -q $q -qxy $qxy | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }
      $name = $item.Name.replace($item.Extension, "")
      $in  = $orig_path + $name + ".pc"
      $out = $comp_path + $name + "-q=" + $q + "-qxy=" + $qxy + "-.pc"
      Move-Item -Path $in -Destination $out -Force
      $comp_size = (Get-Item $out).Length / 1kb
      $line += @{
        "size (KB)"              = $comp_size
        "rate"                   = $orig_size / $comp_size
        "Decompression time (ms" = (Measure-Command { executables/x64_Release_Compress.exe $out -a pc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $in  = $comp_path + $name + "-q=" + $q + "-qxy=" + $qxy + "-pc.wav"
      $out = $dcmp_path + $name + "-q=" + $q + "-qxy=" + $qxy + ".wav"
      Move-Item -Path $in -Destination $out -Force

      $results += New-Object PSObject -Property $line
    }
  }
}
$results | export-csv -Path $rsut_path -NoTypeInformation

# exit

# for ($i = 0; $i -le 10; $i++) {
#   for ($j = 0; $j -le 10; $j++) {
    
#     $j / 10
#     foreach ($item in $items) {
#       $item.Name
#       $in = "compressed_samples/" + $item.Name
#       # $out = "decompressed_samples/" + $item.Name.replace($item.Extension, "mp3.wav")
  
#       $line = [ordered] @{
#         Sample = $item.Name.replace($item.Extension, "")
#         CMP    = (Measure-Command { code/executables/x64_Release_Compress.exe $in -a .cmp -q ($i / 10) | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
#       }

#     }
#   }
# }



# for ($ctr = 1 ; $ctr -le 100 ; $ctr++) {

#   $results = @()

#   foreach ($item in $items) {
#     $item.Name
#     $in = "compressed_samples/" + $item.Name
#     # $out = "decompressed_samples/" + $item.Name.replace($item.Extension, "mp3.wav")

#     $line = [ordered] @{
#       Sample = $item.Name.replace($item.Extension, "")
#       CMP    = (Measure-Command { code/executables/x64_Release_Compress.exe $in -a .cmp | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
#     }

#     $in = "compressed_samples/" + $item.Name.replace($item.Extension, ".mp3")
#     $out = "decompressed_samples/" + $item.Name.replace($item.Extension, ".mp3.wav")
#     $line += @{
#       MP3 = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
#     }

#     $in = "compressed_samples/" + $item.Name.replace($item.Extension, ".m4a")
#     $out = "decompressed_samples/" + $item.Name.replace($item.Extension, ".m4a.wav")
#     $line += @{
#       AAC = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
#     }

#     $in = "compressed_samples/" + $item.Name.replace($item.Extension, ".opus")
#     $out = "decompressed_samples/" + $item.Name.replace($item.Extension, ".opus.wav")
#     $line += @{
#       Opus = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
#     }

#     $results += New-Object PSObject -Property $line
#   }

#   $results | export-csv -Path ("./results/Decompression_time_milliseconds/" + $ctr + ".csv") -NoTypeInformation
# }
# ###########################
# ## Moving .cmp.wav files ##
# ###########################
# $items = Get-ChildItem -Path ./compressed_samples | Where-Object { $_.Name.EndsWith(".cmp.wav") } #-Encoding UTF8

# foreach ($item in $items) {
#   $in = "compressed_samples/" + $item.Name
#   $out = "decompressed_samples/" + $item.Name
#   Move-Item -Path $in -Destination $out -Force
# }