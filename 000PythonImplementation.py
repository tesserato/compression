import numpy as np
import wave
import plotly.graph_objects as go

def read_wav(path): 
  """returns signal & fps"""
  wav = wave.open(path , 'r')
  signal = np.frombuffer(wav.readframes(-1) , np.int16)
  fps = wav.getframerate()
  return signal, fps

def maxComplex(c):
  return c[np.argmax(np.abs(c))]

def rss(s, lag):
  return np.sum(s[:-lag] * s[lag:])

def segmentSignal(s):
  t = 2.0
  d = 0.8
  lmax = 6
  N = len(s)
  FT = np.fft.rfft(s)
  k = np.argmax(np.abs(FT))
  T0 = np.ceil(N / k)
  T0over2int = int(np.ceil(T0/2))
  T = int(T0over2int + np.argmax([rss(s, lag) for lag in range(T0over2int, int(lmax * T0))]))
  nSet = []
  n = 0
  phiPrime = 0
  nPrime = 0
  insideThreshold = False
  while (n + T < N):
    phi = np.angle(maxComplex(np.fft.rfft(s[n: n + T])))
    if phi > t:
      insideThreshold = True
      if phi > phiPrime:
        phiPrime = phi
        nPrime = n
    else:
      if insideThreshold:
        phiPrime = 0
        n = int(nPrime + np.ceil(d * T))
        nSet.append(n)
      insideThreshold = False
    n += 1
  return nSet



'''==============='''
''' Read wav file '''
'''==============='''
s, fps = read_wav(f"./testSamples/11_bassoon.wav")
s = s - np.average(s)
amplitude = np.max(np.abs(s))
s = s / amplitude

'''========='''
''' Segment '''
'''========='''
nSet = segmentSignal(s)


'''===='''
'''PLOT'''
'''===='''
X = []
Y = []
for n in nSet:
  X.append(n)
  X.append(n)
  X.append(None)
  Y.append(-1)
  Y.append(1)
  Y.append(None)

FONT = dict(
    family="Latin Modern Roman",
    color="black",
    size=16
  )

'''Plotting'''
fig = go.Figure()
fig.layout.template ="plotly_white" 
fig.update_layout(
  xaxis_title="<b><i>n</i></b>",
  yaxis_title="<b>Amplitude</b>",
  legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1, itemsizing='constant'),
  margin=dict(l=0, r=0, b=0, t=0),
  font=FONT,
  titlefont=FONT
)

fig.layout.xaxis.title.font=FONT
fig.layout.yaxis.title.font=FONT

fig.update_xaxes(showline=False, showgrid=False, zeroline=False)
fig.update_yaxes(showline=True, showgrid=False, zeroline=True)

fig.add_trace(
  go.Scatter(
    name="Signal",
    # x=[0, len(T)],
    y=s,
    mode="lines",
    # line_shape='spline',
    line=dict(width=.5, color="black"),
  )
)

fig.add_trace(
  go.Scatter(
    name="frontier",
    x=X,
    y=Y,
    mode="lines",
    # line_shape='spline',
    line=dict(width=.5, color="red"),
  )
)

fig.show(config=dict({'scrollZoom': True}))