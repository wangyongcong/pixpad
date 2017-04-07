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
		self.list_format = None
		self.data = None


def read(path, header_only = False):
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
				cur_elem.list_format = (prefix + fmt1, size1, "".join((prefix, "%d", fmt2)), size2)
				cur_elem.format += fmt2
				cur_elem.size = size2
			elif prop_type in PROPERTY_TYPE and not cur_elem.list_format:
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
	if header_only:
		f.close()
		return None

	ply = {}
	for cur_elem in elements:
		print cur_elem.name, "(%d)" % cur_elem.cnt
		for prop_name, prop_type in cur_elem.props:
			print "\t", prop_name, prop_type
		if cur_elem.list_format:
			fmt1, size1, fmt2, size2 = cur_elem.list_format
			s = f.read(size1)
			cnt = struct.unpack(fmt1, s)[0]
			fmt2 = fmt2 % cnt
			size2 *= cnt
			print "\tData: '%s' (%d Bytes)" % (fmt2, size2)
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
		ply[cur_elem.name] = cur_elem
	f.close()
	print "EOF"
	return ply


def save_raw(path, ply_data):
	print "save to", path
	f = open(path, "wb")
	vertex = ply_data.get("vertex", None)
	inf = float("inf")
	# x_min, y_min, z_min, x_max, y_max, z_max
	bounding = [inf, inf, inf, -inf, -inf, -inf]
	if vertex:
		cnt = vertex.cnt
		size= vertex.size
		f.write(struct.pack("<II", cnt, size))
		fmt = "<" + vertex.format[1:]
		for v in vertex.data:
			x, y, z = v[0], v[1], v[2]
			if x < bounding[0]:
				bounding[0] = x
			if y < bounding[1]:
				bounding[1] = y
			if z < bounding[2]:
				bounding[2] = z
			if x > bounding[3]:
				bounding[3] = x
			if y > bounding[4]:
				bounding[4] = y
			if z > bounding[5]:
				bounding[5] = z
			f.write(struct.pack(fmt, *v))
	else:
		f.write(struct.pack("<II", 0, 0))
	tristrips = ply_data.get("tristrips", None)
	if tristrips:
		cnt = len(tristrips.data)
		size = tristrips.size
		f.write(struct.pack("<II", cnt, tristrips.size))
		fmt = "<" + tristrips.list_format[2][1:] % cnt
		f.write(struct.pack(fmt, *tristrips.data))
	else:
		f.write(struct.pack("<II", 0, 0))
	f.write(struct.pack("<ffffff", *bounding))
	f.close()
	print "bounding box:", bounding


def main(argv):
	doc = '''
Usage: 
    python ply.py *.ply
Options:
    --help: show help
    --header: parse header only
    --out: output raw data
    '''
	options = set()
	for v in argv:
		if v.startswith("--"):
			options.add(v[2:])
	if "help" in options:
		print doc
		return
	try:
		path = argv[1]
	except:
		print doc
		return
	if not os.path.isfile(path):
		print "File not found:", path
		return
	is_header_only = "header" in options
	ply_data = read(path, is_header_only)
	if is_header_only:
		return
	if "out" in options and ply_data:
		out = "".join((os.path.splitext(path)[0], ".mesh"))
		save_raw(out, ply_data)


if __name__ == "__main__":
	main(sys.argv)
