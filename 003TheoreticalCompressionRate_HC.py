from os import listdir
from os.path import isfile, join
import signal_envelope as se 
import numpy as np
import os

rate = "Sample,n,m,r\n"
original_path = "000_original_samples/"

for file in listdir(original_path):
  # print(file)
  if file.endswith(".wav"):
    name = file.replace(".wav", "")
    
    W, fps = se.read_wav(original_path + name + ".wav")

    n = W.size
    m = n / np.argmax(np.abs(np.fft.rfft(W)))
    r = n / (m + 2 * (n / m))

    rate+= f"{name},{n},{m},{r}\n"



f = open("004_results_HC/Theoretical_compression_rate.csv", "w")
f.write(rate)
f.close()
