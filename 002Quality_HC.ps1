$path_original = "000_original_samples/"
$path_ViSQOL = "001_ViSQOL/"
$path_compressed = "002_compressed_samples_HC/"
$path_decompressed = "003_decompressed_samples_HC/"
$path_results = "004_results_HC/"

$exe = "C:\Users\tesse\_bazel_tesse\m64zn6ua\execroot\__main__\bazel-out\x64_windows-opt\bin\visqol.exe"

$model = $path_ViSQOL + "libsvm_nu_svr_model.txt"
$items = Get-ChildItem -Path $path_original | Where-Object { $_.Extension -eq ".wav" }

$results = @()

foreach ($item in $items) {
  $item.Name
  $original_size = $item.Length / 1kb

  $wav_in = $path_original + $item.Name
  $wav_audio = $path_ViSQOL + "original_audio_mode/" + $item.Name
  if (-not (Test-Path $wav_audio)) {
    ffmpeg -loglevel error -hide_banner -y -i $wav_in -ar 48000 $wav_audio
  }
  $wav_speech = $path_ViSQOL + "original_speech_mode/" + $item.Name
  if (-not (Test-Path $wav_speech)) {
    ffmpeg -loglevel error -hide_banner -y -i $wav_in -ar 16000 $wav_speech
  }


  $cmp_in = $path_decompressed + $item.Name.replace($item.Extension, ".hc.wav")
  $cmp_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".hc.wav")
  if (-not (Test-Path $cmp_audio)) {
    ffmpeg -loglevel error -hide_banner -y -i $cmp_in -ar 48000 $cmp_audio
  }
  $cmp_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".hc.wav")
  if (-not (Test-Path $cmp_speech)) {
    ffmpeg -loglevel error -hide_banner -y -i $cmp_in -ar 16000 $cmp_speech
  }


  $mp3_in = $path_compressed + $item.Name.replace($item.Extension, ".mp3")
  $mp3_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".mp3.wav")
  if (-not (Test-Path $mp3_audio)) {
    ffmpeg -loglevel error -hide_banner -y -i $mp3_in -ar 48000 $mp3_audio
  }
  $mp3_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".mp3.wav")
  if (-not (Test-Path $mp3_speech)) {
    ffmpeg -loglevel error -hide_banner -y -i $mp3_in -ar 16000 $mp3_speech
  }

  
  $m4a_in = $path_compressed + $item.Name.replace($item.Extension, ".m4a")
  $m4a_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".m4a.wav")
  if (-not (Test-Path $m4a_audio)) {
    ffmpeg -loglevel error -hide_banner -y -i $m4a_in -ar 48000 $m4a_audio
  }
  $m4a_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".m4a.wav")
  if (-not (Test-Path $m4a_speech)) {
    ffmpeg -loglevel error -hide_banner -y -i $m4a_in -ar 16000 $m4a_speech
  }


  $opus_in = $path_compressed + $item.Name.replace($item.Extension, ".opus")
  $opus_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".opus.wav")
  if (-not (Test-Path $opus_audio)) {
    ffmpeg -loglevel error -hide_banner -y -i $opus_in -ar 48000 $opus_audio
  }
  $opus_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".opus.wav")
  if (-not (Test-Path $opus_speech)) {
    ffmpeg -loglevel error -hide_banner -y -i $opus_in -ar 16000 $opus_speech
  }

  # HC ###############
  $res = (python "metrics.py" $wav_in ($path_decompressed + $item.Name.replace($item.Extension, ".hc.wav"))).Split("-")
  $compressed_size = (Get-Item ($path_compressed + $item.Name.replace($item.Extension, ".hc"))).Length / 1kb

  $obj = & $exe --reference_file $wav_audio --degraded_file $cmp_audio --similarity_to_quality_model $model  
  $line = [ordered] @{
    Sample                  = $item.Name.replace($item.Extension, "")
    "Original size (KB)"    = $original_size
    "HC Size (KB)"          = $compressed_size
    "HC Rate"               = $original_size / $compressed_size
    "HC MSE"                = [double] $res[0]
    "HC Absolute Error" = [double] $res[1]
    "HC MOS-LQO audio"      = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $cmp_speech --similarity_to_quality_model $model --use_speech_mode
  $line += [ordered] @{
    "HC MOS-LQO speech" = $obj[2].Split(":")[1].Trim()
  }

  # MP3 ###############
  $res = (python "metrics.py" $wav_in ($path_decompressed + $item.Name.replace($item.Extension, ".mp3.wav"))).Split("-")
  $compressed_size = (Get-Item ($path_compressed + $item.Name.replace($item.Extension, ".mp3"))).Length / 1kb

  $obj = & $exe --reference_file $wav_audio --degraded_file $mp3_audio --similarity_to_quality_model $model  
  $line += [ordered] @{
    "MP3 Size (KB)"          = $compressed_size
    "MP3 Rate"               = $original_size / $compressed_size
    "MP3 MSE"                = [double] $res[0]
    "MP3 Absolute Error" = [double] $res[1]
    "MP3 MOS-LQO audio"      = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $mp3_speech --similarity_to_quality_model $model --use_speech_mode
  $line += [ordered] @{
    "MP3 MOS-LQO speech" = $obj[2].Split(":")[1].Trim()
  }

  # ACC ###############
  $res = (python "metrics.py" $wav_in ($path_decompressed + $item.Name.replace($item.Extension, ".m4a.wav"))).Split("-")
  $compressed_size = (Get-Item ($path_compressed + $item.Name.replace($item.Extension, ".m4a"))).Length / 1kb
  
  $obj = & $exe --reference_file $wav_audio --degraded_file $m4a_audio --similarity_to_quality_model $model  
  $line += [ordered] @{
    "ACC Size (KB)"          = $compressed_size
    "ACC Rate"               = $original_size / $compressed_size
    "ACC MSE"                = [double] $res[0]
    "ACC Absolute Error" = [double] $res[1]
    "ACC MOS-LQO audio"      = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $m4a_speech --similarity_to_quality_model $model --use_speech_mode
  $line += [ordered] @{
    "ACC MOS-LQO speech" = $obj[2].Split(":")[1].Trim()
  }

  # Opus ###############
  $res = (python "metrics.py" $wav_in ($path_decompressed + $item.Name.replace($item.Extension, ".opus.wav"))).Split("-")
  $compressed_size = (Get-Item ($path_compressed + $item.Name.replace($item.Extension, ".opus"))).Length / 1kb


  $obj = & $exe --reference_file $wav_audio --degraded_file $opus_audio --similarity_to_quality_model $model  
  $line += [ordered] @{
    "OPUS Size (KB)"          = $compressed_size
    "OPUS Rate"               = $original_size / $compressed_size
    "OPUS MSE"                = [double] $res[0]
    "OPUS Absolute Error" = [double] $res[1]
    "OPUS MOS-LQO audio"      = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $opus_speech --similarity_to_quality_model $model --use_speech_mode
  $line += [ordered] @{
    "OPUS MOS-LQO speech" = $obj[2].Split(":")[1].Trim()
  }
  $results += New-Object PSObject -Property $line
}

$results | export-csv -Path ($path_results + "Quality.csv") -NoTypeInformation


