
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

/*! \file types.hpp
    \brief Custom types

    \fullpath 
    Include/ql/%types.hpp
*/

// $Id$

#ifndef quantlib_types_hpp
#define quantlib_types_hpp

#include "ql/qldefines.hpp"

namespace QuantLib {

    //! integer number
    typedef int Integer;
    
    //! real number
    typedef double Real;
    
    //! size of a container
    typedef size_t Size;
    
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
