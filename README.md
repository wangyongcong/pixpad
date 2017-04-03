pixpad
======

An experimental software renderer.

Build
=====

需要Visual Studio 2015 (or later)，暂未提供CMake

### 3rd libs
第三方库全部放在3rd目录下，目前依赖的第三方库有：

* zlib
* libpng
* tcplap
* OpenEXR
* OpenCollada

### Build 3rd libs

1. 安装 CMake 和 Python 2.7
2. 将相关的库源码解压到3rd目录下(zlib, libpng, tcplab)
3. 执行 `git submodule init` 和 `update` 获取openexr和opencollada的代码
4. 执行 `python make3rd.py [libname]` 创建工程, 工程文件在build目录下
5. 依次 build & install 相关的库

Roadmap
====

- ~~VS和FS多线程并行~~
- ~~基于tile的光栅化~~
- ~~测试框架~~
- ZBuffer
- Sampler
- 基于tile的内存布局
- more...
