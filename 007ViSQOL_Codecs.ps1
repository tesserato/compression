


$exe = "C:\Users\tesse\_bazel_tesse\m64zn6ua\execroot\__main__\bazel-out\x64_windows-opt\bin\visqol.exe"
$orig_path = "000_original_samples/"
$outpath = "010_results_PC/Quality_Codecs_ViSQOL.csv"
# $cmppath = "007_compressed_samples_PC/"
$path = "009_compressed_samples_codecs/"
$temp = "001_ViSQOL/"
$model = $temp + "libsvm_nu_svr_model.txt"

function visqol {
  param (
    $o_path, $o_name, $d_path, $d_name
  )


  $sample_number = [int] $o_name.Split("_")[0]

  # $sample_number
  # $o_path
  # $o_name
  # $d_path
  # $d_name
  # exit

  if ($sample_number -le 8) {
    # Write-Output "Speech"
    $o_path_c = $temp + "original_speech_mode/" + $o_name
    if (-not (Test-Path $o_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($o_path + $o_name) -ar 16000 $o_path_c
    }

    $d_path_c = $temp + "temp/" + $d_name + ".wav"
    if (-not (Test-Path $d_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($d_path + $d_name) -ar 16000 $d_path_c
    }

    $obj = & $exe --reference_file $o_path_c --degraded_file $d_path_c --similarity_to_quality_model $model --use_speech_mode
    $mode = $obj[1]
    $MOS_LQO = $obj[2].Split(":")[1].Trim()
  }
  else {
    # Write-Output "Audio"
    $o_path_c = $temp + "original_audio_mode/" + $o_name
    if (-not (Test-Path $o_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($o_path + $o_name) -ar 48000 $o_path_c
    }

    $d_path_c = $temp + "temp/" + $d_name + ".wav"
    if (-not (Test-Path $d_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($d_path + $d_name) -ar 48000 $d_path_c
    }

    $obj = & $exe --reference_file $o_path_c --degraded_file $d_path_c --similarity_to_quality_model $model
    $mode = $obj[1]
    $MOS_LQO = $obj[2].Split(":")[1].Trim()
  }

  return New-Object PsObject -Property @{MOS_LQO = $MOS_LQO ; mode = $mode }
}

$items = Get-ChildItem -Path $path

$results = @()

foreach ($item in $items) {
  $item.Name

  # $compressed_path = $path + $item.Name

  $info = $item.Name.replace($item.Extension, "").Split("-")

  # $info
  

 

  $name = $info[0]
  $q = $info[1]#.Split("=")[1]
  $ext = $item.Extension

  # $name
  $q
  # $ext
exit

  $original_path = $orig_path +$name+".wav"
  $original_size = (Get-Item $original_path).Length / 1kb 
  $compressed_size = $item.Length / 1kb
  $compressed_path = $temp + "temp/" + $item.Name+ ".wav"

  # $original_path
  # $compressed_path
  

  $o = visqol $orig_path ($name+".wav") $path $item.Name

  $res = (python "metrics.py" $original_path $compressed_path).Split("-")

  $line = [ordered] @{
    "Sample"               = $name
    "Codec"                = $ext
    "Original size (KB)"   = $original_size
    "Compressed size (KB)" = $compressed_size
    "Bitrate (Kbit/s)"     = $q
    "Rate"                 = $original_size / $compressed_size
    "ViSQOL Mode"          = $o.mode
    "MOS_LQO"              = $o.MOS_LQO
    "MSE"                  = [double] $res[0]
    "AVG Absolute Error"   = [double] $res[1]
  }
  $results += New-Object PSObject -Property $line
}

$results | export-csv -Path $outpath -NoTypeInformation


