
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

namespace QuantLib {

	class Error : public std::exception {
	  public:
		// constructors
		explicit Error(const std::string& what = "") : message(what) {}
		const char* what() const { return message.c_str(); }
	  protected:
		std::string message;
	};
	
	void Assert(bool condition, const std::string& description);
	void Require(bool condition, const std::string& description);
	void Ensure(bool condition, const std::string& description);
	
	// specialized errors
	
	class AssertionFailedError : public Error {
	  public:
		explicit AssertionFailedError(const std::string& what = "")
		: Error(what) {}
	};
		
	class IllegalArgumentError : public Error {
	  public:
		explicit IllegalArgumentError(const std::string& what = "")
		: Error(what) {}
	};
	
	class IllegalResultError : public Error {
	  public:
		explicit IllegalResultError(const std::string& what = "")
		: Error(what) {}
	};
	
	class OutOfMemoryError : public Error {
	  public:
		explicit OutOfMemoryError(const std::string& whatClass = "unknown class")
		: Error(whatClass+": out of memory") {}
	};
	
	
	// inline definitions
	
	inline void Assert(bool condition, const std::string& description) {
		if (!condition)
			throw AssertionFailedError(description);
	}
	
	inline void Require(bool condition, const std::string& description) {
		if (!condition)
			throw IllegalArgumentError(description);
	}
	
	inline void Ensure(bool condition, const std::string& description) {
		if (!condition)
			throw IllegalResultError(description);
	}

}


#endif

