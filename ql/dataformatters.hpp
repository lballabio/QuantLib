
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
#include <ql/date.hpp>
#include <ql/currency.hpp>
#include <ql/Math/matrix.hpp>
#include <iostream>

namespace QuantLib {

    //! Formats amounts in Euro for output
    /*! Formatting follows Euro convention (x,xxx,xxx.xx) */
    class EuroFormatter {
      public:
        static std::string toString(Decimal amount);
    };


    /*! \relates Date
        \deprecated send to the stream the output of DateFormatter
    */
    std::ostream& operator<<(std::ostream&, const Date&);


    #ifndef QL_PATCH_MICROSOFT
    /*! \relates Array
        \deprecated send to the stream the output of ArrayFormatter
    */
    std::ostream& operator<<(std::ostream&, const Array&);

    /*! \relates Matrix
        \deprecated send to the stream the output of MatrixFormatter
    */
    std::ostream& operator<<(std::ostream&, const Matrix&);
    #endif

}


#endif
