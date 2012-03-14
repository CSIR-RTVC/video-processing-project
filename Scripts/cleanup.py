import os, re

ext_list = ["obj", "idb", "manifest", "pdb", "ncb"]	

total_size_bytes = 0
for dirname, dirnames, filenames in os.walk('.'):
    #for subdirname in dirnames:
        #print os.path.join(dirname, subdirname);
    for filename in filenames:
		for ext in ext_list:
			expr = "^[a-zA-Z0-9\s]+." + ext + "$" 
			matcher = re.compile(expr)
			if matcher.match(filename):
				statinfo = os.stat(os.path.join(dirname, filename))
				total_size_bytes += statinfo.st_size
				print "Deleting: " + os.path.join(dirname, filename) + " " + str(statinfo.st_size) + " Bytes"
				os.remove(os.path.join(dirname, filename))
print "Total: " + str(total_size_bytes/1024) + " KB (" + str(total_size_bytes) + " Bytes)"
