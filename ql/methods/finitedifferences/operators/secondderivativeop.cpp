/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

namespace QuantLib {

    SecondDerivativeOp::SecondDerivativeOp(
        Size direction,
        const ext::shared_ptr<FdmMesher>& mesher)
    : TripleBandLinearOp(direction, mesher) {

        for (const auto& iter : *mesher->layout()) {
            const Size i = iter.index();
            const Real hm = mesher->dminus(iter, direction_);
            const Real hp = mesher->dplus(iter, direction_);

            const Real zetam1 = hm*(hm+hp);
            const Real zeta0  = hm*hp;
            const Real zetap1 = hp*(hm+hp);

            const Size co = iter.coordinates()[direction_];
            if (co == 0 || co == mesher->layout()->dim()[direction]-1) {
                lower_[i] = diag_[i] = upper_[i] = 0.0;
            }
            else {
                lower_[i] =  2.0/zetam1;
                diag_[i]  = -2.0/zeta0;
                upper_[i] =  2.0/zetap1;
            }
        }
    }
}
