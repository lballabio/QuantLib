AC_DEFUN([mytoupper],[translit([$1],[abcdefghijklmnopqrstuvwxyz.],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_])])

dnl QL_CHECK_FUNCTION(foo,bar,arg) tests for the
dnl   presence of foo in namespace std or in the global 
dnl   namespace.
dnl
dnl   QL_FOO is #defined as foo with full namespace 
dnl     specification if foo is found.
dnl   arg is a valid argument for foo.
dnl
dnl   The line `#undef QL_FOO'
dnl      must appear in acconfig.h
dnl 
dnl   This macro assumes foo is in standard header <bar> .

AC_DEFUN(
  [QL_CHECK_FUNCTION],
  [
    AC_MSG_CHECKING(if C++ environment has $1)
    AC_TRY_COMPILE(
    [#include<$2>],
    [
    double x;
    char s[10];
    std::$1($3);
    ],[
      result="found in std"
      AC_DEFINE(mytoupper(QL_$1),[std::$1])
    ],[
      AC_TRY_COMPILE(
      [#include<$2>],
      [
      double x;
      $1($3);
      ],[
        result=found
        AC_DEFINE(mytoupper(QL_$1),[$1])
      ],[
        result="not found"
      ])
    ])
    AC_MSG_RESULT([$result])
  ])


dnl QL_CHECK_TYPENAME(foo,bar,arg) tests for the
dnl   presence of foo in namespace std or in the global
dnl   namespace.
dnl
dnl   QL_FOO is #defined as foo with full namespace
dnl     specification if foo is found.
dnl   arg is a valid argument for a foo constructor.
dnl
dnl   The line `#undef QL_FOO'
dnl      must appear in acconfig.h
dnl
dnl   This macro assumes foo is in standard header <bar> .

AC_DEFUN(
  [QL_CHECK_TYPENAME],
  [
    AC_MSG_CHECKING(if C++ environment has $1)
    AC_TRY_COMPILE(
    [#include<$2>],
    [
    std::$1 x = std::$1($3);
    ],[
      result="found in std"
      AC_DEFINE(mytoupper(QL_$1),[std::$1])
    ],[
      AC_TRY_COMPILE(
      [#include<$2>],
      [
      $1 x = $1($3);
      ],[
        result=found
        AC_DEFINE(mytoupper(QL_$1),[$1])
      ],[
        result="not found"
      ])
    ])
    AC_MSG_RESULT([$result])
  ])


dnl QL_CHECK_TEMPLATE_CLASS(foo,types,bar,arg) tests for the
dnl   presence of foo<types> in namespace std or in the global
dnl   namespace.
dnl
dnl   QL_FOO is #defined as foo with full namespace
dnl     specification if foo is found.
dnl   arg is a valid argument for a foo constructor.
dnl
dnl   The line `#undef QL_FOO'
dnl      must appear in acconfig.h
dnl
dnl   This macro assumes foo is in standard header <bar> .

AC_DEFUN(
  [QL_CHECK_TEMPLATE_CLASS],
  [
    AC_MSG_CHECKING(if C++ environment has $1)
    AC_TRY_COMPILE(
    [#include<$3>],
    [
    std::$1<$2> x = std::$1<$2>($4);
    ],[
      result="found in std"
      AC_DEFINE(mytoupper(QL_$1),[std::$1])
    ],[
      AC_TRY_COMPILE(
      [#include<$3>],
      [
      $1<$2> x = $1<$2>($4);
      ],[
        result=found
        AC_DEFINE(mytoupper(QL_$1),[$1])
      ],[
        result="not found"
      ])
    ])
    AC_MSG_RESULT([$result])
  ])



