# This script compresses and decompresses the wav files at $path_original $end - $start times, to obtain the compression / decompression times. The compressed files are saved in $path_compressed, in the .hc (current work), .mp3, .m4a (ACC) and .opus formats. Those files are decompressed and saved as .wav (.hc.wav, .mp3.wav, .m4a.wav and .opus.wav) in $path_decompressed. The results of each compression and decompression run are saved, respectively, at $path_result_cmp and $path_result_dec and are meant to be summarised with the python script "001Consolidate_HC.py"

$path_original = "000_original_samples/"
$path_compressed = "002_compressed_samples_HC/"
$path_decompressed = "003_decompressed_samples_HC/"
$path_result_cmp = "004_results_HC/Compression_time_milliseconds/"
$path_result_dec = "004_results_HC/Decompression_time_milliseconds/"

# $runs = 1..1000
# $runs = 349, 373, 591, 595, 687, 764, 774, 788, 832, 874, 936, 954
# $runs = 104, 106, 112, 114, 115, 122, 123, 125, 141, 142, 149, 149, 158, 165, 169, 172, 187, 197, 198, 204, 212, 215, 219, 224, 235, 24, 245, 246, 261, 264, 265, 267, 268, 27, 271, 272, 279, 292, 298, 302, 318, 326, 329, 33, 335, 339, 347, 356, 360, 364, 379, 380, 386, 394, 396, 397, 402, 406, 410, 411, 415, 422, 447, 45, 450, 458, 463, 468, 48, 481, 490, 491, 495, 503, 504, 510, 511, 512, 533, 542, 55, 554, 555, 557, 569, 576, 582, 593, 608, 62, 623, 628, 641, 642, 648, 650, 651, 656, 657, 66, 662, 676, 679, 68, 680, 683, 686, 689, 698, 706, 707, 713, 723, 724, 73, 735, 738, 74, 763, 775, 782, 783, 785, 789, 79, 802, 806, 809, 809, 81, 810, 816, 836, 838, 850, 853, 854, 857, 866, 867, 869, 872, 875, 878, 887, 889, 895, 898, 912, 916, 920, 923, 928, 933, 942, 95, 955, 96, 962, 964, 965, 966, 966, 970, 975, 978, 981, 992, 993, 996, 998
$runs = 125, 782, 850

$items = Get-ChildItem -Path $path_original | Where-Object { $_.Extension -eq ".wav" } #-Encoding UTF8

foreach ($ctr in $runs) {
  ###############################################################################################################
  ## COMPRESSING ################################################################################################
  ###############################################################################################################
  $results = @()

  foreach ($item in $items) {
    $item.Name
    $in = $path_original + $item.Name
    if ($in -notmatch "_rec" -and $in -notmatch ".m4a" -and $in -notmatch ".mp3" -and $in -notmatch ".opus") {
      $line = [ordered] @{
        Sample = $item.Name.replace($item.Extension, "")
      }

      $line += @{
        HC = (Measure-Command { executables/x64_Release_Compress.exe $in | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $out = $path_compressed + $item.Name.replace($item.Extension, ".mp3")
      $line += @{
        MP3 = (Measure-Command { lame --quiet -q 0 -b 8 --resample 8 $in $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }
      # Measure-Command {ffmpeg -hide_banner -y -i $in -b:a 8k -ar 8k -map_metadata -1 $out | Out-Default }

      $out = $path_compressed + $item.Name.replace($item.Extension, ".m4a")
      $line += @{
        ACC = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -ss 0 -i $in -b:a 4k -map_metadata -1 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $out = $path_compressed + $item.Name.replace($item.Extension, ".opus")
      $line += @{
        Opus = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -c:a libopus -b:a 6k -map_metadata -1 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
      }

      $results += New-Object PSObject -Property $line
    }
  }
  $results | export-csv -Path ($path_result_cmp + $ctr + ".csv") -NoTypeInformation

  ###############################################################################################################
  ## DECOMPRESSING ##############################################################################################
  ###############################################################################################################
  $results = @()

  foreach ($item in $items) {
    $item.Name
    $in = $path_original + $item.Name.replace($item.Extension, ".hc")

    $line = [ordered] @{
      Sample = $item.Name.replace($item.Extension, "")
      HC     = (Measure-Command { executables/x64_Release_Compress.exe $in -a .hc | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $path_compressed + $item.Name.replace($item.Extension, ".mp3")
    $out = $path_decompressed + $item.Name.replace($item.Extension, ".mp3.wav")
    $line += @{
      MP3 = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $path_compressed + $item.Name.replace($item.Extension, ".m4a")
    $out = $path_decompressed + $item.Name.replace($item.Extension, ".m4a.wav")
    $line += @{
      AAC = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $in = $path_compressed + $item.Name.replace($item.Extension, ".opus")
    $out = $path_decompressed + $item.Name.replace($item.Extension, ".opus.wav")
    $line += @{
      Opus = (Measure-Command { ffmpeg -loglevel quiet -hide_banner -y -i $in -ar 44100 $out | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
    }

    $results += New-Object PSObject -Property $line
  }

  $results | export-csv -Path ($path_result_dec + $ctr + ".csv") -NoTypeInformation
}

######################
## Moving .hc files ##
######################
$items = Get-ChildItem -Path $path_original | Where-Object { $_.Extension -eq ".hc" } #-Encoding UTF8

foreach ($item in $items) {
  $in = $path_original + $item.Name
  $out = $path_compressed + $item.Name

  Move-Item -Path $in -Destination $out -Force
}

###########################
## Moving .hc.wav files ##
###########################
$items = Get-ChildItem -Path $path_original | Where-Object { $_.Name.EndsWith(".hc.wav") } #-Encoding UTF8

foreach ($item in $items) {
  $in = $path_original + $item.Name
  $out = $path_decompressed + $item.Name
  Move-Item -Path $in -Destination $out -Force
}