
SUBDIRS=Tests

EXTRA_DIST=README.txt extconf.rb quantlib_wrap.cpp

quantlib_wrap.cpp:
	$(SWIG) -ruby -c++ -I../SWIG -o quantlib_wrap.cpp ../SWIG/QuantLib.i

ruby: quantlib_wrap.cpp
	mv Makefile Makefile.tmp
	ruby extconf.rb
	mv Makefile Makefile.ruby
	mv Makefile.tmp Makefile
	$(MAKE) -f Makefile.ruby

ruby-install:
	$(MAKE) -f Makefile.ruby install


