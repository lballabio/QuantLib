
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_formats_h
#define quantlib_formats_h

#include "qldefines.h"
#include "date.h"
#include "null.h"
#include <string>
#include <sstream>

QL_BEGIN_NAMESPACE(QuantLib)

std::string IntegerFormat(int i, int digits = Null<int>());
std::string DoubleFormat(double x, int precision = Null<int>());
std::string DoubleFormat(double x, int precision, int digits);
std::string EuroFormat(double);
std::string RateFormat(double rate, int precision = 5);
std::string DateFormat(const Date&);
std::string ShortDateFormat(const Date&);

// inline definitions

inline std::string IntegerFormat(int i, int digits) {
	std::ostringstream out;
	if (!IsNull(digits)) {
		out.width(digits);
		out.fill('0');
	}
	out << i;
	return out.str();
}

inline std::string DoubleFormat(double x, int precision) {
	std::ostringstream out;
	if (!IsNull(precision)) {
		out.setf(std::ios_base::fixed);
		out.precision(precision);
	}
	out << x;
	return out.str();
}

inline std::string DoubleFormat(double x, int precision, int digits) {
	std::ostringstream out;
	out.setf(std::ios_base::fixed);
	out.precision(precision);
	out.width(digits);
	out.fill(' ');
	out << x;
	return out.str();
}

inline std::string EuroFormat(double amount) {
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
	output += IntegerFormat(int(amount));
	amount -= int(amount);
	while (triples > 0) {
		amount *= 1000;
		output += ","+IntegerFormat(int(amount),3);
		amount -= int(amount);
		triples--;
	}
	amount *= 100;
	output += "."+IntegerFormat(int(amount+0.5),2);
	return output;
}

inline std::string RateFormat(double rate, int precision) {
	return DoubleFormat(rate*100,precision)+"%";
}

inline std::string DateFormat(const Date& d) {
	static const std::string monthName[] = {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };
	std::string output;
	output = monthName[d.month()] + " " + IntegerFormat(d.dayOfMonth()) + ", " + IntegerFormat(d.year());
	return output;
}

inline std::string ShortDateFormat(const Date& d) {
	std::string output;
	int dd = d.dayOfMonth(), mm = int(d.month()), yyyy = d.year();
	output = (mm < 10 ? "0" : "") + IntegerFormat(mm) + "/";
	output += (dd < 10 ? "0" : "") + IntegerFormat(dd) + "/";
	output += IntegerFormat(yyyy);
	return output;
}

QL_END_NAMESPACE(QuantLib)


#endif
