@echo off
echo Generating wrappers...
swig -python -c++ -shadow -keyword -opt -I"%QL_DIR%\SWIG" -o quantlib_wrap.cpp ..\SWIG\QuantLib.i

pause
