
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

// $Source$

// $Log$
// Revision 1.4  2000/12/13 18:09:52  nando
// CVS keyword added
//

/*! \file dataformatters.h
	\brief Classes used to format data for output
*/

#ifndef quantlib_data_formatters_h
#define quantlib_data_formatters_h

#include "qldefines.h"
#include "date.h"
#include "null.h"
#include <string>

namespace QuantLib {

	//! Formats integers for output
	class IntegerFormatter {
	  public:
		static std::string toString(int i, int digits = Null<int>());
	};

	//! Formats doubles for output
	class DoubleFormatter {
	  public:
		static std::string toString(double x, int precision = Null<int>(), int digits = Null<int>());
	};

	//! Formats amounts in Euro for output
	/*! Formatting follows Euro convention (x,xxx,xxx.xx) */
	class EuroFormatter {
	  public:
		static std::string toString(double amount);
	};
	
	//! Formats rates for output
	/*! Formatting is in percentage form (xx.xxxxx%) */
	class RateFormatter {
	  public:
		static std::string toString(double rate, int precision = 5);
	};
	
	//! Formats dates for output
	/*! Formatting can be in short (mm/dd/yyyy) or long (Month ddth, yyyy) form */
	class DateFormatter {
	  public:
		static std::string toString(const Date& d, bool shortFormat = false);
	};

	//! Formats strings as lower- or uppercase
	class StringFormatter {
	  public:
		static std::string toLowercase(const std::string& s);
		static std::string toUppercase(const std::string& s);
	};
	
}


#endif
