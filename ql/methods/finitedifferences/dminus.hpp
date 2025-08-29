/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file dminus.hpp
    \brief \f$ D_{-} \f$ matricial representation
*/

#ifndef quantlib_d_minus_h
#define quantlib_d_minus_h

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    //! \f$ D_{-} \f$ matricial representation
    /*! The differential operator \f$ D_{-} \f$ discretizes the
        first derivative with the first-order formula
        \f[ \frac{\partial u_{i}}{\partial x} \approx
            \frac{u_{i}-u_{i-1}}{h} = D_{-} u_{i}
        \f]

        \ingroup findiff
    */
    class DMinus : public TridiagonalOperator {
      public:
        DMinus(Size gridPoints, Real h);
    };


    // inline definitions

    inline DMinus::DMinus(Size gridPoints, Real h)
    : TridiagonalOperator(gridPoints) {
        setFirstRow(-1/h,1/h);                  // linear extrapolation
        setMidRows(-1/h,1/h,0.0);
        setLastRow(-1/h,1/h);
    }

}


#endif
