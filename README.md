# Linux apps for Windows

## Introduction

This project features copies of Linux programs for the Windows command line.

## Usage

Make sure you have [MinGW](https://sourceforge.net/projects/mingw/) installed and added *gcc.exe* to PATH.  

You can build the binaries yourself or use the batch file which will create a new directory which holds the executables. 

---

Custom build -> build.bat [Name of the apps without .c space seperated]
```batch
build.bat cat touch
```
Build all at once
```
build.bat all
```