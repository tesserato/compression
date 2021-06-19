
################
## Converting ##
################

$name = "15_trumpet.wav"
# $name = "01_sopranoA.wav"
$in = "000_original_samples/" + $name

executables/x64_Release_Compress.exe $in -q 0

exit


$job = Start-Job -Name "job" -ScriptBlock {
  # $comp_time = (Measure-Command { executables/x64_Release_Compress.exe $in -q 1 | Out-Default } | Select-Object -Property Milliseconds)."Milliseconds"
  executables/x64_Release_Compress.exe $in -q 1
}


$job | Wait-Job -Timeout 0.1 # seconds


# Stop-Job -Name "job"

$job
# $comp_time
# $job | Where-Object {$_.State -ne "Completed"} | Stop-Job
if ($job.State -eq "Running"){
  $job| Stop-Job
  Write-Output "done"
}

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
