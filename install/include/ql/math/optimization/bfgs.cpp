/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

 /*
 Copyright (C) 2009 Frédéric Degraeve

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

#include <ql/math/optimization/bfgs.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/linesearch.hpp>

namespace QuantLib {

    Array BFGS::getUpdatedDirection(const Problem& P,
                                    Real,
                                    const Array& oldGradient) {
        if (inverseHessian_.rows() == 0)
        {
            // first time in this update, we create needed structures
            inverseHessian_ = Matrix(P.currentValue().size(),
                                     P.currentValue().size(), 0.);
            for (Size i = 0; i < P.currentValue().size(); ++i)
                inverseHessian_[i][i] = 1.;
        }

        Array diffGradient;
        Array diffGradientWithHessianApplied(P.currentValue().size(), 0.);

        diffGradient = lineSearch_->lastGradient() - oldGradient;
        for (Size i = 0; i < P.currentValue().size(); ++i)
            for (Size j = 0; j < P.currentValue().size(); ++j)
                diffGradientWithHessianApplied[i] += inverseHessian_[i][j] * diffGradient[j];

        Real fac, fae, fad;
        Real sumdg, sumxi;

        fac = fae = sumdg = sumxi = 0.;
        for (Size i = 0; i < P.currentValue().size(); ++i)
        {
            fac += diffGradient[i] * lineSearch_->searchDirection()[i];
            fae += diffGradient[i] * diffGradientWithHessianApplied[i];
            sumdg += std::pow(diffGradient[i], 2.);
            sumxi += std::pow(lineSearch_->searchDirection()[i], 2.);
        }

        if (fac > std::sqrt(1e-8 * sumdg * sumxi))  // skip update if fac not sufficiently positive
        {
            fac = 1.0 / fac;
            fad = 1.0 / fae;

            for (Size i = 0; i < P.currentValue().size(); ++i)
                diffGradient[i] = fac * lineSearch_->searchDirection()[i] - fad * diffGradientWithHessianApplied[i];

            for (Size i = 0; i < P.currentValue().size(); ++i)
                for (Size j = 0; j < P.currentValue().size(); ++j)
                {
                    inverseHessian_[i][j] += fac * lineSearch_->searchDirection()[i] * lineSearch_->searchDirection()[j];
                    inverseHessian_[i][j] -= fad * diffGradientWithHessianApplied[i] * diffGradientWithHessianApplied[j];
                    inverseHessian_[i][j] += fae * diffGradient[i] * diffGradient[j];
                }
        }
        //else
        //  throw "BFGS: FAC not sufficiently positive";


        Array direction(P.currentValue().size());
        for (Size i = 0; i < P.currentValue().size(); ++i)
        {
            direction[i] = 0.0;
            for (Size j = 0; j < P.currentValue().size(); ++j)
                direction[i] -= inverseHessian_[i][j] * lineSearch_->lastGradient()[j];
        }

        return direction;
    }

}
