#!/usr/bin/python3

import sys
import csv
import pandas as pd
import numpy as np
from plotly.subplots import make_subplots
import plotly.graph_objs as go

df=pd.read_csv(sys.argv[1])
signals = [("Z1", df['Z1']), ("Z2", df['Z2']), ("T7", df['T7']), ("T4", df['T4']), ("T39", df['T39']), ("Z3", df['Z3']), ("Track", df['Track'])]
time=(df['Index']*0.005)*1000000
fig_plotly = make_subplots(rows=len(signals), cols=1, shared_xaxes=True, subplot_titles=[t[0] for t in signals], vertical_spacing=0.08)
# Use 'time' as the common x-axis for all signals
for idx, (name, data) in enumerate(signals, start=1):
  fig_plotly.add_trace(go.Scatter(x=time, y=data, mode='lines', name=name), row=idx, col=1)
  fig_plotly.update_yaxes(title_text=name, row=idx, col=1)
fig_plotly.update_layout(height=600, width=1200, title_text="Interactive Signals Plot", showlegend=False)
fig_plotly.show()
