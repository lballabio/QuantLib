# Define some specific macros
%define _aclocaldir %{_datadir}/aclocal

# Check whether cppunit is install. If it is then
# attempt to build the test-suite package
%define test %(if rpm -q cppunit cppunit-static cppunit-devel 2>&1 > /dev/null; then echo 1; else echo 0;fi)

Summary: The free/open-source library for quantitative finance.
Name: QuantLib
Version: 0.3.3
Epoch: 0
Release: 1
License: BSD License
Group: System Environment/Libraries
Packager: Liguo Song (Leo) <liguo.song@vanderbilt.edu>
Vendor: QuantLib.org
Source0: http://prdownloads.sourceforge.net/quantlib/QuantLib-%{version}.tar.gz
URL: http://quantlib.org/
Buildroot: %{_tmppath}/%{name}-%{version}-root
BuildRequires: automake >= 1.7, autoconf >= 2.54

%description
QuantLib is an open source C++ library for financial quantitative analysts 
and developers. 

%package devel
Summary: The header files and the static library.
Group: Development/Libraries
Requires: QuantLib = %{version}
BuildRequires: automake >= 1.7, autoconf >= 2.54

%description devel
QuantLib is an open source C++ library for financial quantitative analysts 
and developers. 

Install QuantLib-devel if you are going to develop programs which will
use the standard C libraries.

%if %{test}
%package test-suite
Summary: The test-suite to check the setup of quantlib installation.
Group: Development/Tools
Requires: QuantLib = %{version}, cppunit >= 1.8.0, cppunit-devel >= 1.8.0, cppunit-static >= 1.8.0

%description test-suite
QuantLib is an open source C++ library for financial quantitative analysts 
and developers. 

The QuantLib-test-suite will validate the compiled code against pre-constructed test
cases, and helps in validating the library.
%endif

%prep
%setup -q 

%build
CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS ; \
CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS ; \
FFLAGS="${FFLAGS:-%optflags}" ; export FFLAGS ; \
./configure --prefix=%{_prefix}; \
make -j 4; \
make check

%install
rm -rf %{buildroot}
make prefix=%{buildroot}%{_prefix} install
%if !%{test}
rm -rf %{buildroot}%{_prefix}/bin/quantlib-test-suite
rm -rf %{buildroot}%{_prefix}/man/man1/quantlib-test-suite.1
echo "Warning: quantlib-test-suite is not build!"
echo "Warning: cppunit, cppunit-devel and cppunit-static are needed!"
%endif 

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files devel
%defattr(-,root,root)
%doc Examples
%{_includedir}/ql/
%{_libdir}/libQuantLib.a
%{_libdir}/libQuantLib.so
%{_datadir}/aclocal/*
%{_datadir}/emacs/site-lisp/*
%{_bindir}/quantlib-config
%{_mandir}/man1/quantlib-config.1

%if %{test}
%files test-suite
%defattr(-,root,root)
%{_mandir}/man1/quantlib-test-suite.1
%{_bindir}/quantlib-test-suite
%endif

%files
%defattr(-,root,root)
%doc Authors.txt LICENSE.TXT ChangeLog.txt Readme.txt News.txt Contributors.txt
%doc History.txt INSTALL.txt TODO.txt
%doc Docs/*
%{_libdir}/libQuantLib.la
%{_libdir}/libQuantLib.so.0
%{_libdir}/libQuantLib.so.0.0.0

%changelog
* Tue Aug 22 2003 Liguo Song <liguo.song@vanderbilt.edu>
- Initial package. Refer to Changelog.txt for previous changes.
- RPM package beta test
