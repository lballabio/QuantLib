
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
/*! \file valueatcenter.hpp
    \brief compute value, first, and second derivatives at grid center

    \fullpath
    ql/FiniteDifferences/%valueatcenter.hpp
*/

// $Id$

#ifndef quantlib_finite_difference_value_at_center_h
#define quantlib_finite_difference_value_at_center_h

#include <ql/array.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! mid-point value
        double valueAtCenter(const Array& a);

        //! mid-point first derivative
        double firstDerivativeAtCenter(const Array& a,
            const Array& g);

        //! mid-point second derivative
        double secondDerivativeAtCenter(const Array& a,
            const Array& g);

    }

}


#endif
