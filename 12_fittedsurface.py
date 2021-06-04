# from os import linesep
# from statistics import mode
import numpy as np
from numpy.core.function_base import linspace
import plotly.graph_objects as go
import signal_envelope as se
from scipy import interpolate
# from plotly.subplots import make_subplots
import scipy
import time



'''###### Read Pseudo-cycles info ######'''
X = np.genfromtxt(f"X.csv", delimiter=",")
Y = np.genfromtxt(f"Y.csv", delimiter=",")
Z = np.genfromtxt(f"Z.csv", delimiter=",")


# X = []
# Y = []
# Z = []
# # S = np.zeros((Xpc.size - 1, (Xpc[1:] - Xpc[:-1]).max()))
# for i in range(1, Xpc.size):
#   x0 = Xpc[i - 1]
#   x1 = Xpc[i]
#   w = W[x0 : x1] #/ np.abs(W[x0 : x1]).max()
#   l = len(w) 
#   X += (np.zeros(l) + i - 1).tolist()
#   Y += np.linspace(0, 1, l, endpoint=True).tolist()
#   Z += w.tolist()
#   # S[i - 1, : l] = w

# X = np.array(X).flatten() # position in the cycle
# Z = np.array(Z).flatten()
# Y = np.array(Y).flatten()
# X = X / X[-1]


# start = time.time()
# tx = np.linspace(0, 1, 120)
# ty = np.linspace(0, 1, 80)
# tck = scipy.interpolate.bisplrep(X, Y, Z, kx=5, ky=5, task=-1, tx=tx, ty=ty)
# end = time.time() - start
# print(f"\nEncoding time={end * 1000} milliseconds")

# txr, tyr, c, kx, ky = tck

# nlen = len(c) + len(Xpc)
# print(f"n = {n}, pseudo cycles={len(Xpc)}, coefs={len(c)}, rate={n / nlen}")


# Xp = []
# Yp = []
# Zp = []
# for x in linspace(0, 1):
#   for y in np.linspace(0, 1):
#     Xp.append(x)
#     Yp.append(y)
#     Zp.append(interpolate.bisplev(x, y, tck))

# start = time.time()
# Znew = []
# for x, y in zip(X, Y):
#   Znew.append(interpolate.bisplev(x, y, tck))
# Znew = [0 for _ in range(Xpc[0])] + Znew + [0 for _ in range(n - Xpc[-1])]
# Znew = np.array(Znew).flatten()
# end = time.time() - start
# print(f"Decoding time={end * 1000} milliseconds")

# mse = np.average((W - Znew)**2)
# print(f"MSE={mse}")
# se.save_wav(Znew / np.abs(Znew).max() * amp, f"tests/fitted/{name}_surface.wav")

# exit()

'''============================================================================'''
'''                               PLOT WAVEFORM                                '''
'''============================================================================'''

FONT = dict(
    family="Latin Modern Roman",
    color="black",
    size=16
  )

eye_x = 3
eye_y = 1.5
eye_z = 1.4
zoom  = 4.3

modulus = np.sqrt(eye_x**2 + eye_y**2 + eye_z**2)

eye_x = zoom * eye_x / modulus
eye_y = zoom * eye_y / modulus
eye_z = zoom * eye_z / modulus

fig = go.Figure()

scene = {
  'aspectmode': 'manual',
            'aspectratio': {'x': 4, 'y': 1, 'z': 1},
            'camera': {'center': {'x': 0, 'y': -.8, 'z': -1.2},
                        'eye': {'x': eye_x, 'y': eye_y, 'z': eye_z}},
            # 'domain': {'x': [0.0, 0.45], 'y': [0.0, 1.0]},
            'xaxis': {'autorange': 'reversed',
                      'dtick': 0.1,
                      'gridcolor': 'black',
                      'gridwidth': 1,
                      # "range":[0, 1],
                      'showgrid': True,
                      'title': {'text': '<b>Pseudo cycle <i>j</i></b>'},
                      'zerolinecolor': 'black',
                      'zerolinewidth': 4},
            'yaxis': {'gridcolor': 'black',
                      'gridwidth': 1,
                      # "range":[0, 1],
                      'showgrid': True,
                      'title': {'text': '<b>Pseudo cycle index <i>k</i></b>'},
                      'zerolinecolor': 'black',
                      'zerolinewidth': 4},
            'zaxis': {'gridcolor': 'black',
                      'gridwidth': 1,
                      # "range":[-1, 1],
                      'showgrid': True,
                      'title': {'text': '<b>Amplitude</b>'},
                      'zerolinecolor': 'black',
                      'zerolinewidth': 4}
}

fig.layout.template ="plotly_white" 
fig.update_layout(
  # xaxis_title="<b>Pseudo cycle</b>",
  # yaxis_title="<b>Frame</b>",
  legend=dict(orientation='h', yanchor='top', xanchor='left', y=1.1, itemsizing='constant'),
  margin=dict(l=0, r=0, b=0, t=0),
  font=FONT,
  titlefont=FONT,
)

fig.layout["scene"] = scene

fig.layout.xaxis.title.font=FONT
fig.layout.yaxis.title.font=FONT

fig.update_xaxes(showline=False, showgrid=False, zerolinewidth=1, zerolinecolor='black')
fig.update_yaxes(showline=False, showgrid=False, zerolinewidth=1, zerolinecolor='black')

# X = np.linspace(0, 1, S.shape[1], endpoint=False)
# Y = np.linspace(0, 1, S.shape[0], endpoint=False)



fig.add_trace(
  go.Scatter3d(
    name="original",
    x=X,
    y=Y,
    z=Z,
    showlegend=True,
    mode="markers",
    marker=dict(
      size=.8,
      color=Z,                # set color to an array/list of desired values
      colorscale='viridis',   # choose a colorscale
      opacity=.8
    )
  )
)

# fig.add_trace(
#   go.Mesh3d(
#     name="Fitted",
#     x=Xp,
#     y=Yp,
#     z=Zp,
#     showlegend=True,
#     intensity=Zp,
#     colorscale='Burgyl_r',
#     showscale=False,
#     cmax=1,
#     cmin=-1,
#     opacity=1
#   )
# )

import sys
fig.show(config=dict({'scrollZoom': True}))
# save_name = sys.argv[0].split('/')[-1].replace(".py", "") + "_" + name
# wid = 600
# hei = 500
# fig.write_image("./paper/images/" + save_name + ".svg", width=wid, height=hei, format="svg")
# fig.write_image("./site/public/images/" + save_name + ".webp", width=int(1.7*wid), height=int(1.5*hei), format="webp")
# fig.write_html("./site/public/images/" + save_name + ".html", include_plotlyjs="cdn", include_mathjax="cdn")
# print("saved:", save_name)