
/*
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

/*! \file types.hpp
    \brief Custom types
*/

#ifndef quantlib_types_hpp
#define quantlib_types_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! integer number
    typedef int Integer;

    //! real number
    typedef double Real;

    //! size of a container
    /*! \ingroup types */
    typedef QL_SIZE_T Size;

    //! continuous quantity with 1-year units
    /*! \ingroup types */
    typedef double Time;

    //! used to describe discount factors between dates.
    /*! \ingroup types */
    typedef double DiscountFactor;

    //! used to describe interest rates.
    /*! \ingroup types */
    typedef double Rate;

    //! used to describe spreads on interest rates.
    /*! \ingroup types */
    typedef double Spread;


}


#endif
