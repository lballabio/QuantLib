
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

class Error : public QL_ADD_NAMESPACE(std,exception) {
  public:
	// constructors
	explicit Error(const QL_ADD_NAMESPACE(std,string)& what = "") : message(what) {}
	const char* what() const { return message.c_str(); }
  protected:
	QL_ADD_NAMESPACE(std,string) message;
};

void Assert(bool condition, const QL_ADD_NAMESPACE(std,string)& description);
void Require(bool condition, const QL_ADD_NAMESPACE(std,string)& description);
void Ensure(bool condition, const QL_ADD_NAMESPACE(std,string)& description);

// specialized errors

class AssertionFailedError : public Error {
  public:
	explicit AssertionFailedError(const QL_ADD_NAMESPACE(std,string)& what = "")
	: Error(what) {}
};
	
class IllegalArgumentError : public Error {
  public:
	explicit IllegalArgumentError(const QL_ADD_NAMESPACE(std,string)& what = "")
	: Error(what) {}
};

class IllegalResultError : public Error {
  public:
	explicit IllegalResultError(const QL_ADD_NAMESPACE(std,string)& what = "")
	: Error(what) {}
};

class OutOfMemoryError : public Error {
  public:
	explicit OutOfMemoryError(const QL_ADD_NAMESPACE(std,string)& whatClass = "unknown class")
	: Error(whatClass+": out of memory") {}
};


// inline definitions

inline void Assert(bool condition, const QL_ADD_NAMESPACE(std,string)& description) {
	if (!condition)
		throw AssertionFailedError(description);
}

inline void Require(bool condition, const QL_ADD_NAMESPACE(std,string)& description) {
	if (!condition)
		throw IllegalArgumentError(description);
}

inline void Ensure(bool condition, const QL_ADD_NAMESPACE(std,string)& description) {
	if (!condition)
		throw IllegalResultError(description);
}

QL_END_NAMESPACE(QuantLib)


#endif

