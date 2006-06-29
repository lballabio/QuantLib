/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 200

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_pseudoroot_hpp
#define quantlib_pseudoroot_hpp

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    /* For each time step, generates the pseudo-square root of the covariance
       matrix for that time step.
    */
    class PseudoRoot
    {
    public:

        virtual ~PseudoRoot();

        virtual const Matrix& pseudoRoot(Size i) const=0;
        virtual Size numberOfRates() const=0;
        virtual Size numberOfFactors() const=0;

    };

}

#endif
