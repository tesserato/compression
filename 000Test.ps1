
################
## Converting ##
################
$items = Get-ChildItem -Path .\original_samples | Where-Object {$_.Extension -eq ".wav"} #-Encoding UTF8



foreach ($item in $items) {
  $item.Name
  $in = "original_samples/" + $item.Name
  if ($in -notmatch "_rec" -and $in -notmatch ".m4a" -and $in -notmatch ".mp3" -and $in -notmatch ".opus"){
    executables/x64_Release_Compress.exe $in

    $compressed_name = $item.Name.replace($item.Extension,".hc")
    Move-Item -Path ("original_samples/" + $compressed_name) -Destination ("compressed_samples/" + $compressed_name) -Force

    executables/x64_Release_Compress.exe "compressed_samples/" + $compressed_name

    # $decompressed_name = 
    # Move-Item -Path "original_samples/" + $compressed_name -Destination "compressed_samples/" + $compressed_name -Force

  }
}

# ######################
# ## Moving .hc files ##
# ######################
# $items = Get-ChildItem -Path ./original_samples | Where-Object {$_.Extension -eq ".hc"} #-Encoding UTF8

# foreach ($item in $items) {
#   $in = "original_samples/" + $item.Name
#   $out = "compressed_samples/" + $item.Name

#   Move-Item -Path $in -Destination $out -Force
# }

