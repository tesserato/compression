$in = "000_original_samples/01_sopranoA.wav"

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


