
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file dataformatters.hpp
    \brief Classes used to format data for output
*/

// $Id$

#ifndef quantlib_data_formatters_h
#define quantlib_data_formatters_h

#include <ql/date.hpp>
#include <ql/currency.hpp>
#include <ql/array.hpp>

namespace QuantLib {

    //! Formats integers for output
    class IntegerFormatter {
      public:
        static std::string toString(long l, int digits = 0);
        static std::string toOrdinal(long l);
    };

    //! Formats doubles for output
    class DoubleFormatter {
      public:
        static std::string toString(double x, int precision = 6,
                                    int digits = 0);
    };

    //! Formats arrays for output
    class ArrayFormatter {
      public:
        template<class DataIterator>
        static std::string toString(DataIterator begin,
                                    DataIterator end,
                                    int precision = 6,
                                    int digits = 0, 
                                    Size elementsPerRow = QL_MAX_INT) {
            std::string s = "[ ";
            DataIterator i;
            Size n;
            for (i=begin, n=0; i!=end; i++, n++) {
                if (n == elementsPerRow) {
                    s += "\n  ";
                    n = 0;
                }
                if (i!=begin)
                    s += " ; ";
                s += DoubleFormatter::toString(*i, precision, digits);
            }
            s += " ]";
            return s;
        }
    };

    
    /*! \relates Array */
    std::ostream& operator<< (std::ostream&, const Array&);
    
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
        enum Format { Long, Short, ISO };
        static std::string toString(const Date& d, Format f = Long);
    };

    /*! \relates Date */
    std::ostream& operator<< (std::ostream&, const Date&);

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
