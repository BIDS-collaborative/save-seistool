# This program writes multiple stream data into one mseed file
#=============================================================

# Imports
import glob
from obspy.core import read

# Read multiple mSEED files
st = []
i = 0

# glob.glob returns a list of files in the directory
for file in glob.glob("/Users/Proxima/Documents/classes/Fall2015/hacking-measurement/save-seistool/examples/CHILE_M8.3/*.*"):
	i += 1
	if (i>5):
		break
	else:
 		st.append(read(file))


for stream in st:
	stream.write("5_trace.mseed",format="mSEED")

print("5 streams written into file 5_trace.mseed")
	