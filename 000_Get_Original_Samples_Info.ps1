# This script gathers information from the original wav samples at $orig_path and save them in a .csv file at $rsut_path

$orig_path = "000_original_samples/"
$rsut_path = "000_Original_Samples_Info.csv"

Function AverageHarmonicRatio($in) {
  [xml]$xml = java -jar "MPEG7_audio_encoder/MPEG7AudioEnc-0.4-rc3.jar" $in "MPEG7_audio_encoder\config.xml"

  $hr = $xml.Mpeg7.Description.MultimediaContent.Audio.AudioDescriptor.HarmonicRatio.SeriesOfScalar.Raw.Split(" ")

  # $hratios.GetType()

  [double]$RunningTotal = 0.0
  foreach ($r in $hr) {
    [double]$d = $r
    $RunningTotal += $d
  }

  return ($RunningTotal) / [double]($hr.Length)
}


$items = Get-ChildItem -Path $orig_path | Where-Object { $_.Extension -eq ".wav" }


$results = @()
foreach ($item in $items) {
  $item.Name
  $info = ffprobe -v error -show_format -show_streams ($orig_path + $item.Name)

  [double]$bytes_per_sample = [double]$info[13].Split("=")[1] / 8.0
  [int]$n = $info[20].Split("=")[1]
  [double]$harmpeg7 = AverageHarmonicRatio($orig_path + $item.Name)
  [double]$har = (python "harmonicity.py" ($orig_path + $item.Name))

  $line = [ordered] @{
    "Sample"                 = $item.Name
    "Avg Harmonicity MPEG-7" = $harmpeg7
    "Harmonicity"            = $har
    "Duration (ms)"          = [double]$info[48].Split("=")[1] * 1000
    "Size in disk (kb)"      = $item.Length #/ 1kb
    "Size (kb)"              = $info[49].Split("=")[1]
    "Theor. size (kb)"       = 44 + ($bytes_per_sample * $n) #/ 1kb
    "n (number of frames)"   = $n
    "Theoretical Bit Rate"   = $info[22].Split("=")[1]
    "Measured Bit Rate"      = [double](8 * $item.Length / $info[48].Split("=")[1])
    "Sample rate (fps)"      = $info[10].Split("=")[1]
  }
  $results += New-Object PSObject -Property $line

}
$results | export-csv -Delimiter ';' -Encoding UTF8 -Path $rsut_path -NoTypeInformation