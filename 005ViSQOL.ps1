$items = Get-ChildItem -Path original_samples | Where-Object {$_.Extension -eq ".wav"}
$model ="ViSQOL/libsvm_nu_svr_model.txt"
$exe = "C:\Users\tesse\_bazel_tesse\m64zn6ua\execroot\__main__\bazel-out\x64_windows-opt\bin\visqol.exe"

$results = @()

foreach ($item in $items) {

  $item.Name

  $wav_in = "original_samples/" + $item.Name
  $wav_audio = "ViSQOL/audio_mode/" + $item.Name
  $wav_speech = "ViSQOL/speech_mode/" + $item.Name
  ffmpeg -loglevel error -hide_banner -y -i $wav_in -ar 48000 $wav_audio
  ffmpeg -loglevel error -hide_banner -y -i $wav_in -ar 16000 $wav_speech

  $cmp_in = "decompressed_samples/" + $item.Name.replace($item.Extension, ".cmp.wav")
  $cmp_audio = "ViSQOL/audio_mode/" + $item.Name.replace($item.Extension, ".cmp.wav")
  $cmp_speech = "ViSQOL/speech_mode/" + $item.Name.replace($item.Extension, ".cmp.wav")
  ffmpeg -loglevel error -hide_banner -y -i $cmp_in -ar 48000 $cmp_audio
  ffmpeg -loglevel error -hide_banner -y -i $cmp_in -ar 16000 $cmp_speech

  $mp3_in = "compressed_samples/" + $item.Name.replace($item.Extension, ".mp3")
  $mp3_audio = "ViSQOL/audio_mode/" + $item.Name.replace($item.Extension, ".mp3.wav")
  $mp3_speech = "ViSQOL/speech_mode/" + $item.Name.replace($item.Extension, ".mp3.wav")
  ffmpeg -loglevel error -hide_banner -y -i $mp3_in -ar 48000 $mp3_audio
  ffmpeg -loglevel error -hide_banner -y -i $mp3_in -ar 16000 $mp3_speech

  $m4a_in = "compressed_samples/" + $item.Name.replace($item.Extension, ".m4a")
  $m4a_audio = "ViSQOL/audio_mode/" + $item.Name.replace($item.Extension, ".m4a.wav")
  $m4a_speech = "ViSQOL/speech_mode/" + $item.Name.replace($item.Extension, ".m4a.wav")
  ffmpeg -loglevel error -hide_banner -y -i $m4a_in -ar 48000 $m4a_audio
  ffmpeg -loglevel error -hide_banner -y -i $m4a_in -ar 16000 $m4a_speech

  $opus_in = "compressed_samples/" + $item.Name.replace($item.Extension, ".opus")
  $opus_audio = "ViSQOL/audio_mode/" + $item.Name.replace($item.Extension, ".opus.wav")
  $opus_speech = "ViSQOL/speech_mode/" + $item.Name.replace($item.Extension, ".opus.wav")
  ffmpeg -loglevel error -hide_banner -y -i $opus_in -ar 48000 $opus_audio
  ffmpeg -loglevel error -hide_banner -y -i $opus_in -ar 16000 $opus_speech



  $obj = & $exe --reference_file $wav_audio --degraded_file $cmp_audio --similarity_to_quality_model $model  
  $line = [ordered] @{
    Sample = $item.Name.replace($item.Extension,"")
    Audio_CMP = $obj[2].Split(":")[1].Trim()
  }

  $obj = & $exe --reference_file $wav_speech --degraded_file $cmp_speech --similarity_to_quality_model $model --use_speech_mode
  $line += @{
    Speech_CMP = $obj[2].Split(":")[1].Trim()
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

$results | export-csv -Path results/Quality_ViSQOL.csv -NoTypeInformation


