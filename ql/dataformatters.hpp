
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

#include <ql/Math/matrix.hpp>

namespace QuantLib {

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

}


#endif
