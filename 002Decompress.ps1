# $items = Get-ChildItem -Path compressed_samples | Where-Object {$_.Extension -eq ".mp3" -or $_.Extension -eq ".m4a" -or $_.Extension -eq ".opus"}

$items = Get-ChildItem -Path compressed_samples | Where-Object {$_.Extension -eq ".cmp"}

###################
## Decompressing ##
###################
for ($ctr = 1 ; $ctr -le 100 ; $ctr++){

  $results = @()

  foreach ($item in $items) {
    $item.Name
    $in = "compressed_samples/" + $item.Name
    # $out = "decompressed_samples/" + $item.Name.replace($item.Extension, "mp3.wav")

    $line = [ordered] @{
      Sample = $item.Name.replace($item.Extension,"")
      CMP = (Measure-Command {code/executables/x64_Release_Compress.exe $in -a .cmp | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = "compressed_samples/" + $item.Name.replace($item.Extension, ".mp3")
    $out = "decompressed_samples/" + $item.Name.replace($item.Extension, ".mp3.wav")
    $line += @{
      MP3 = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = "compressed_samples/" + $item.Name.replace($item.Extension, ".m4a")
    $out = "decompressed_samples/" + $item.Name.replace($item.Extension, ".m4a.wav")
    $line += @{
      AAC = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = "compressed_samples/" + $item.Name.replace($item.Extension, ".opus")
    $out = "decompressed_samples/" + $item.Name.replace($item.Extension, ".opus.wav")
    $line += @{
      Opus = (Measure-Command {ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $results += New-Object PSObject -Property $line
  }

  $results | export-csv -Path ("./results/Decompression_time_milliseconds/" + $ctr + ".csv") -NoTypeInformation
}
###########################
## Moving .cmp.wav files ##
###########################
$items = Get-ChildItem -Path ./compressed_samples | Where-Object {$_.Name.EndsWith(".cmp.wav")} #-Encoding UTF8

foreach ($item in $items) {
  $in = "compressed_samples/" + $item.Name
  $out = "decompressed_samples/" + $item.Name
  Move-Item -Path $in -Destination $out -Force
}