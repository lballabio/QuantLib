
		QuantLib --- the free/open-source library for quantitative finance
		------------------------------------------------------------------
		                <http://quantlib.org>

QuantLib is Non-Copylefted Free Software.
QuantLib is OSI Certified Open Source Software.
OSI Certified is a certification mark of the Open Source Initiative.

Documentation for the QuantLib library is both online and downloadable in a
number of formats from <http://quantlib.org>.
You are going to need it. Trust us.

The project status is available at <http://quantlib.org>.
See the NEWS file for recent developments.

QuantLib depends on Boost <http://www.boost.org>. You will need to download,
build, and install Boost before compiling and using QuantLib.

Please report bugs using the Bug Tracker at
<http://sourceforge.net/tracker/?group_id=12740&atid=112740>,
submit patches using the Patch Tracker at
<http://sourceforge.net/tracker/?group_id=12740&atid=312740>, and
request features using the Feature Request Tracker at
<http://sourceforge.net/tracker/?group_id=12740&atid=362740>.

You can also use the mailing list <quantlib-users@lists.sourceforge.net> for
feedback, questions, etc. The mailing list is only for subscribers, so please
subscribe before posting

If you are interested in participating in QuantLib development, please send
mail to Ferdinando Ametrano -- nando AT ametrano DOT net -- describing your
experience and interests.



============= UNIX USERS =================

The simplest way to compile and install QuantLib is:

-# `cd' to the QuantLib directory and type `./configure' to configure the
   package for your system.  If you're using `csh' on an old version of
   System V, you might need to type `sh ./configure' instead to prevent
   `csh' from trying to execute `configure' itself.
   Running `configure' takes awhile.  While running, it prints some
   messages telling which features it is checking for.
-# Type `make' to compile the package.
-# Type `make install' to install the programs and any data files and
   documentation.
-# You can remove the program binaries and object files from the
   source code directory by typing `make clean'.  To also remove the
   files that `configure' created (so you can compile the package for
   a different kind of computer), type `make distclean'.  There is
   also a `make maintainer-clean' target, but that is intended mainly
   for the package's developers.  If you use it, you will need some GNU
   tools that usually only developers use, and which are not required to
   build QuantLib from tarballs. These are automake, autoconf, libtool, GNU
   m4, GNU make, and maybe others. They all come with recent GNU/Linux
   distributions. To begin the build process after `make distclean' start
   with `sh ./bootstrap' which will prepare the package for compilation. You
   can then use `configure' and `make' in the usual way.


GNU Libtool is used to build shared libraries.  Compilation of shared
libraries can be turned off by specifying the `--disable-shared'
option to `configure', e.g.

  ./configure --disable-shared

This is recommended if you encounter problems building the library.

The file INSTALL.txt contains more detailed instructions.


============= WIN32 USERS =================

QuantLib.dsw and QuantLib.dsp are MS VC++ 6 workspace and project files.

QuantLib.mak is the MS VC++ 6 command line makefile to be used with NMAKE, as
in: NMAKE /f "QuantLib.mak" CFG="QuantLib - Win32 Release"

QuantLib.sln and QuantLib.vcproj are MS VC++ 7.1 solution and project files.

makefile.mak is the Borland command line makefile. You can download the
Borland Free compiler from: <http://www.borland.com/bcppbuilder/freecompiler/>

QuantLib.nsi is a script file used to generate the Win32 binary installer. It
has to be used by NSIS, the great free Nullsoft Scriptable Install System,
that can be downloaded from <http://nsis.sourceforge.net>
