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
<http://quantlib.org/license.shtml>.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <vector>

namespace QuantLib {


    Real SwapForwardMappings::annuity(const CurveState& cs, Size startIndex, Size endIndex,Size numeraireIndex)
    {
        Real annuity =0.0;
        for (Size i=startIndex; i < endIndex; ++i)
            annuity+= cs.rateTaus()[i]*cs.discountRatio(i+1,numeraireIndex);

        return annuity;

    }

    // compute derivative of swap-rate to underlying forward rate
    Real SwapForwardMappings::swapDerivative(const CurveState& cs, Size startIndex, Size endIndex, Size forwardIndex)
    {
        if (forwardIndex < startIndex)
            return 0.0;
        if (forwardIndex >= endIndex)
            return 0.0;

        Real numerator = cs.discountRatio(endIndex,startIndex)-1;
        Real swapAnnuity = annuity(cs, startIndex, endIndex,endIndex);

        Real ratio = cs.rateTaus()[forwardIndex]/(1+ cs.rateTaus()[forwardIndex]*cs.forwardRate(forwardIndex));

        Real part1 = ratio*numerator/swapAnnuity;
        Real part2 = numerator/(swapAnnuity*swapAnnuity);

        part2*= ratio* annuity(cs,forwardIndex,endIndex,endIndex);


        return part1-part2;
    }

    Disposable<Matrix>
        SwapForwardMappings::coterminalSwapForwardJacobian(const CurveState& cs)
    {
        Size n = cs.numberOfRates();
        const std::vector<Rate>& f = cs.forwardRates();
        const std::vector<Time>& tau = cs.rateTaus();

        // coterminal floating leg values
        std::vector<Real> a(n);
        for (Size k=0; k<n; ++k)
            a[k] = cs.discountRatio(k,n)-1.0;
        //p[k]-p[n];

        Matrix jacobian = Matrix(n, n, 0.0);
        for (Size i=0; i<n; ++i) {     // i = swap rate index
            for (Size j=i; j<n; ++j) { // j = forward rate index
                Real bi = cs.coterminalSwapAnnuity(n,i);
                Real bj = cs.coterminalSwapAnnuity(n,j);
                jacobian[i][j] =
                    //   p[j+1]*tau[j]/b[i] +
                    tau[j]/cs.coterminalSwapAnnuity(j+1,i) +
                    // tau[j]/(1.0+f[j]*tau[j]) *
                    tau[j]/(1.0+f[j]*tau[j]) *
                    //    (-a[j]*b[i]+a[i]*b[j])/(b[i]*b[i]);
                    (-a[j]*bi+a[i]*bj)/(bi*bi);

            }
        }
        return jacobian;
    }

    Disposable<Matrix>
        SwapForwardMappings::coterminalSwapZedMatrix(const CurveState& cs,
        const Spread displacement) {
            Size n = cs.numberOfRates();
            Matrix zMatrix = coterminalSwapForwardJacobian(cs);
            const std::vector<Rate>& f = cs.forwardRates();
            const std::vector<Rate>& sr = cs.coterminalSwapRates();
            for (Size i=0; i<n; ++i)
                for (Size j=i; j<n; ++j)
                    zMatrix[i][j] *= (f[j]+displacement)/(sr[i]+displacement);
            return zMatrix;
    }


    Disposable<Matrix>
        SwapForwardMappings::coinitialSwapForwardJacobian(const CurveState& cs)
    {
        Size n = cs.numberOfRates();

        Matrix jacobian = Matrix(n, n, 0.0);
        for (Size i=0; i<n; ++i)      // i = swap rate index
            for (Size j=0; j<n; ++j)  // j = forward rate index
                jacobian[i][j] =swapDerivative(cs, 0, i+1, j);

        return jacobian;
    }

    Disposable<Matrix>
        SwapForwardMappings::coinitialSwapZedMatrix(const CurveState& cs,
        const Spread displacement)
    {
        Size n = cs.numberOfRates();
        Matrix zMatrix = coinitialSwapForwardJacobian(cs);
        const std::vector<Rate>& f = cs.forwardRates();
        std::vector<Rate> sr(n);

        for (Size i=0; i<n; ++i)
            sr[i] = cs.cmSwapRate(0,i+1);

        for (Size i=0; i<n; ++i)
            for (Size j=i; j<n; ++j)
                zMatrix[i][j] *= (f[j]+displacement)/(sr[i]+displacement);
        return zMatrix;


    }


}
