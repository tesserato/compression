
################
## Converting ##
################

$paths = 
"001_ViSQOL/HC/audio_mode/",
"001_ViSQOL/HC/speech_mode/",
"001_ViSQOL/original_audio_mode/",
"001_ViSQOL/original_speech_mode/",
"002_compressed_samples/",
"003_decompressed_samples/",
"004_results/",
"004_results/Compression_time_milliseconds",
"004_results/Decompression_time_milliseconds",
"005_compressed_samples_PC_qxy/",
"006_decompressed_samples_PC_qxy/",
"007_compressed_samples_PC/",
"008_decompressed_samples_PC/",
"009_compressed_samples_codecs/",
"010_results_PC/"

foreach ($path in $paths){
  Remove-Item $path
  New-Item -ItemType Directory -Force -Path $path

}
exit

$name = "01_sopranoA.wav"
$in = "001_original_samples/" + $name

executables/x64_Release_Compress.exe $in -q 0.9


# $items = Get-ChildItem -Path "001_original_samples/" | Where-Object {$_.Extension -eq ".wav"} #-Encoding UTF8



# foreach ($item in $items) {
#   $item.Name
#   $in = "001_original_samples/" + $item.Name
#   if ($in -notmatch "_rec" -and $in -notmatch ".m4a" -and $in -notmatch ".mp3" -and $in -notmatch ".opus"){
#     executables/x64_Release_Compress.exe $in


#     $compressed_name = $item.Name.replace($item.Extension,".hc")
#     Move-Item -Path ("001_original_samples/" + $compressed_name) -Destination ("002_compressed_samples/" + $compressed_name) -Force

#     executables/x64_Release_Compress.exe ("002_compressed_samples/" + $compressed_name)

#     $decompressed_name = $item.Name.replace($item.Extension,"_reconstructed.wav")
#     Move-Item -Path ("002_compressed_samples/" + $decompressed_name) -Destination ("003_decompressed_samples/" + $decompressed_name) -Force

#   }
# }
