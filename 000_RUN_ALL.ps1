# This script runs all necessary 

$paths = 
"001_ViSQOL/HC/audio_mode/",
"001_ViSQOL/HC/speech_mode/",
"001_ViSQOL/original_audio_mode/",
"001_ViSQOL/original_speech_mode/",
"002_compressed_samples/",
"003_decompressed_samples/",
"004_results/",
"004_results/Compression_time_milliseconds/",
"004_results/Decompression_time_milliseconds/",
"005_compressed_samples_PC_qxy/",
"006_decompressed_samples_PC_qxy/",
"007_compressed_samples_PC/",
"008_decompressed_samples_PC/",
"009_compressed_samples_codecs/",
"010_results_PC/"

# foreach ($path in $paths){
#   $path
#   Remove-Item $path -Recurse
#   New-Item -ItemType Directory -Force -Path $path
# }

# ./001Compress_HC.ps1

# python ./001Consolidate_HC.py

./002Compress_PC_qxy.ps1

./003Compress_PC.ps1

# ./004Compress.ps1