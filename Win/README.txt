The makefile is to be used with the free Borland C++ Compiler available at
http://www.borland.com/bcppbuilder/freecompiler/

Visual C++ 6.0 projects files are also supplied, but the compiled DLL does not 
work due to, probably, a bug in the compiler. In particular, broken object
code is generated for the BSMAmericanPricer class which implies that the
python test Python/Tests/greeks_in_american.py fails.
Please let us know whether it works on your configuration, or if you 
are aware of any workarounds.

