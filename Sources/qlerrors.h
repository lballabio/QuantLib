
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_error_h
#define quantlib_error_h

#include "qldefines.h"
#include <exception>
#include <string>

QL_BEGIN_NAMESPACE(QuantLib)

class Error : public std::exception {
  public:
	// constructors
	explicit Error(std::string what = "") 
	: std::exception(what.c_str()) {}
};

void Assert(bool condition, std::string description);
void Require(bool condition, std::string description);
void Ensure(bool condition, std::string description);

// specialized errors

class AssertionFailedError : public Error {
  public:
	explicit AssertionFailedError(std::string what = "")
	: Error(what) {}
};
	
class IllegalArgumentError : public Error {
  public:
	explicit IllegalArgumentError(std::string what = "")
	: Error(what) {}
};

class IllegalResultError : public Error {
  public:
	explicit IllegalResultError(std::string what = "")
	: Error(what) {}
};

class OutOfMemoryError : public Error {
  public:
	explicit OutOfMemoryError(std::string whatClass = "unknown class")
	: Error(whatClass+": out of memory") {}
};


// inline definitions

inline void Assert(bool condition, std::string description) {
	if (!condition)
		throw AssertionFailedError(description);
}

inline void Require(bool condition, std::string description) {
	if (!condition)
		throw IllegalArgumentError(description);
}

inline void Ensure(bool condition, std::string description) {
	if (!condition)
		throw IllegalResultError(description);
}

QL_END_NAMESPACE(QuantLib)


#endif

