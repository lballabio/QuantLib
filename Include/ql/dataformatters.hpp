
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file dataformatters.hpp
    \fullpath Include/ql/%dataformatters.hpp
    \brief Classes used to format data for output

*/

// $Id$
// $Log$
// Revision 1.8  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.7  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.6  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.5  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.4  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_data_formatters_h
#define quantlib_data_formatters_h

#include "ql/date.hpp"
#include "ql/currency.hpp"

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
    /*! Formatting can be in short (mm/dd/yyyy)
        or long (Month ddth, yyyy) form.
    */
    class DateFormatter {
      public:
        static std::string toString(const Date& d, bool shortFormat = false);
    };

    //! Formats currencies for output
    class CurrencyFormatter {
      public:
        static std::string toString(Currency c);
    };

    //! Formats strings as lower- or uppercase
    class StringFormatter {
      public:
        static std::string toLowercase(const std::string& s);
        static std::string toUppercase(const std::string& s);
    };

}


#endif
