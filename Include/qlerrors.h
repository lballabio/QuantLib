
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/


/*! \file qlerrors.h
	\brief Classes and functions for error handling.
*/

#ifndef quantlib_error_h
#define quantlib_error_h

#include "qldefines.h"
#include <exception>
#include <string>

namespace QuantLib {

	//! Base error class
	class Error : public std::exception {
	  public:
		explicit Error(const std::string& what = "") : message(what) {}
		//! returns the error message.
		const char* what() const { return message.c_str(); }
	  private:
		std::string message;
	};
	
	/*! It throws an error if the given condition is not verified
		\relates Error
	*/
	void Assert(bool condition, const std::string& description);
	/*! It throws an error if the given pre-condition is not verified
		\relates Error
	*/
	void Require(bool condition, const std::string& description);
	/*! It throws an error if the given post-condition is not verified
		\relates Error
	*/
	void Ensure(bool condition, const std::string& description);
	
	
	//! Specialized error
	/*! Thrown upon a failed assertion.
	*/
	class AssertionFailedError : public Error {
	  public:
		explicit AssertionFailedError(const std::string& what = "")
		: Error(what) {}
	};
		
	//! Specialized error
	/*! Thrown upon passing an argument with an illegal value.
	*/
	class IllegalArgumentError : public Error {
	  public:
		explicit IllegalArgumentError(const std::string& what = "")
		: Error(what) {}
	};
	
	//! Specialized error
	/*! Thrown upon obtaining a result outside the allowed range.
	*/
	class IllegalResultError : public Error {
	  public:
		explicit IllegalResultError(const std::string& what = "")
		: Error(what) {}
	};
	
	//! Specialized error
	/*! Thrown upon failed allocation.
	*/
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

