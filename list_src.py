# coding: utf8

import sys
import os
import os.path

#-------------------------------------------------
# List all source files and add to CMakeLists
#-------------------------------------------------

HEADER_EXT = set((".h", ".hpp"))
SOURCE_EXT = set((".c", ".cpp"))

IDX_HDR = 0
IDX_SRC = 1

def src_filter(ret, top, names):
	top = top.replace("\\", "/")
	for name in names:
		ext = os.path.splitext(name)[1]
		if ext in HEADER_EXT:
			i = IDX_HDR
		elif ext in SOURCE_EXT:
			i = IDX_SRC
		else:
			continue
		name = "/".join((top, name))
		if not os.path.isfile(name):
			continue
		ret[i].append(name)

def main(argv):
	base = "3rd/include"
	subdirs = os.listdir(base)
	for name in subdirs:
		hdr = []
		src = []
		os.path.walk("/".join((base, name)), src_filter, (hdr, src))
		f = open(name + ".txt", "w")
		f.write("".join(("set(", name, "_src\n")))
		for v in src:
			f.write("".join(("\t", v, "\n")))
		f.write(")\n")
		f.write("".join(("set(", name, "_hdr\n")))
		for v in hdr:
			f.write("".join(("\t", v, "\n")))
		f.write(")\n")
		f.write("source_group(%s FILES ${%s_hdr} ${%s_src})" % (name, name, name))
		f.close()


if __name__ == '__main__':
	main(sys.argv[1:])
