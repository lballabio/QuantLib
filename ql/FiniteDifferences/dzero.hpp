
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

/*! \file dzero.hpp
    \brief \f$ D_{0} \f$ matricial representation
*/

#ifndef quantlib_d_zero_h
#define quantlib_d_zero_h

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    //! \f$ D_{0} \f$ matricial representation
    /*! The differential operator \f$ D_{0} \f$ discretizes the
        first derivative with the second-order formula
        \f[ \frac{\partial u_{i}}{\partial x} \approx
            \frac{u_{i+1}-u_{i-1}}{2h} = D_{0} u_{i}
        \f]

        \ingroup findiff
    */
    class DZero : public TridiagonalOperator {
      public:
        DZero(Size gridPoints, double h);
    };


    // inline definitions

    inline DZero::DZero(Size gridPoints, double h)
    : TridiagonalOperator(gridPoints) {
        setFirstRow(-1/h,1/h);                  // linear extrapolation
        setMidRows(-1/(2*h),0.0,1/(2*h));
        setLastRow(-1/h,1/h);                   // linear extrapolation
    }

}


#endif
