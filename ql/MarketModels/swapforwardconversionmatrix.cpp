/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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


#include <ql/MarketModels/swapforwardconversionmatrix.hpp>
#include <vector>

namespace QuantLib {

    Disposable<Matrix> swaptionsCovarianceMatrix(
                                    const Matrix& zMatrix, 
                                    const Matrix& forwardCovarianceMatrix)
    {
        Matrix result = zMatrix * forwardCovarianceMatrix * transpose(zMatrix);
        return result;
    }

    Disposable<Matrix> zMatrix(const CurveState& cs, 
                                       Size E,  // E = index of expiry                   
                                       Size ST, // ST = swap tenor
                                       Rate displacement)
    {
        Size M = ST+E; //M = index of maturity
        CurveState newCS(cs.rateTimes().begin()+E,cs.rateTimes().begin()+M);
        newCS.setOnForwardRates(cs.forwardRates().begin()+E,
                                cs.forwardRates().begin()+M);
        return zMatrixFull(newCS, displacement);
    }

    Disposable<Matrix> zMatrixFull(const CurveState& cs, Rate displacement) 
    {
        Matrix result = swapForwardJacobian(cs);    // derivative dsr[i]/df[j]
        const std::vector<Rate> f = cs.forwardRates();  // forward rates
        const std::vector<Rate> sr = cs.coterminalSwapRates();  // coterminal swap rates
        for (Size i=0; i<sr.size(); ++i) {      // i swap rate index
            for (Size j=i; j<f.size(); ++j) {   // j forward rate index
                result[i][j] *= (f[j]+displacement)/(sr[i]+displacement);
            }
        }
        return result;
    }

    Disposable<Matrix> swapForwardJacobian(const CurveState& cs) 
    {
        std::vector<Real> b = cs.coterminalSwapRatesAnnuities();    // coterminal annuities
        std::vector<Real> a = std::vector<Real>(b.size());          // coterminal floating leg values
        Size n = b.size();                                          // n° of coterminal swaps
        const std::vector<Real> p = cs.discountRatios();            // discount factors
        const std::vector<Rate> f = cs.forwardRates();              // forward rates
        const std::vector<Time> t = cs.rateTaus();                  // accrual factors

        for (Size k=0; k<n; ++k)    // compute coterminal floating leg values
            a[k] = p[k]-p[n];

        Matrix jacobian = Matrix(n, n, 0.0);
        for (Size i=0; i<n; ++i) {     // i swap rate index
            for (Size j=i; j<n; ++j) { // j forward rate index
                //Real temp = t[j]/1.0+f[j]*t[j];   ERROR!!
                //jacobian[i][j] =
                //    p[j+1]*t[j]/b[i]-temp*a[j]/b[i]+temp*a[i]*b[j]/(b[i]*b[i]);
                jacobian[i][j] =    // derivative dsr[i]/df[j]
                    p[j+1]*t[j]/b[i] + t[j]/(1.0+f[j]*t[j])*(-a[j]*b[i] + a[i]*b[j])/(b[i]*b[i]);
            }
        }
        return jacobian;
    }
}
