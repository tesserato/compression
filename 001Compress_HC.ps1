
$runs = 1

$path_original     = "001_original_samples/"
$path_compressed   = "002_compressed_samples/"
$path_decompressed = "003_decompressed_samples/"
###############################################################################################################
## COMPRESSING ################################################################################################
###############################################################################################################

$items = Get-ChildItem -Path $path_original | Where-Object {$_.Extension -eq ".wav"} #-Encoding UTF8

for ($ctr = 1 ; $ctr -le $runs ; $ctr++){

  $results = @()

  foreach ($item in $items) {
    $item.Name
    $in = $path_original + $item.Name
    if ($in -notmatch "_rec" -and $in -notmatch ".m4a" -and $in -notmatch ".mp3" -and $in -notmatch ".opus"){
      $line = [ordered] @{
        Sample = $item.Name.replace($item.Extension,"")
      }

      $line += @{
        HC = (Measure-Command {executables/x64_Release_Compress.exe $in | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $out = $path_compressed + $item.Name.replace($item.Extension,".mp3")
      $line += @{
        MP3 = (Measure-Command {lame --quiet -q 0 -b 8 --resample 8 $in $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }
      # Measure-Command {ffmpeg -hide_banner -y -i $in -b:a 8k -ar 8k -map_metadata -1 $out | Out-Default }

      $out = $path_compressed + $item.Name.replace($item.Extension,".m4a")
      $line += @{
        ACC = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -ss 0 -i $in -b:a 4k -map_metadata -1 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $out = $path_compressed + $item.Name.replace($item.Extension,".opus")
      $line += @{
        Opus = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -c:a libopus -b:a 6k -map_metadata -1 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $results += New-Object PSObject -Property $line
    }
  }

  $results | export-csv -Path ("004_results/Compression_time_milliseconds/" + $ctr + ".csv") -NoTypeInformation
}

######################
## Moving .hc files ##
######################
$items = Get-ChildItem -Path $path_original | Where-Object {$_.Extension -eq ".hc"} #-Encoding UTF8

foreach ($item in $items) {
  $in = $path_original + $item.Name
  $out = $path_compressed + $item.Name

  Move-Item -Path $in -Destination $out -Force
}

###############################################################################################################
## DECOMPRESSING ##############################################################################################
###############################################################################################################
for ($ctr = 1 ; $ctr -le $runs ; $ctr++){

  $results = @()

  foreach ($item in $items) {
    $item.Name
    $in = $path_compressed + $item.Name
    # $out = "decompressed_samples/" + $item.Name.replace($item.Extension, "mp3.wav")

    $line = [ordered] @{
      Sample = $item.Name.replace($item.Extension,"")
      CMP = (Measure-Command {executables/x64_Release_Compress.exe $in -a .hc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $path_compressed + $item.Name.replace($item.Extension, ".mp3")
    $out = $path_decompressed + $item.Name.replace($item.Extension, ".mp3.wav")
    $line += @{
      MP3 = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $path_compressed + $item.Name.replace($item.Extension, ".m4a")
    $out = $path_decompressed + $item.Name.replace($item.Extension, ".m4a.wav")
    $line += @{
      AAC = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $path_compressed + $item.Name.replace($item.Extension, ".opus")
    $out = $path_decompressed + $item.Name.replace($item.Extension, ".opus.wav")
    $line += @{
      Opus = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $results += New-Object PSObject -Property $line
  }

  $results | export-csv -Path ("./004_results/Decompression_time_milliseconds/" + $ctr + ".csv") -NoTypeInformation
}
###########################
## Moving .cmp.wav files ##
###########################
$items = Get-ChildItem -Path $path_compressed | Where-Object {$_.Name.EndsWith(".hc.wav")} #-Encoding UTF8

foreach ($item in $items) {
  $in = $path_compressed + $item.Name
  $out = $path_decompressed + $item.Name
  Move-Item -Path $in -Destination $out -Force
}