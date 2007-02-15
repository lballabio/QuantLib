/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/MarketModels/DriftComputation/cmsmmdriftcalculator.hpp>


namespace QuantLib {

    CMSMMDriftCalculator::CMSMMDriftCalculator(const Matrix& pseudo,
        const std::vector<Spread>& displacements,
        const std::vector<Time>& taus,
        Size numeraire,
        Size alive,
        Size spanningFwds)
        : dim_(taus.size()), factors_(pseudo.columns()),
        isFullFactor_(factors_==dim_ ? true : false),
        numeraire_(numeraire), alive_(alive),
        displacements_(displacements), oneOverTaus_(taus.size()),
        pseudo_(pseudo), tmp_(taus.size(), 0.0),
        downs_(taus.size()), ups_(taus.size()),
        wkaj_(factors_,taus.size()),
        wkajN_(factors_,taus.size()),
        PjPnWk_(factors_,1+taus.size()),
        spanningFwds_(spanningFwds)
        {

            // Check requirements
            QL_REQUIRE(dim_>0, "Dim out of range");
            QL_REQUIRE(displacements.size() == dim_,
                "Displacements out of range");
            QL_REQUIRE(pseudo.rows()==dim_,
                "pseudo.rows() not consistent with dim");
            QL_REQUIRE(pseudo.columns()>0 && pseudo.columns()<=dim_,
                "pseudo.rows() not consistent with pseudo.columns()");
            QL_REQUIRE(alive<dim_, "Alive out of bounds");
            QL_REQUIRE(numeraire_<=dim_, "Numeraire larger than dim");
            QL_REQUIRE(numeraire_>=alive, "Numeraire smaller than alive");

            // Precompute 1/taus
            for (Size i=0; i<taus.size(); ++i)
                oneOverTaus_[i] = 1.0/taus[i];

            // Compute covariance matrix from pseudoroot
            const Disposable<Matrix> pT = transpose(pseudo_);
            C_ = pseudo_*pT;

            // Compute lower and upper extrema for (non reduced) drift calculation
            for (Size i=alive_; i<dim_; ++i) {
                downs_[i] = std::min(i+1, numeraire_);
                ups_[i]   = std::max(i+1, numeraire_);
                }
        }

    void CMSMMDriftCalculator::compute(const CMSwapCurveState& cs,
                                       std::vector<Real>& drifts) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(drifts.size()==cs.numberOfRates(),
                   "drifts.size() <> numberOfRates");
        #endif


        const std::vector<Time>& taus = cs.rateTaus();
        // final bond is numeraire

        // Compute cross variations
        for (Size k = 0; k<PjPnWk_.rows(); ++k) {
            PjPnWk_[k][dim_] = 0.0;
            wkaj_[k][dim_-1]=0.0;

            for (Integer j=static_cast<Integer>(dim_)-2;
                 j>=static_cast<Integer>(alive_)-1; --j)
            {
                double sr = cs.cmSwapRate(j+1,spanningFwds_);
                Integer endIndex = std::min(j+spanningFwds_+1,dim_);
                Real first = sr * wkaj_[k][j+1];
                Real second = cs.cmSwapAnnuity(numeraire_,j+1,spanningFwds_) 
                * (sr+displacements_[j+1])
                *pseudo_[j+1][k];
                Real third = PjPnWk_[k][endIndex];
                PjPnWk_[k][j+1] = first
                + second
                + third;

                if (j>=static_cast<Integer>(alive_))
                {
                    wkaj_[k][j] = wkaj_[k][j+1] + PjPnWk_[k][j+1]*taus[j];

                    if (j+spanningFwds_+1 <= dim_)
                        wkaj_[k][j] -= PjPnWk_[k][endIndex]*taus[endIndex-1];
                }

            }
        }

        Real PnOverPN = cs.discountRatio(dim_, numeraire_);
        Real PnOverPN = 1.0;

        for (Size j=0; j<dim_; ++j)
            for (Size k=0; k<factors_; ++k)
                wkajN_[k][j] =  wkaj_[k][j]*PnOverPN
                    -PjPnWk_[k][numeraire_]*PnOverPN*cs.cmSwapAnnuity(numeraire_,j,spanningFwds_);



        for (Size j = 0; j< dim_; ++j)
        {
            drifts[j]=0.0;
            for (Size k=0; k<factors_; ++k)
            {
                drifts[j] += pseudo_[j][k]*wkajN_[k][j];
            }
            drifts[j] /= -cs.cmSwapAnnuity(numeraire_,j,spanningFwds_);
        }
    }

}
