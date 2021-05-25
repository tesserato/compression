
################
## Converting ##
################
$items = Get-ChildItem -Path .\original_samples | Where-Object {$_.Extension -eq ".wav"} #-Encoding UTF8

for ($ctr = 1 ; $ctr -le 100 ; $ctr++){

  $results = @()

  foreach ($item in $items) {
    $item.Name
    $in = "original_samples/" + $item.Name
    if ($in -notmatch "_rec" -and $in -notmatch ".m4a" -and $in -notmatch ".mp3" -and $in -notmatch ".opus"){
      $line = [ordered] @{
        Sample = $item.Name.replace($item.Extension,"")
      }

      $line += @{
        HC = (Measure-Command {code/executables/x64_Release_Compress.exe $in | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $out = "compressed_samples/" + $item.Name.replace($item.Extension,".mp3")
      $line += @{
        MP3 = (Measure-Command {lame --quiet -q 0 -b 8 --resample 8 $in $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }
      # Measure-Command {ffmpeg -hide_banner -y -i $in -b:a 8k -ar 8k -map_metadata -1 $out | Out-Default }

      $out = "compressed_samples/" + $item.Name.replace($item.Extension,".m4a")
      $line += @{
        ACC = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -ss 0 -i $in -b:a 4k -map_metadata -1 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $out = "compressed_samples/" + $item.Name.replace($item.Extension,".opus")
      $line += @{
        Opus = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -c:a libopus -b:a 6k -map_metadata -1 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $results += New-Object PSObject -Property $line
    }
  }

  $results | export-csv -Path ("./results/Compression_time_milliseconds/" + $ctr + ".csv") -NoTypeInformation


}

######################
## Moving cmp files ##
######################
$items = Get-ChildItem -Path ./original_samples | Where-Object {$_.Extension -eq ".hc"} #-Encoding UTF8

foreach ($item in $items) {
  $in = "original_samples/" + $item.Name
  $out = "compressed_samples/" + $item.Name

  Move-Item -Path $in -Destination $out -Force
}

######################
## Moving csv files ##
######################
$items = Get-ChildItem -Path ./original_samples | Where-Object {$_.Extension -eq ".csv"} #-Encoding UTF8

foreach ($item in $items) {
  $in = "original_samples/" + $item.Name
  $out = "csvs/" + $item.Name

  Move-Item -Path $in -Destination $out -Force
}