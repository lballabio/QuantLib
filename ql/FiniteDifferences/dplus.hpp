
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
/*! \file dplus.hpp
    \brief \f$ D_{+} \f$ matricial representation

    \fullpath
    ql/FiniteDifferences/%dplus.hpp
*/

// $Id$

#ifndef quantlib_d_plus_h
#define quantlib_d_plus_h

#include <ql/FiniteDifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! \f$ D_{+} \f$ matricial representation
        /*! The differential operator \f$ D_{+} \f$ discretizes the
            first derivative with the first-order formula
            \f[ \frac{\partial u_{i}}{\partial x} \approx
                \frac{u_{i+1}-u_{i}}{h} = D_{+} u_{i}
            \f]
        */
        class DPlus : public TridiagonalOperator {
          public:
            DPlus(Size gridPoints, double h);
        };

        // inline definitions

        inline DPlus::DPlus(Size gridPoints, double h)
        : TridiagonalOperator(gridPoints) {
            setFirstRow(-1/h,1/h);
            setMidRows(0.0,-1/h,1/h);
            setLastRow(-1/h,1/h);                   // linear extrapolation
        }

    }

}


#endif
