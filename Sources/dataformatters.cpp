
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file dataformatters.cpp
    \brief classes used to format data for output

    $Source$
    $Log$
    Revision 1.18  2001/05/09 11:06:19  nando
    A few comments modified/removed

    Revision 1.17  2001/04/10 07:54:33  lballabio
    Ruby histories (the Ruby way)

    Revision 1.16  2001/04/09 14:13:33  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.15  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.14  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.13  2001/03/12 17:35:11  lballabio
    Removed global IsNull function - could have caused very vicious loops

    Revision 1.12  2001/02/23 17:24:45  lballabio
    Allow formatting of null dates

    Revision 1.11  2001/01/17 14:37:56  nando
    tabs removed

*/

#include "ql/dataformatters.hpp"
#include "ql/null.hpp"
#include <stdio.h>

namespace QuantLib {

    std::string IntegerFormatter::toString(int i, int digits) {
        if (i == Null<int>())
	    return std::string("null");
        char s[64];
        sprintf(s,"%*d",(digits>64?64:digits),i);
        return std::string(s);
    }

    std::string DoubleFormatter::toString(double x, int precision, int digits) {
        if (x == Null<double>())
	    return std::string("null");
        char s[64];
        sprintf(s,"%*.*lf",(digits>64?64:digits),(precision>64?64:precision),x);
        return std::string(s);
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
        if (d == Date()) {
            output = "Null date";
        } else {
            int dd = d.dayOfMonth(), mm = int(d.month()), yyyy = d.year();
            if (shortFormat) {
                output = (mm < 10 ? "0" : "") + IntegerFormatter::toString(mm);
                output += (dd < 10 ? "/0" : "/") +
                    IntegerFormatter::toString(dd);
                output += "/" + IntegerFormatter::toString(yyyy);
            } else {
                output = monthName[mm-1] + " ";
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
