
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/04/09 14:03:54  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file dataformatters.hpp
    \brief Classes used to format data for output
*/

#ifndef quantlib_data_formatters_h
#define quantlib_data_formatters_h

#include "ql/qldefines.hpp"
#include "ql/date.hpp"
#include "ql/null.hpp"
#include <string>

namespace QuantLib {

    //! Formats integers for output
    class IntegerFormatter {
      public:
        static std::string toString(int i, int digits = 0);
    };

    //! Formats doubles for output
    class DoubleFormatter {
      public:
        static std::string toString(double x, int precision = 6,
            int digits = 0);
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
