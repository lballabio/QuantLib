
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file types.hpp
    \brief Custom types

    \fullpath
    ql/%types.hpp
*/

// $Id$

#ifndef quantlib_types_hpp
#define quantlib_types_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! integer number
    typedef int Integer;

    //! real number
    typedef double Real;

    //! size of a container
    typedef QL_SIZE_T Size;

    //! continuous quantity with 1-year units
    typedef double Time;

    //! used to describe discount factors between dates.
    typedef double DiscountFactor;

    //! used to describe interest rates.
    typedef double Rate;

    //! used to describe spreads on interest rates.
    typedef double Spread;


}


#endif
