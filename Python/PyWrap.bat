@echo off
echo Generating wrappers...
swig -python -c++ -shadow -keyword -opt -I..\SWIG -o quantlib_wrap.cpp ..\SWIG\QuantLib.i

pause
