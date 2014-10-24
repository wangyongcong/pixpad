# coding: utf-8
import sys
import os
import os.path
import struct

# name        type        number of bytes
# ---------------------------------------
# char       character                 1
# uchar      unsigned character        1
# short      short integer             2
# ushort     unsigned short integer    2
# int        integer                   4
# uint       unsigned integer          4
# float      single-precision float    4
# double     double-precision float    8
PROPERTY_TYPE = {
	"char": ("b", 1),
	"uchar": ("B", 1),
	"short": ("h", 2),
	"ushort": ("H", 2),
	"int": ("i", 4),
	"uint": ("I", 4),
	"float": ("f", 4),
	"double": ("d", 8),
}


class CElement(object):
	
	def __init__(self, name, cnt):
		self.name = name
		self.cnt = cnt
		self.props = []
		self.format = ""
		self.size = 0
		self.is_list = False
		self.data = None


def read(path):
	try:
		f = open(path, "rb")
	except IOError:
		print "[ERROR] Can't open file:", path
		return None
	# HEADER
	s = f.readline().rstrip()
	if s != "ply":
		f.close()
		print "[ERROR] It's not a valid PLY file"
		return None
	s = f.readline().rstrip()
	print s
	if not s.startswith("format"):
		f.close()
		print "[ERROR] Unknown PLY file format", s
		return None
	val = s.split(" ")
	if len(val) < 3:
		f.close()
		print "[ERROR] Unknown PLY file format", s
		return None
	fmt = val[1]
	# version = val[2]
	if not fmt.startswith("binary"):
		f.close()
		print "[ERROR] Only binary PLY file is supported", s
		return None
	endian = fmt[7:]
	if endian.startswith("little"):
		is_little_endian = True
		prefix = "<"
	elif endian.startswith("big"):
		is_little_endian = False
		prefix = ">"
	elements = []
	cur_elem = None
	max_headers = 64
	cnt = 0
	s = f.readline().rstrip()
	while s and s != "end_header":
		s = s.decode("utf-8")
		print s
		if s.startswith("element"):
			val = s.split(" ")
			cur_elem = CElement(val[1], int(val[2]))
			cur_elem.format = prefix
			elements.append(cur_elem)
		elif s.startswith("property"):

			val = s.split(" ")
			prop_type = val[1]
			prop_name = val[-1]
			cur_elem.props.append((prop_name, prop_type))
			if prop_type == "list":
				size_type, elem_type = val[2], val[3]
				fmt1, size1 = PROPERTY_TYPE[size_type]
				fmt2, size2 = PROPERTY_TYPE[elem_type]
				cur_elem.format = (prefix + fmt1, size1, "".join((prefix, "%d", fmt2)), size2)
				cur_elem.is_list = True
			elif prop_type in PROPERTY_TYPE and not cur_elem.is_list:
				fmt, size = PROPERTY_TYPE[prop_type]
				cur_elem.format += fmt
				cur_elem.size += size
			else:
				f.close()
				print "[ERROR] Unknown property format:", prop_type, prop_name
				return None
		cnt += 1
		if cnt >= max_headers:
			f.close()
			print "[ERROR] Too many headers"
			return None
		s = f.readline().rstrip()
	for cur_elem in elements:
		print cur_elem.name, "(%d)" % cur_elem.cnt
		for prop_name, prop_type in cur_elem.props:
			print "\t", prop_name, prop_type
		if cur_elem.is_list:
			fmt1, size1, fmt2, size2 = cur_elem.format
			s = f.read(size1)
			cnt = struct.unpack(fmt1, s)[0]
			fmt2 = fmt2 % cnt
			size2 *= cnt
			print "\ttData: '%s' (%d Bytes)" % (fmt2, size2)
			s = f.read(size2)
			if len(s) < size2:
				f.close()
				print "[ERROR] Bad list data!"
				return None
			cur_elem.data = struct.unpack(fmt2, s)
			assert len(cur_elem.data) == cnt
		else:
			print "\tData: '%s' (%d Bytes)" % (cur_elem.format, cur_elem.size)
			assert struct.calcsize(cur_elem.format) == cur_elem.size
			cur_elem.data = []
			for i in xrange(cur_elem.cnt):
				s = f.read(cur_elem.size)
				if len(s) < cur_elem.size:
					f.close()
					print "[ERROR] Bad data!"
					return None
				cur_elem.data.append(struct.unpack(cur_elem.format, s))
	f.close()
	print "EOF"
	return elements


if __name__ == "__main__":
	read("torus.ply")
