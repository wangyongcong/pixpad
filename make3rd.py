# encoding: utf-8
import os
import os.path
import time
import argparse


SOURCE_PATH = r"3rd"
BUILD_PATH = r"build"
# if INSTALL_PREFIX is not defined, default install path will be used
# INSTALL_PREFIX = ""
INSTALL_PREFIX = r"install"
INSTALL_PATHS = {
	"INSTALL_BIN_DIR": "bin",
	"INSTALL_INC_DIR": "include",
	"INSTALL_LIB_DIR": "lib",
	"INSTALL_MAN_DIR": "man",
	"INSTALL_PKGCONFIG_DIR": "pkgconfig",
}
CMAKE = r"cmake {options} {src_dir}"

PROJECT_CONFIG = {
	"zlib": {
		"version": "1.2.8",
	},
	"libpng": {
		"version": "1.6.21",
		"depends": ("zlib", ),
	},
	"openexr/IlmBase": {
		"install": os.path.join(INSTALL_PREFIX, "openexr"),
	},
	"OpenCOLLADA": {

	},
}

BUILD_ORDER = [
	"zlib",
	"libpng",
	"openexr",
	"OpenCOLLADA",
]


def call_system(cmd):
	print "[call_system]", cmd
	t1 = time.time()
	if os.system(cmd) != 0:
		raise Exception("ASSERT ERROR")
	t2 = time.time()
	print "use", t2-t1


def generate_project(script_dir, proj_name, proj_cfg):
	version = proj_cfg.get("version")
	if version:
		proj_name_full = "-".join((proj_name, version))
	else:
		proj_name_full = proj_name
	print "generating", proj_name_full
	src_dir = os.path.normpath(os.path.join(script_dir, SOURCE_PATH, proj_name_full))
	build_dir = os.path.normpath(os.path.join(script_dir, BUILD_PATH, proj_name_full))
	if not os.path.exists(build_dir):
		print "mkdir", build_dir
		os.makedirs(build_dir)
	os.chdir(build_dir)
	options = []
	# set install path
	if "install" in proj_cfg:
		install_dir = os.path.normpath(os.path.join(script_dir, proj_cfg["install"]))
		options.append(r'''-DCMAKE_INSTALL_PREFIX:PATH=%s''' % install_dir)
	elif INSTALL_PREFIX:
		install_dir = os.path.normpath(os.path.join(script_dir, INSTALL_PREFIX, proj_name))
		options.append(r'''-DCMAKE_INSTALL_PREFIX:PATH=%s''' % install_dir)
		for opt, sub_dir in INSTALL_PATHS.iteritems():
			options.append("-D%s:PATH=%s" % (opt, os.path.join(install_dir, sub_dir)))
	depends = proj_cfg.get("depends")
	if depends:
		for libname in depends:
			libpath = os.path.normpath(os.path.join(script_dir, INSTALL_PREFIX, libname))
			options.append("-DCMAKE_PREFIX_PATH=%s" % libpath)
	# apply custom options
	extra_options = proj_cfg.get("options")
	if extra_options:
		for opt, val in extra_options.iteritems():
			options.append("-D%s=%s" % (opt, val))
	# call CMake
	cmake_cmd = CMAKE.format(options=" ".join(options), src_dir=src_dir)
	call_system(cmake_cmd)


def main():
	description = "Generate 3rd projects."
	parser = argparse.ArgumentParser(description=description)
	parser.add_argument("project", type=str, help="Specify project name.", default="all")
	args = parser.parse_args()
	script_dir = os.path.dirname(os.path.realpath(__file__))
	if args.project == "all":
		for proj_name in BUILD_ORDER:
			proj_cfg = PROJECT_CONFIG[proj_name]
			generate_project(script_dir, proj_name, proj_cfg)
	else:
		proj_cfg = PROJECT_CONFIG[args.project]
		generate_project(script_dir, args.project, proj_cfg)


if __name__ == '__main__':
	main()
