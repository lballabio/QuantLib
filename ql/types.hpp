/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

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
#include <cstddef>

namespace QuantLib {

    //! integer number
    /*! \ingroup types */
    typedef QL_INTEGER Integer;

    //! large integer number
    /*! \ingroup types */
    typedef QL_BIG_INTEGER BigInteger;

    //! positive integer
    /*! \ingroup types */
    typedef unsigned QL_INTEGER Natural;

    //! large positive integer
    typedef unsigned QL_BIG_INTEGER BigNatural;

    //! real number
    /*! \ingroup types */
    typedef QL_REAL Real;

    //! decimal number
    /*! \ingroup types */
    typedef Real Decimal;

    //! size of a container
    /*! \ingroup types */
    typedef std::size_t Size;

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

    //! probability
    /*! \ingroup types */
    typedef Real Probability;

    //! type conversions wrapper Real -> T, to allow specialisations for custom Real types
    /*! \ingroup types */
    template <class T>
    T ql_cast(const Real& x) {
        return static_cast<T>(x);
    };
}


#endif
