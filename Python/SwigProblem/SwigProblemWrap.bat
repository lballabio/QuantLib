@echo off
echo Generating wrappers...
swig -python -c++ -shadow -keyword -opt -o mylib_wrap.cpp MyLib.i

pause
