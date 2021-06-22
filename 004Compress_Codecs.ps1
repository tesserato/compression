
$orig_path = "000_original_samples/"
$comp_path = "009_compressed_samples_codecs/"
# $dcmp_path = "008_decompressed_samples_PC/"
$rsut_path = "010_results_PC/Codecs.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }

$B = 8, 16, 24, 32, 40, 48, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320

$results = @()
foreach ($item in $items) {
  $item.Name
  $orig_size = $item.Length / 1kb
  $in = $orig_path + $item.Name
  $orig_duration = [double](ffprobe -i $in -show_entries format=duration -v quiet -of csv="p=0") * 1000
  $orig_samplerate = (ffprobe -v error -show_format -show_streams $in)[10].Split("=")[1] # show all
  # ffprobe -v error -show_format -show_streams $in
  # exit

  foreach ($quality in $B) {
    $bitrate = $quality * 1000
    $line = [ordered] @{
      "Sample"                     = $item.Name.replace($item.Extension, "")
      "Original duration (ms)"     = $orig_duration
      "Original size (KB)"         = $orig_size
      "Original sample rate (fps)" = $orig_samplerate
      "Bitrate (Kbit/s)"           = $quality
    }

    #####################
    ######## MP3 ########
    #####################
    if ($quality -ge 32) {
      $out = $comp_path + $item.Name.replace($item.Extension, "") + "-" + $quality + ".mp3"
      $comp_time = (Measure-Command { ffmpeg -loglevel error -hide_banner -y -i $in -b:a $bitrate -map_metadata -1 $out } | Select-Object -Property Milliseconds)."Milliseconds"
      $comp_size = (Get-Item $out).Length / 1kb
      $info = ffprobe -v error -show_format -show_streams $out
      $line += [ordered] @{
        "MP3 compression time (ms)" = $comp_time
        "MP3 Comp Time / Orig dur"  = $comp_time / $orig_duration
        "MP3 size (KB)"             = $comp_size
        "MP3 rate"                  = $orig_size / $comp_size
        "MP3 sample rate"           = $info[10].Split("=")[1]
        "MP3 Bit Rate"              = $info[22].Split("=")[1]
      }
    }

    #####################
    ######## ACC ########
    #####################
    $out = $comp_path + $item.Name.replace($item.Extension, "") + "-" + $quality + ".m4a"
    $comp_time = (Measure-Command { ffmpeg -loglevel error -hide_banner -y -i $in -b:a $bitrate -map_metadata -1 $out } | Select-Object -Property Milliseconds)."Milliseconds"
    $comp_size = (Get-Item $out).Length / 1kb
    $info = ffprobe -v error -show_format -show_streams $out
    $line += [ordered] @{
      "ACC compression time (ms)" = $comp_time
      "ACC Comp Time / Orig dur"  = $comp_time / $orig_duration
      "ACC size (KB)"             = $comp_size
      "ACC rate"                  = $orig_size / $comp_size
      "ACC sample rate"           = $info[10].Split("=")[1]
      "ACC Bit Rate"              = $info[22].Split("=")[1]
    }

    #####################
    ######## Opus ########
    #####################
    if ($quality -le 256) {
      $out = $comp_path + $item.Name.replace($item.Extension, "") + "-" + $quality + ".opus"
      $comp_time = (Measure-Command { ffmpeg -loglevel error -hide_banner -y -i $in -b:a $bitrate -map_metadata -1 $out } | Select-Object -Property Milliseconds)."Milliseconds"
      $comp_size = (Get-Item $out).Length / 1kb
      $info = ffprobe -v error -show_format -show_streams $out
      # $info[51]
      # exit
      $line += [ordered] @{
        "Opus compression time (ms)" = $comp_time
        "Opus Comp Time / Orig dur"  = $comp_time / $orig_duration
        "Opus size (KB)"             = $comp_size
        "Opus rate"                  = $orig_size / $comp_size
        "Opus sample rate"           = $info[10].Split("=")[1]
        "Opus Bit Rate"              = $info[51].Split("=")[1]
      }
    }
    $results += New-Object PSObject -Property $line
  }
}
$results | export-csv -Path $rsut_path -NoTypeInformation