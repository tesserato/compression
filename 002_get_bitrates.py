import glob
import os
from pydub.utils import mediainfo

sample_names = [n.split("\\")[-1].replace(".wav","") for n in glob.glob("./000_original_samples/*.wav")]

print(sample_names)

csv = "Sample,Wav bps,HC bps,AAC bps,MP3 bps,Opus bps\n"
for name in sample_names:
  wav_size  = os.path.getsize(f"./000_original_samples/{name}.wav")
  hc_size   = os.path.getsize(f"./002_compressed_samples_HC/{name}.hc")
  m4a_size  = os.path.getsize(f"./002_compressed_samples_HC/{name}.m4a")
  mp3_size  = os.path.getsize(f"./002_compressed_samples_HC/{name}.mp3")
  opus_size = os.path.getsize(f"./002_compressed_samples_HC/{name}.opus")
  

  wav_info = mediainfo(f"./000_original_samples/{name}.wav")
  # wav_kbps = 8 * wav_size / float(wav_info['duration'])

  hc_kbps = int(8 * hc_size / float(wav_info['duration']))

  m4a_info = mediainfo(f"./002_compressed_samples_HC/{name}.m4a")
  # m4a_kbps = 8 * m4a_size / float( m4a_info['duration'])

  mp3_info = mediainfo(f"./002_compressed_samples_HC/{name}.mp3")
  # mp3_kbps = 8 * mp3_size / float( mp3_info['duration'])

  opus_info = mediainfo(f"./002_compressed_samples_HC/{name}.opus")
  # print(opus_info)

  # exit()
  csv+=f"{name},{wav_info['bit_rate']},{hc_kbps},{m4a_info['bit_rate']},{mp3_info['bit_rate']},{opus_info['bit_rate']}\n"

with open('./004_results_HC/Bitrates.csv', 'w') as f:
  f.write(csv)