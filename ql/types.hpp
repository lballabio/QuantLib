
/*
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

/*! \file types.hpp
    \brief Custom types
*/

#ifndef quantlib_types_hpp
#define quantlib_types_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! integer number
    /*! \ingroup types */
    typedef int Integer;

    //! real number
    /*! Modify this typedef for less or more precision.
        Changes will be propagated to all dependent types.

        \ingroup types 
    */
    typedef double Real;

    //! decimal number
    /*! \ingroup types */
    typedef Real Decimal;

    //! size of a container
    /*! \ingroup types */
    typedef QL_SIZE_T Size;

    //! continuous quantity with 1-year units
    /*! \ingroup types */
    typedef Real Time;

    //! discount factor between dates
    /*! \ingroup types */
    typedef Real DiscountFactor;

    //! interest rates
    /*! \ingroup types */
    typedef Real Rate;

    //! spreads on interest rates
    /*! \ingroup types */
    typedef Real Spread;

    //! volatility
    /*! \ingroup types */
    typedef Real Volatility;

}


#endif
