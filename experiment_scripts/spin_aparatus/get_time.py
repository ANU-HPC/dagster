import time
import sys
if (len(sys.argv)>1):
	print(time.time()-float(sys.argv[1]))
else:
	print(time.time())
