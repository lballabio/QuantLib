AC_DEFUN([mytoupper],[translit([$1],[abcdefghijklmnopqrstuvwxyz.],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_])])

dnl QL_USING_STD_NAME(foo) tests the C++ for the presence of foo in
dnl   namespace std.
dnl
dnl   If foo is in namespace std, QL_USING_STD_FOO is #defined to
dnl      using std::foo .
dnl
dnl   The line `#define QL_USING_STD_FOO ' (with trailing space )
dnl      must appear in acconfig.h
dnl 
dnl   This macro assumes foo is in standard header <foo> .

AC_DEFUN(
  [QL_USING_STD_NAME],
  [
    AC_MSG_CHECKING(if C++ environment has $1 in std.)
    AC_TRY_LINK(
    [
    #include <$1>

    using std::$1;

    ],[
      ;
    ],[
      ql_$1_in_std=yes
      AC_DEFINE(mytoupper(QL_USING_STD_$1),[using std::$1;])
    ],[
      ql_$1_in_std=no
      AC_DEFINE(mytoupper(QL_USING_STD_$1),)
    ])
    AC_MSG_RESULT([$ql_$1_in_std])
  ])

dnl QL_FUN_IN_STD_HEADER(foo,bar,arg) tests the C++ for the
dnl   presence of foo in namespace std.
dnl
dnl   If foo is in namespace std, QL_FOO is #defined to
dnl      std::foo .
dnl   arg is a valid argument for foo.
dnl
dnl   The line `#undef QL_FOO'
dnl      must appear in acconfig.h
dnl 
dnl   This macro assumes foo is in standard header <bar> .

AC_DEFUN(
  [QL_FUN_IN_STD_HEADER],
  [
    AC_MSG_CHECKING(if C++ environment has $1 in std)
    AC_TRY_COMPILE(
    [#include<$2>],
    [
    double dv;
    std::$1($3);
    ],[
      ql_$1_in_std=yes
      AC_DEFINE(mytoupper(QL_$1),[std::$1])
    ],[
      ql_$1_in_std=no
      AC_DEFINE(mytoupper(QL_$1),[$1])
    ])
    AC_MSG_RESULT([$ql_$1_in_std])
  ])

AC_DEFUN(
	[QL_TMPL_IN_STD_HEADER],
	[
		AC_MSG_CHECKING(if C++ environment has $1 in std)
		AC_TRY_COMPILE(
			[#include <$3>],
			[
				std::$1<$2>($4);
			],[
				ql_$1_in_std=yes
				AC_DEFINE(mytoupper(QL_$1),[std::$1])
			],[
				ql_$1_in_std=no
				AC_DEFINE(mytoupper(QL_$1),[$1])
			])
		AC_MSG_RESULT([$ql_$1_in_std])
	])

