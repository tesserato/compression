
$orig_path = "000_original_samples/"
$rsut_path = "010_results_PC/Original_Samples_Info.csv"

$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }


$results = @()
foreach ($item in $items) {
  $item.Name
  # $path = $orig_path + $item.Name
  # $orig_size = $item.Length / 1kb
  $info = ffprobe -v error -show_format -show_streams ($orig_path + $item.Name)

  [double]$bytes_per_sample = [double]$info[13].Split("=")[1] / 8.0
  $bytes_per_sample
  [double]$n = $info[20].Split("=")[1]

  # $theoretical_size = ($bits_per_sample * $n) / 1kb

  # 1kb
  exit

  $line = [ordered] @{
    Sample              = $item.Name
    "Duration (ms)"     = [double]$info[48].Split("=")[1] * 1000
    "Size in disk (kb)" = $item.Length #/ 1kb
    "Size (kb)"         = $info[49].Split("=")[1]
    "Theor. size (kb)"  = 44 + ($bytes_per_sample * $n) #/ 1kb
    "n"                 = $n
    "Bit Rate"          = $info[22].Split("=")[1]
    "Sample rate (fps)" = $info[10].Split("=")[1]
  }
  $results += New-Object PSObject -Property $line



}
$results | export-csv -Path $rsut_path -NoTypeInformation