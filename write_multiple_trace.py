# This program writes multiple stream data into one mseed file
#=============================================================

# Imports
import glob
from obspy.core import read
from obspy.core.stream import Stream
from obspy.core.trace import Trace

# Read multiple mSEED files
# st = read("/Users/Proxima/Documents/classes/Fall2015/hacking-measurement/save-seistool/examples/CHILE_M8.3/BDM.BK.LHN.00.D.2015.259.225431.gz")
st = Stream()
i = 0

# glob.glob returns a list of files in the directory
for file in glob.glob("../examples/CHILE_M8.3/*.gz"):
	i += 1
	if (i>5):
		break
	else:
 		st += read(file)

st.write("5_trace.mseed",format="mSEED")
# for stream in st:
# 	# stream.write("5_trace.mseed",format="mSEED")
# 	multiplestream += stream

# multiplestream = Stream(traces=[Trace(s) for s in st])
# multiplestream.write("5_trace_test.mseed",format="MSEED")

print("5 streams written into file 5_trace.mseed")
	