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


W, fps = read_wav(sys.argv[1])
W = W - np.average(W)
amp = np.max(np.abs(W))
W = W / amp
n = W.size

t0 = int(np.round(n / np.argmax(np.fft.rfft(W))))

C = []
for i in range(t0//2, 3 * t0):
  C.append(np.average(W[:n - i] * W[i:]))

C = np.array(C)
C = C / np.average(W * W)

print(f"{C.max():.20f}")