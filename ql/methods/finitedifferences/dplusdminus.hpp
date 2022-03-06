/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file dplusdminus.hpp
    \brief \f$ D_{+}D_{-} \f$ matricial representation
*/

#ifndef quantlib_d_plus_d_minus_h
#define quantlib_d_plus_d_minus_h

#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    //! \f$ D_{+}D_{-} \f$ matricial representation
    /*! The differential operator \f$  D_{+}D_{-} \f$ discretizes the
        second derivative with the second-order formula
        \f[ \frac{\partial^2 u_{i}}{\partial x^2} \approx
            \frac{u_{i+1}-2u_{i}+u_{i-1}}{h^2} = D_{+}D_{-} u_{i}
        \f]

        \ingroup findiff

        \test the correctness of the returned values is tested by
              checking them against numerical calculations.
    */
    class DPlusDMinus : public TridiagonalOperator {
      public:
        DPlusDMinus(Size gridPoints, Real h);
    };


    // inline definitions

    inline DPlusDMinus::DPlusDMinus(Size gridPoints, Real h)
    : TridiagonalOperator(gridPoints) {
        setFirstRow(0.0,0.0);                   // linear extrapolation
        setMidRows(1/(h*h),-2/(h*h),1/(h*h));
        setLastRow(0.0,0.0);                    // linear extrapolation
    }

}


#endif


#ifndef id_34cf7dfd34ce7709b3981ad640796254
#define id_34cf7dfd34ce7709b3981ad640796254
inline bool test_34cf7dfd34ce7709b3981ad640796254(const int* i) {
    return i != nullptr;
}
#endif
