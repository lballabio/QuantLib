
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

/*! \file dataformatters.cpp
	\brief classes used to format data for output
	
	$Source$
	$Name$
	$Log$
	Revision 1.6  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#include "dataformatters.h"
#include <cctype>
#include <sstream>

namespace QuantLib {
	
	std::string IntegerFormatter::toString(int i, int digits) {
		std::ostringstream out;
		if (!IsNull(digits)) {
			out.width(digits);
			out.fill('0');
		}
		out << i;
		return out.str();
	}

	std::string DoubleFormatter::toString(double x, int precision, int digits) {
		std::ostringstream out;
		if (!IsNull(precision)) {
			out.setf(std::ios_base::fixed);
			out.precision(precision);
		}
		if (!IsNull(digits)) {
			out.width(digits);
			out.fill(' ');
		}
		out << x;
		return out.str();
	}

	std::string EuroFormatter::toString(double amount) {
		std::string output;
		if (amount < 0.0) {
			output = "-";
			amount = -amount;
		} else {
			output = " ";
		}
		int triples = 0;
		while (amount >= 1000.0) {
			amount /= 1000;
			triples++;
		}
		output += IntegerFormatter::toString(int(amount));
		amount -= int(amount);
		while (triples > 0) {
			amount *= 1000;
			output += ","+IntegerFormatter::toString(int(amount),3);
			amount -= int(amount);
			triples--;
		}
		amount *= 100;
		output += "."+IntegerFormatter::toString(int(amount+0.5),2);
		return output;
	}

	std::string RateFormatter::toString(double rate, int precision) {
		return DoubleFormatter::toString(rate*100,precision)+"%";
	}

	std::string DateFormatter::toString(const Date& d, bool shortFormat) {
		static const std::string monthName[] = {
			"January", "February", "March", "April", "May", "June",
			"July", "August", "September", "October", "November", "December" };
		std::string output;
		int dd = d.dayOfMonth(), mm = int(d.month()), yyyy = d.year();
		if (shortFormat) {
			output = (mm < 10 ? "0" : "") + IntegerFormatter::toString(mm);
			output += (dd < 10 ? "/0" : "/") + IntegerFormatter::toString(dd);
			output += "/" + IntegerFormatter::toString(yyyy);
		} else {
			output = monthName[mm] + " ";
			output += IntegerFormatter::toString(dd);
			switch (dd) {
			  case 1:
			  case 21:
			  case 31:
				output += "st, ";
				break;
			  case 2:
			  case 22:
				output += "nd, ";
				break;
			  case 3:
			  case 23:
				output += "rd, ";
				break;
			  default:
				output += "th, ";
			}
			output += IntegerFormatter::toString(yyyy);
		}
		return output;
	}

	std::string StringFormatter::toLowercase(const std::string& s) {
		std::string output = s;
		for (std::string::iterator i=output.begin(); i!=output.end(); i++)
			*i = QL_TOLOWER(*i);
		return output;
	}

	std::string StringFormatter::toUppercase(const std::string& s) {
		std::string output = s;
		for (std::string::iterator i=output.begin(); i!=output.end(); i++)
			*i = QL_TOUPPER(*i);
		return output;
	}

}
