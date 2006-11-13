/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mark Joshi

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


#include <ql/MarketModels/swapforwardmappings.hpp>
#include <ql/MarketModels/curvestate.hpp>
#include <vector>

namespace QuantLib {

    Disposable<Matrix>
    SwapForwardMappings::coterminalSwapForwardJacobian(const CurveState& cs)
    {
        Size n = cs.numberOfRates();
        const std::vector<Real>& b = cs.coterminalSwapAnnuities();
        const std::vector<Real>& p = cs.discountRatios();
        const std::vector<Rate>& f = cs.forwardRates();
        const std::vector<Time>& tau = cs.rateTaus();

        // coterminal floating leg values
        std::vector<Real> a(n);
        for (Size k=0; k<n; ++k)
            a[k] = p[k]-p[n];
        Matrix jacobian = Matrix(n, n, 0.0);
        for (Size i=0; i<n; ++i) {     // i = swap rate index
            for (Size j=i; j<n; ++j) { // j = forward rate index
                jacobian[i][j] = 
                    p[j+1]*tau[j]/b[i] + tau[j]/(1.0+f[j]*tau[j]) *
                    (-a[j]*b[i]+a[i]*b[j])/(b[i]*b[i]);
            }
        }
        return jacobian;
    }

    Disposable<Matrix>
    SwapForwardMappings::coterminalSwapZedMatrix(const CurveState& cs,
                                                 const Spread displacement)
    {
        Size n = cs.numberOfRates();
        Matrix result = coterminalSwapForwardJacobian(cs);
        const std::vector<Rate>& f = cs.forwardRates();
        const std::vector<Rate>& sr = cs.coterminalSwapRates();
        for (Size i=0; i<n; ++i) {
            for (Size j=i; j<n; ++j) {
                result[i][j] *= (f[j]+displacement)/(sr[i]+displacement);
            }
        }
        return result;  // zMatrix = f[j]/sr[j] * dsr[i]/df[j]
    }

}
