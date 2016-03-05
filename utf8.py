# coding: utf8
# -------------------------------------------------
# Convert all source files to UTF-8 (without BOM)
# -------------------------------------------------
import sys
import os
import os.path
import codecs


class CFileExtFilter(object):
	def __init__(self, file_extensions):
		if isinstance(file_extensions, str):
			ext_list = file_extensions.split(";")
		else:
			ext_list = list(file_extensions)
		self.valid_exts = set(ext_list)
		print "Fiel filter:", self.valid_exts

	def __call__(self, file_name):
		return os.path.splitext(file_name)[1] in self.valid_exts


class CFileEncodingConverter(object):
	# file block size
	BLOCKSIZE = 1024 * 4

	def __init__(self, src_encoding, dst_encoding, out_path, file_filter=None):
		self.file_filter = file_filter
		self.src_encoding = src_encoding
		self.dst_encoding = dst_encoding
		self.out_path = out_path
		self.converted = 0

	def __call__(self, context, current_dir, subdir_or_files):
		print current_dir
		out_path = os.path.join(self.out_path, current_dir)
		if not os.path.exists(out_path):
			os.makedirs(out_path)
		for file_name in subdir_or_files:
			file_path = os.path.join(current_dir, file_name)
			if not os.path.isfile(file_path):
				continue
			if self.file_filter and not self.file_filter(file_path):
				continue
			self._convert(file_path, os.path.join(out_path, file_name))

	def _convert(self, src_file, dst_file):
		with codecs.open(src_file, "r", self.src_encoding) as fin:
			with codecs.open(dst_file, "w+", self.dst_encoding) as fout:
				while True:
					contents = fin.read(self.__class__.BLOCKSIZE)
					if not contents:
						break
					fout.write(contents)

def main():
	if len(sys.argv) < 4:
		print "Usage: python utf8.py src_dir src_encoding dst_dir file_type"
		return
	src_dir = os.path.normpath(sys.argv[1])
	src_encoding = sys.argv[2]
	out_dir = os.path.normpath(sys.argv[3])
	if len(sys.argv) > 4:
		file_filter = CFileExtFilter(sys.argv[4])
	else:
		file_filter = None
	converter = CFileEncodingConverter(src_encoding, "utf-8", out_dir, file_filter)
	os.path.walk(src_dir, converter, None)

if __name__ == "__main__":
	main()
