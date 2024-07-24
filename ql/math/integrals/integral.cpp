/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud

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

#include <ql/math/integrals/integral.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Integrator::Integrator(Real absoluteAccuracy,
                          Size maxEvaluations)
    : absoluteAccuracy_(absoluteAccuracy),
      maxEvaluations_(maxEvaluations) {
        QL_REQUIRE(absoluteAccuracy > QL_EPSILON,
                   std::scientific << "required tolerance (" <<
                   absoluteAccuracy << ") not allowed. It must be > " <<
                   QL_EPSILON);
    }

    void Integrator::setAbsoluteAccuracy(Real accuracy) {
        absoluteAccuracy_= accuracy;
    }

    void Integrator::setMaxEvaluations(Size maxEvaluations) {
        maxEvaluations_ = maxEvaluations;
    }

    Real Integrator::absoluteAccuracy() const {
        return absoluteAccuracy_;
    }

    Size Integrator::maxEvaluations() const {
        return maxEvaluations_;
    }

    Real Integrator::absoluteError() const {
        return absoluteError_;
    }

    void Integrator::setAbsoluteError(Real error) const {
        absoluteError_ = error;
    }

    Size Integrator::numberOfEvaluations() const {
        return evaluations_;
    }

    void Integrator::setNumberOfEvaluations(Size evaluations) const {
        evaluations_ = evaluations;
    }

    void Integrator::increaseNumberOfEvaluations(Size increase) const {
        evaluations_ += increase;
    }

    bool Integrator::integrationSuccess() const {
        return evaluations_ <= maxEvaluations_
            && absoluteError_ <= absoluteAccuracy_;
    }

    Real Integrator::operator()(const std::function<Real (Real)>& f,
                                Real a,
                                Real b) const {
        evaluations_ = 0;
        if (a == b)
            return 0.0;
        if (b > a)
            return integrate(f, a, b);
        else
            return -integrate(f, b, a);
    }

}
