# This file takes a single seismic data file and tries to analyze it
#====================================================================

# Import libraries
import glob
from obspy.core import read

# Read miniSEED data file
st = read("BDM.BK.LHE.00.D.2015.259.225431",format="mSEED")
# Print stream. A stream is an object that contains all data from a miniSEED file.
# A stream is a collection of traces.
print("Printing stream")
print(st)
# Print length of stream. This is equal to the number of traces in the data file.
print("Length of stream")
print(len(st))
# Print elements of stream. This stream has only 1 trace.
print("First element of stream")
tr = st[0]
print(tr)
# Access meta-data
print ("Accessing meta-data")
print(tr.stats)
# Access waveform data
print ("Accessing waveform data")
print(tr.data)
print ("Length of waveform")
print(len(tr.data))
# Data preview
st.plot()