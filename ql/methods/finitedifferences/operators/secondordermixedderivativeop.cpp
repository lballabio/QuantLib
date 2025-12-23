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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondordermixedderivativeop.hpp>

namespace QuantLib {
    SecondOrderMixedDerivativeOp::SecondOrderMixedDerivativeOp(
        Size d0, Size d1,
        const ext::shared_ptr<FdmMesher>& mesher)
    : NinePointLinearOp(d0, d1, mesher) {

        for (const auto& iter : *mesher->layout()) {
            const Size i = iter.index();
            const Real hm_d0 = mesher->dminus(iter, d0_);
            const Real hp_d0 = mesher->dplus(iter, d0_);
            const Real hm_d1 = mesher->dminus(iter, d1_);
            const Real hp_d1 = mesher->dplus(iter, d1_);

            const Real zetam1 = hm_d0*(hm_d0+hp_d0);
            const Real zeta0  = hm_d0*hp_d0;
            const Real zetap1 = hp_d0*(hm_d0+hp_d0);
            const Real phim1  = hm_d1*(hm_d1+hp_d1);
            const Real phi0   = hm_d1*hp_d1;
            const Real phip1  = hp_d1*(hm_d1+hp_d1);

            const Size c0 = iter.coordinates()[d0_];
            const Size c1 = iter.coordinates()[d1_];
            if (c0 == 0 && c1 == 0) {
                // lower left corner
                a00_[i] = a01_[i] = a02_[i] = a10_[i] = a20_[i] = 0.0;
                a21_[i] = a12_[i] = -(a11_[i] = a22_[i] = 1.0/(hp_d0*hp_d1));
            }
            else if (c0 == mesher->layout()->dim()[d0_]-1 && c1 == 0) {
                // upper left corner
                a22_[i] = a21_[i] = a20_[i] = a10_[i] = a00_[i] = 0.0;
                a11_[i] = a02_[i] = -(a01_[i] = a12_[i] = 1.0/(hm_d0*hp_d1));
            }
            else if (c0 == 0 && c1 == mesher->layout()->dim()[d1_]-1) {
                // lower right corner
                a00_[i] = a01_[i] = a02_[i] = a12_[i] = a22_[i] = 0.0;
                a20_[i] = a11_[i] = -(a10_[i] = a21_[i] = 1.0/(hp_d0*hm_d1));
            }
            else if (c0 == mesher->layout()->dim()[d0_]-1 && c1 == mesher->layout()->dim()[d1_]-1) {
                // upper right corner
                a20_[i] = a21_[i] = a22_[i] = a12_[i] = a02_[i] = 0.0;
                a10_[i] = a01_[i] = -(a00_[i] = a11_[i] = 1.0/(hm_d0*hm_d1));
            }
            else if (c0 == 0) {
                // lower side
                a00_[i] = a01_[i] = a02_[i] = 0.0;

                a20_[i] = -(a10_[i] = hp_d1/(hp_d0*phim1));
                a11_[i] = -(a21_[i] = (hp_d1-hm_d1)/(hp_d0*phi0));
                a12_[i] = -(a22_[i] = hm_d1/(hp_d0*phip1));
            }
            else if (c0 == mesher->layout()->dim()[d0_]-1) {
                // upper side
                a20_[i] = a21_[i] = a22_[i] = 0.0;

                a10_[i] = -(a00_[i] = hp_d1/(hm_d0*phim1));
                a01_[i] = -(a11_[i] = (hp_d1-hm_d1)/(hm_d0*phi0));
                a02_[i] = -(a12_[i] = hm_d1/(hm_d0*phip1));
            }
            else if (c1 == 0) {
                // left side
                a00_[i] = a10_[i] = a20_[i] = 0.0;

                a02_[i] = -(a01_[i] = hp_d0/(zetam1*hp_d1));
                a11_[i] = -(a12_[i] = (hp_d0-hm_d0)/(zeta0*hp_d1));
                a21_[i] = -(a22_[i] = hm_d0/(zetap1*hp_d1));
            }
            else if (c1 == mesher->layout()->dim()[d1_]-1) {
                // right side
                a22_[i] = a12_[i] = a02_[i] = 0.0;

                a01_[i] = -(a00_[i] = hp_d0/(zetam1*hm_d1));
                a10_[i] = -(a11_[i] = (hp_d0-hm_d0)/(zeta0*hm_d1));
                a20_[i] = -(a21_[i] = hm_d0/(zetap1*hm_d1));
            }
            else {
                a00_[i] =  hp_d0*hp_d1/(zetam1*phim1);
                a10_[i] = -(hp_d0-hm_d0)*hp_d1/(zeta0*phim1);
                a20_[i] = -hm_d0*hp_d1/(zetap1*phim1);
                a01_[i] = -hp_d0*(hp_d1-hm_d1)/(zetam1*phi0);
                a11_[i] = (hp_d0-hm_d0)*(hp_d1-hm_d1)/(zeta0*phi0);
                a21_[i] =  hm_d0*(hp_d1-hm_d1)/(zetap1*phi0);
                a02_[i] = -hp_d0*hm_d1/(zetam1*phip1);
                a12_[i] =  hm_d1*(hp_d0-hm_d0)/(zeta0*phip1);
                a22_[i] =  hm_d0*hm_d1/(zetap1*phip1);
            }
        }
    }
}
