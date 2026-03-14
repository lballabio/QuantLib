@echo off
if "%1" == "" ( echo Usage: runtest exename ) else (
  %1 --log_level=message --build_info=yes --result_code=no --report_level=short ) 
