
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000-2004 StatPro Italia srl

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

#ifndef quantlib_data_formatters_hpp
#define quantlib_data_formatters_hpp

#include <ql/basicdataformatters.hpp>
#include <ql/currency.hpp>
#include <ql/option.hpp>
#include <ql/Math/matrix.hpp>
#include <iostream>

namespace QuantLib {

    //! Formats amounts in Euro for output
    /*! Formatting follows Euro convention (x,xxx,xxx.xx) */
    class EuroFormatter {
      public:
        static std::string toString(Decimal amount);
    };

    //! Formats rates for output
    /*! Formatting is in percentage form (xx.xxxxx%) */
    class RateFormatter {
      public:
        static std::string toString(Rate rate,
                                    Integer precision = 5);
    };

    //! Formats volatilities for output
    /*! Formatting is in percentage form (xx.xxxxx%) */
    class VolatilityFormatter {
      public:
        static std::string toString(Volatility vol,
                                    Integer precision = 5);
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
    std::ostream& operator<<(std::ostream&, const Date&);


    //! Formats currencies for output
    class CurrencyFormatter {
      public:
        static std::string toString(CurrencyTag c);
    };

    //! Formats option type for output
    class OptionTypeFormatter {
      public:
        static std::string toString(Option::Type type);
    };


    //! Formats arrays for output
    class ArrayFormatter {
      public:
        template<class DataIterator>
        static std::string toString(DataIterator begin,
                                    DataIterator end,
                                    Integer precision = 6,
                                    Integer digits = 0,
                                    Size elementsPerRow = QL_MAX_INTEGER) {
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
                s += DecimalFormatter::toString(*i, precision, digits);
            }
            s += " ]";
            return s;
        }
    };


    #ifndef QL_PATCH_MICROSOFT
    /*! \relates Array */
    std::ostream& operator<< (std::ostream&,
                              const Array&);

    /*! \relates Matrix */
    std::ostream& operator<< (std::ostream&,
                              const Matrix&);
    #endif

}


#endif
