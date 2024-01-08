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

namespace QuantLib {

    Real DiscreteTrapezoidIntegral::operator()(
        const Array& x, const Array& f)    const {

        const Size n = f.size();
        QL_REQUIRE(n == x.size(), "inconsistent size");

        Real sum = 0.0;

        for (Size i=0; i < n-1; ++i) {
            sum += (x[i+1]-x[i])*(f[i]+f[i+1]);
        }

        return 0.5*sum;
    }

    Real DiscreteSimpsonIntegral::operator()(
        const Array& x, const Array& f)    const {

        const Size n = f.size();
        QL_REQUIRE(n == x.size(), "inconsistent size");

        Real sum = 0.0;

        for (Size j=0; j < n-2; j+=2) {
            const Real dxj   = x[j+1]-x[j];
            const Real dxjp1 = x[j+2]-x[j+1];

            const Real alpha = dxjp1*(2*dxj-dxjp1);
            const Real dd = dxj+dxjp1;
            const Real k = dd/(6*dxjp1*dxj);
            const Real beta = dd*dd;
            const Real gamma = dxj*(2*dxjp1-dxj);

            sum += k*(alpha*f[j]+beta*f[j+1]+gamma*f[j+2]);
        }
        if ((n & 1) == 0U) {
            sum += 0.5*(x[n-1]-x[n-2])*(f[n-1]+f[n-2]);
        }

        return sum;
    }

    Real DiscreteTrapezoidIntegrator::integrate(
        const ext::function<Real (Real)>& f, Real a, Real b) const {
            const Size n=maxEvaluations()-1;
            const Real d=(b-a)/n;
            
            Real sum = f(a)/2;
            
            for(Size i=0;i<n-1;++i) {
                a += d;
                sum += f(a);
            }
            sum += f(b)/2;
            
            increaseNumberOfEvaluations(maxEvaluations());
            
            return d * sum;
    }

    Real DiscreteSimpsonIntegrator::integrate(
        const ext::function<Real (Real)>& f, Real a, Real b) const {
            const Size n=maxEvaluations()-1;
            const Real d=(b-a)/n, d2=d*2;
            
            Real sum = 0.0, x = a + d;
            for(Size i=1;i<n;i+=2) {//to time 4
                sum += f(x);
                x += d2;
            }
            sum *= 2;

            x = a+d2;
            for(Size i=2;i<n-1;i+=2) {//to time 2
                sum += f(x);
                x += d2;
            }
            sum *= 2;

            sum += f(a);
            if(n&1)
                sum += 1.5*f(b)+2.5*f(b-d);
            else
                sum += f(b);
            
            increaseNumberOfEvaluations(maxEvaluations());
            
            return d/3 * sum;
    }
}
