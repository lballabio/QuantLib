/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

#include <ql/math/optimization/linesearch.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace QuantLib {

    Real LineSearch::update(Array& params,
                            const Array& direction,
                            Real beta,
                            const Constraint& constraint) {

        Real diff=beta;
        Array newParams = params + diff*direction;
        bool valid = constraint.test(newParams);
        Integer icount = 0;
        while (!valid) {
            if (icount > 200)
                QL_FAIL("can't update linesearch");
            diff *= 0.5;
            icount ++;
            newParams = params + diff*direction;
            valid = constraint.test(newParams);
        }
        params += diff*direction;
        return diff;
    }

}
