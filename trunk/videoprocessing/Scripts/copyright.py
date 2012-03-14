import os, re, shutil, errno, stat

files = filter(os.path.isfile, os.listdir('.'))

# new copyright
new_file = os.getcwd() + "\\new.txt"
new_f = open(new_file, "r")
new_content = new_f.read()
new_f.close()
print "New copyright notice:\n" + new_content

for file in files:
	print file
	if os.path.basename(file) == "copyright.py":
		print "skipping script"
		continue
	# read
	f_file = open(file, 'r')
	content = f_file.read()
	f_file.close()
	
	#replace
	expr = "COPYRIGHT((.|\n)*)DAMAGE."
	matcher = re.compile(expr)
	
	updated_content = matcher.sub(new_content, content)
	
	#write
	f_file = open(file, 'w')
	f_file.write(updated_content)
	f_file.close()
	
