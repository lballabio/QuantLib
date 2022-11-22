/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

#include <ql/math/integrals/discreteintegrals.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/sum.hpp>

using namespace boost::accumulators;

namespace QuantLib {

    Real DiscreteTrapezoidIntegral::operator()(
        const Array& x, const Array& f)    const {

        const Size n = f.size();
        QL_REQUIRE(n == x.size(), "inconsistent size");

        accumulator_set<Real, features<tag::sum> > acc;

        for (Size i=0; i < n-1; ++i) {
            acc((x[i+1]-x[i])*(f[i]+f[i+1]));
        }

        return 0.5*sum(acc);
    }

    Real DiscreteSimpsonIntegral::operator()(
        const Array& x, const Array& f)    const {

        const Size n = f.size();
        QL_REQUIRE(n == x.size(), "inconsistent size");

        accumulator_set<Real, features<tag::sum> > acc;

        for (Size j=0; j < n-2; j+=2) {
            const Real dxj   = x[j+1]-x[j];
            const Real dxjp1 = x[j+2]-x[j+1];

            const Real alpha = -dxjp1*(2*x[j]-3*x[j+1]+x[j+2]);
            const Real dd = x[j+2]-x[j];
            const Real k = dd/(6*dxjp1*dxj);
            const Real beta = dd*dd;
            const Real gamma = dxj*(x[j]-3*x[j+1]+2*x[j+2]);

            acc(k*alpha*f[j]+k*beta*f[j+1]+k*gamma*f[j+2]);
        }
        if ((n & 1) == 0U) {
            acc(0.5*(x[n-1]-x[n-2])*(f[n-1]+f[n-2]));
        }

        return sum(acc);
    }


    Real DiscreteTrapezoidIntegrator::integrate(
        const ext::function<Real (Real)>& f, Real a, Real b) const {
            const Array x(maxEvaluations(), a, (b-a)/(maxEvaluations()-1));
            Array fv(x.size());
            std::transform(x.begin(), x.end(), fv.begin(), f);

            increaseNumberOfEvaluations(maxEvaluations());
            return DiscreteTrapezoidIntegral()(x, fv);
    }

    Real DiscreteSimpsonIntegrator::integrate(
        const ext::function<Real (Real)>& f, Real a, Real b) const {
            const Array x(maxEvaluations(), a, (b-a)/(maxEvaluations()-1));
            Array fv(x.size());
            std::transform(x.begin(), x.end(), fv.begin(), f);

            increaseNumberOfEvaluations(maxEvaluations());
            return DiscreteSimpsonIntegral()(x, fv);
    }
}
