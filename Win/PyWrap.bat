@echo off
echo Generating wrappers...
swig1.3a5 -python -c++ -shadow -keyword -opt -I..\SWIG -o ..\Python\quantlib_wrap.cpp ..\SWIG\QuantLib.i
copy QuantLib.py ..\Python\QuantLib.py
del QuantLib.py

pause
