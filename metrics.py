import numpy as np
import sys
import wave


def read_wav(path):
  """returns signal & fps"""
  wav = wave.open(path , 'r')
  signal = np.frombuffer(wav.readframes(-1) , np.int16).astype(np.double)
  fps = wav.getframerate()
  return signal, fps

def save_wav(signal, name = 'test.wav', fps = 44100): 
  '''save .wav file to program folder'''
  o = wave.open(name, 'wb')
  o.setframerate(fps)
  o.setnchannels(1)
  o.setsampwidth(2)
  o.writeframes(np.int16(signal)) # Int16
  o.close()


def get_mse(W0, W1):
  a0 = np.max(np.abs(W0))
  a1 = np.max(np.abs(W1))
  # a = max(a0, a1)
  W0 = (W0.astype(float) / float(a0))
  W1 = (W1.astype(float) / float(a1))

  if W0.size > W1.size:
    W1 = np.append(W1, np.zeros( W0.size - W1.size))
  else:
    W0 = np.append(W0, np.zeros( W1.size - W0.size))

  mse = np.average((W0 - W1)**2)

  F0 = np.fft.rfft(W0) * 2 / W0.size
  F1 = np.fft.rfft(W1) * 2 / W1.size

  err = np.average(np.abs(F0 - F1))

  return mse, err


W0, fps0 = read_wav(sys.argv[1])
W1, fps1 = read_wav(sys.argv[2])

mse, err = get_mse(W0, W1)

print(f"{mse:.20f}-{err:.20f}")