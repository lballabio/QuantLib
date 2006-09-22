/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) Mark Joshi 2006

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


#include <ql/MarketModels/swapforwardconversionmatrix.hpp>
#include <vector>

namespace QuantLib {

    Disposable<Matrix> swapForwardJacobian(const CurveState& cs) {

        std::vector<Real> b = cs.coterminalSwapRatesAnnuities();
        std::vector<Real> a = std::vector<Real>(b.size());
        Size n = b.size();
        const std::vector<Real> p = cs.discountRatios();
        const std::vector<Rate> f = cs.forwardRates();
        const std::vector<Time> t = cs.rateTaus();

        for (Size k=0; k<n; ++k)
            a[k] = p[k]-p[n];

        Matrix result = Matrix(n, n, 0.0);
        for (Size j=0; j<n; ++j) {     // j swap rate index
            for (Size i=j; i<n; ++i) { // i forward rate index
                Real temp = t[i]/1.0+f[i]*t[i];
                result[j][i] =
                    p[i+1]*t[i]/b[j]-temp*a[i]/b[j]+temp*a[j]*b[i]/(b[j]*b[j]);
            }
        }
        return result;
    }

    Disposable<Matrix> zMatrix(const CurveState& cs,
                               Rate displacement) {

        Matrix result = swapForwardJacobian(cs);
        const std::vector<Rate> f = cs.forwardRates();
        const std::vector<Rate> sr = cs.coterminalSwapRates();
        for (Size j=0; j<f.size(); ++j) {     // j swap rate index
            for (Size i=j; i<f.size(); ++i) { // i forward rate index
                result[j][i] *= (f[i]+displacement)/
                                (sr[j]+displacement);
            }
        }
        return result;
    }

    Disposable<Matrix> coefficientsCsi(const CurveState& cs, 
                                       Size N,               // N is index of expiry                   
                                       Size M){              // N+M is index of maturity (M swap tenor)
       
        Size L = M+N; //L is index of maturity
        std::vector<Real> b = cs.coterminalAnnuities(L);
        std::vector<Real> a = std::vector<Real>(b.size());
        Size n = b.size();
        const std::vector<Real> p = cs.discountRatios();
        const std::vector<Rate> f = cs.forwardRates();
        const std::vector<Time> t = cs.rateTaus();

        for (Size k=0; k<M; ++k)
            a[k] = p[N+k]-p[L];

        Matrix result = Matrix(M, M, 0.0);
        for (Size j=0; j<M; ++j) {    
            for (Size i=0; i<M; ++i) { 
                result[j][i] =
                    (p[i+1]*f[i]*t[i]/a[0] + f[i]*t[i]*(a[0]*b[i]-a[i]*b[0])/(a[0]*b[0]*(1.0+f[i]*t[i])))*
                    (p[j+1]*f[j]*t[j]/a[0] + f[j]*t[j]*(a[0]*b[j]-a[j]*b[0])/(a[0]*b[0]*(1.0+f[j]*t[j])));
            }
        }
        return result;
    }

}
