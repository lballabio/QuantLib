
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file dataformatters.hpp
    \brief Classes used to format data for output
*/

#ifndef quantlib_data_formatters_h
#define quantlib_data_formatters_h

#include <ql/currency.hpp>
#include <ql/option.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    //! Formats integers for output
    class IntegerFormatter {
      public:
        template<class int_type>
        static std::string toString(int_type l,
                                    unsigned int digits = 0) {
            if (l == Null<int_type>())
                return std::string("null");

            char s[64];
            QL_SPRINTF(s,"%*ld",(digits>64?64:digits),l);
            return std::string(s);
        }
        template<class int_type>
        static std::string toOrdinal(int_type l) {
            std::string suffix;
            if (l==int_type(11) || l==int_type(12) || l==int_type(13))
                suffix = "th";
            else {
                switch (l % int_type(10)) {
                case 1:  suffix = "st";  break;
                case 2:  suffix = "nd";  break;
                case 3:  suffix = "rd";  break;
                default: suffix = "th";
                }
            }
            return toString(l)+suffix;
        }
        template<class int_type>
        static std::string toPowerOfTwo(int_type l,
                                        unsigned int digits = 0) {
            if (long(l) == Null<int_type>())
                return std::string("null");

            unsigned long power = 0;
            std::string sign = "";
            if (l <= int_type(0)) {
                sign = "-"
                l = -l;
            }
            while (!(l & int_type(1))) {
                power++;
                l >>= 1;
            }
            return sign + toString(l,digits) + "*2^" + toString(power,2);
        }

    };

    //! Formats doubles for output
    class DoubleFormatter {
      public:
        static std::string toString(double x,
                                    int precision = 6,
                                    int digits = 0);
        static std::string toExponential(double x,
                                         int precision = 6,
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
                    s += ";\n  ";
                    n = 0;
                }
                if (n!=0)
                    s += " ; ";
                s += DoubleFormatter::toString(*i, precision, digits);
            }
            s += " ]";
            return s;
        }
    };


    /*! \relates Array */
    std::ostream& operator<< (std::ostream&,
                              const Array&);

    /*! \relates Matrix */
    std::ostream& operator<< (std::ostream&,
                              const Matrix&);

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
        static std::string toString(double rate,
                                    int precision = 5);
    };


    //! Formats dates for output
    /*! Formatting can be in short (mm/dd/yyyy)
        or long (Month ddth, yyyy) form.
    */
    class DateFormatter {
      public:
        enum Format { Long, Short, ISO };
        static std::string toString(const Date& d,
                                    Format f = Long);
    };

    /*! \relates Date */
    std::ostream& operator<< (std::ostream&,
                              const Date&);

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

    //! Formats option type for output
    class OptionTypeFormatter {
      public:
        static std::string toString(Option::Type type);
    };

}


#endif
