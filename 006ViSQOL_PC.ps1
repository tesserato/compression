


$exe = "C:\Users\tesse\_bazel_tesse\m64zn6ua\execroot\__main__\bazel-out\x64_windows-opt\bin\visqol.exe"
$orig_path = "000_original_samples/"
$outpath = "010_results_PC/Quality_PC_ViSQOL.csv"
$cmppath = "007_compressed_samples_PC/"
$path = "008_decompressed_samples_PC/"
$temp = "001_ViSQOL/"
$model = $temp + "libsvm_nu_svr_model.txt"

function visqol {
  param (
    $o_path, $o_name, $d_path, $d_name
  )


  # $sample_number = [int] $o_name.Split("_")[0]

  # if ($sample_number -le 8) {
    # Write-Output "Speech"
    $o_path_c = $temp + "original_speech_mode/" + $o_name
    if (-not (Test-Path $o_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($o_path + $o_name) -ar 16000 $o_path_c
    }

    $d_path_c = $temp + "temp/" + $d_name
    if (-not (Test-Path $d_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($d_path + $d_name) -ar 16000 $d_path_c
    }

    $obj = & $exe --reference_file $o_path_c --degraded_file $d_path_c --similarity_to_quality_model $model --use_speech_mode
    # $mode = $obj[1]
    $MOS_LQO_s = $obj[2].Split(":")[1].Trim()
  # }
  # else {
    # Write-Output "Audio"
    $o_path_c = $temp + "original_audio_mode/" + $o_name
    if (-not (Test-Path $o_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($o_path + $o_name) -ar 48000 $o_path_c
    }

    $d_path_c = $temp + "temp/48k-" +$d_name
    if (-not (Test-Path $d_path_c)) {
      ffmpeg -loglevel error -hide_banner -y -i ($d_path + $d_name) -ar 48000 $d_path_c
    }

    $obj = & $exe --reference_file $o_path_c --degraded_file $d_path_c --similarity_to_quality_model $model
    # $mode = $obj[1]
    $MOS_LQO_a = $obj[2].Split(":")[1].Trim()
  # }

  return New-Object PsObject -Property @{MOS_LQO_a = $MOS_LQO_a; MOS_LQO_s = $MOS_LQO_s }
}

$items = Get-ChildItem -Path $path | Where-Object { $_.Extension -eq ".wav" }
# $items = $items[1..5]

$results = @()

foreach ($item in $items) {
  $item.Name

  $compressed_path = $path + $item.Name

  $info = $item.Name.replace($item.Extension, "").Split("-")

 

  $name = $info[0]
  $q = $info[1].Split("=")[1]

  $original_path = $orig_path + $name + ".wav"
  $original_size = $item.Length / 1kb
  $compressed_size = (Get-Item ($cmppath + $item.Name.replace($item.Extension, ".pc"))).Length / 1kb 

  $o = visqol $orig_path ($name + ".wav") $path $item.Name

  $res = (python "metrics.py" $original_path $compressed_path).Split("-")

  $line = [ordered] @{
    "Sample"               = $name
    "Original size (KB)"   = $original_size
    "Compressed size (KB)" = $compressed_size
    "q"                    = $q
    "Rate"                 = $original_size / $compressed_size
    "MOS-LQO speech"       = $o.MOS_LQO_s
    "MOS-LQO audio"        = $o.MOS_LQO_a
    "MSE"                  = [double] $res[0]
    "AVG Absolute Error"   = [double] $res[1]
  }
  $results += New-Object PSObject -Property $line
}

$results | export-csv -Path $outpath -NoTypeInformation


