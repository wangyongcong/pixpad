# coding: utf8
import argparse
import os
import os.path
import re

#-------------------------------------------------
# List all source files and add to CMakeLists
#-------------------------------------------------

HEADER_EXT = {".h", ".hpp"}
SOURCE_EXT = {".h", ".hpp", ".c", ".cpp", ".cxx"}


def replace_text(text, rep_map):
	rep = dict((re.escape(k), v) for k, v in rep_map.items())
	pattern = re.compile("|".join(rep.keys()))
	result = pattern.sub(lambda m: rep[re.escape(m.group(0))], text)
	return result

# list all source files in sub directories
def list_files(src_path, *lst_subdir):
	src_lst = []
	if not lst_subdir:
		for fname in os.listdir(src_path):
			ext = os.path.splitext(fname)[1]
			if ext in SOURCE_EXT:
				src_lst.append('"%s"' % fname)
	else:
		for subdir in lst_subdir:
			path = "/".join([src_path, subdir])
			if not os.path.isdir(path):
				print("direcotry not found", path)
				continue
			for fname in os.listdir(path):
				ext = os.path.splitext(fname)[1]
				if ext in SOURCE_EXT:
					src_lst.append('"%s"' % "/".join([subdir, fname]))
	src_lst.sort()
	return "\n\t".join(src_lst)


# function(dir_to_search, *list_of_subdirs) -> string_to_replace_with
CMAKE_FUNCTIONS = {
	"LIST_FILES": list_files
}


def parse_cmake(makefile, src_path):
	f = open(makefile, "r")
	full_text = f.read()
	f.close()
	src_path = src_path.replace("\\", "/")
	pat = re.compile('\"\$(\w+\([\w\s,]*\))\"')
	all_calls = pat.findall(full_text)
	rep_map = {}
	for func_call in all_calls:
		sep = func_call.find("(")
		func_name = func_call[:sep]
		func_args = func_call[sep + 1:-1]
		handler = CMAKE_FUNCTIONS.get(func_name)
		if handler:
			if func_args:
				args = filter(None, [v.strip() for v in func_args.split(",")])
				ret = handler(src_path, *args)
			else:
				ret = handler(src_path)
			rep_map['"$%s"' % func_call] = ret
	if rep_map:
		full_text = replace_text(full_text, rep_map)
		with open(makefile, "w") as fout:
			fout.write(full_text)


def main():
	parser = argparse.ArgumentParser(description="Add source files to CMakeLists")
	parser.add_argument("src_dir", type=str, help="source directory that contains CMakeLists.txt")
	args = parser.parse_args()
	
	makefile = os.path.join(args.src_dir, "CMakeLists.txt")
	if not os.path.isfile(makefile):
		print("[ERROR] can't find CMakeLists.txt")
		return
	parse_cmake(makefile, args.src_dir)


if __name__ == '__main__':
	main()
