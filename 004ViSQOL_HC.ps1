$path_original     = "000_original_samples/"
$path_ViSQOL       = "001_ViSQOL/"
$path_compressed   = "002_compressed_samples/"
$path_decompressed = "003_decompressed_samples/"
$path_results      = "004_results/"

$exe               = "C:\Users\tesse\_bazel_tesse\m64zn6ua\execroot\__main__\bazel-out\x64_windows-opt\bin\visqol.exe"

$model             = $path_ViSQOL + "libsvm_nu_svr_model.txt"
$items = Get-ChildItem -Path $path_original | Where-Object {$_.Extension -eq ".wav"}

$results = @()

foreach ($item in $items) {

  $item.Name

  $wav_in = $path_original  + $item.Name
  $wav_audio = $path_ViSQOL + "original_audio_mode/" + $item.Name
  $wav_speech = $path_ViSQOL + "original_speech_mode/" + $item.Name
  ffmpeg -loglevel error -hide_banner -y -i $wav_in -ar 48000 $wav_audio
  ffmpeg -loglevel error -hide_banner -y -i $wav_in -ar 16000 $wav_speech

  $cmp_in = $path_decompressed  + $item.Name.replace($item.Extension, ".hc.wav")
  $cmp_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".hc.wav")
  $cmp_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".hc.wav")
  ffmpeg -loglevel error -hide_banner -y -i $cmp_in -ar 48000 $cmp_audio
  ffmpeg -loglevel error -hide_banner -y -i $cmp_in -ar 16000 $cmp_speech

  $mp3_in = $path_compressed  + $item.Name.replace($item.Extension, ".mp3")
  $mp3_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".mp3.wav")
  $mp3_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".mp3.wav")
  ffmpeg -loglevel error -hide_banner -y -i $mp3_in -ar 48000 $mp3_audio
  ffmpeg -loglevel error -hide_banner -y -i $mp3_in -ar 16000 $mp3_speech

  $m4a_in = $path_compressed  + $item.Name.replace($item.Extension, ".m4a")
  $m4a_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".m4a.wav")
  $m4a_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".m4a.wav")
  ffmpeg -loglevel error -hide_banner -y -i $m4a_in -ar 48000 $m4a_audio
  ffmpeg -loglevel error -hide_banner -y -i $m4a_in -ar 16000 $m4a_speech

  $opus_in = $path_compressed  + $item.Name.replace($item.Extension, ".opus")
  $opus_audio = $path_ViSQOL + "HC/audio_mode/" + $item.Name.replace($item.Extension, ".opus.wav")
  $opus_speech = $path_ViSQOL + "HC/speech_mode/" + $item.Name.replace($item.Extension, ".opus.wav")
  ffmpeg -loglevel error -hide_banner -y -i $opus_in -ar 48000 $opus_audio
  ffmpeg -loglevel error -hide_banner -y -i $opus_in -ar 16000 $opus_speech



  $obj = & $exe --reference_file $wav_audio --degraded_file $cmp_audio --similarity_to_quality_model $model  
  $line = [ordered] @{
    Sample = $item.Name.replace($item.Extension,"")
    Audio_HC = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $cmp_speech --similarity_to_quality_model $model --use_speech_mode
  $line += @{
    Speech_HC = $obj[2].Split(":")[1].Trim()
  }


  $obj = & $exe --reference_file $wav_audio --degraded_file $mp3_audio --similarity_to_quality_model $model  
  $line += @{
    Audio_MP3 = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $mp3_speech --similarity_to_quality_model $model --use_speech_mode
  $line += @{
    Speech_MP3 = $obj[2].Split(":")[1].Trim()
  }


  $obj = & $exe --reference_file $wav_audio --degraded_file $m4a_audio --similarity_to_quality_model $model  
  $line += @{
    Audio_AAC = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $m4a_speech --similarity_to_quality_model $model --use_speech_mode
  $line += @{
    Speech_AAC = $obj[2].Split(":")[1].Trim()
  }


  $obj = & $exe --reference_file $wav_audio --degraded_file $opus_audio --similarity_to_quality_model $model  
  $line += @{
    Audio_Opus = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $opus_speech --similarity_to_quality_model $model --use_speech_mode
  $line += @{
    Speech_Opus = $obj[2].Split(":")[1].Trim()
  }

  $results += New-Object PSObject -Property $line
}

$results | export-csv -Path ($path_results + "Quality_ViSQOL.csv") -NoTypeInformation


