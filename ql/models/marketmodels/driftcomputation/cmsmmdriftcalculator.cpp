/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Mark Joshi

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

#include <ql/models/marketmodels/driftcomputation/cmsmmdriftcalculator.hpp>
#include <ql/models/marketmodels/curvestates/cmswapcurvestate.hpp>

namespace QuantLib {

    CMSMMDriftCalculator::CMSMMDriftCalculator(
                                const Matrix& pseudo,
                                const std::vector<Spread>& displacements,
                                const std::vector<Time>& taus,
                                Size numeraire,
                                Size alive,
                                Size spanningFwds)
    : numberOfRates_(taus.size()), numberOfFactors_(pseudo.columns()),
      numeraire_(numeraire), alive_(alive),
      displacements_(displacements), oneOverTaus_(taus.size()),
      pseudo_(pseudo), tmp_(taus.size(), 0.0),
      PjPnWk_(numberOfFactors_,1+taus.size()),
      wkaj_(numberOfFactors_, taus.size()),
      wkajN_(numberOfFactors_, taus.size()),
      downs_(taus.size()), ups_(taus.size()),
      spanningFwds_(spanningFwds) {

        // Check requirements
        QL_REQUIRE(numberOfRates_>0, "Dim out of range");
        QL_REQUIRE(displacements.size() == numberOfRates_,
                   "Displacements out of range");
        QL_REQUIRE(pseudo.rows()==numberOfRates_,
                   "pseudo.rows() not consistent with dim");
        QL_REQUIRE(pseudo.columns()>0 && pseudo.columns()<=numberOfRates_,
                   "pseudo.rows() not consistent with pseudo.columns()");
        QL_REQUIRE(alive<numberOfRates_,
                   "Alive out of bounds");
        QL_REQUIRE(numeraire_<=numberOfRates_,
                   "Numeraire larger than dim");
        QL_REQUIRE(numeraire_>=alive,
                   "Numeraire smaller than alive");

        // Precompute 1/taus
        for (Size i=0; i<taus.size(); ++i)
            oneOverTaus_[i] = 1.0/taus[i];

        // Compute covariance matrix from pseudoroot
        Matrix pT = transpose(pseudo_);
        C_ = pseudo_*pT;

        // Compute lower and upper extrema for (non reduced) drift calculation
        for (Size i=alive_; i<numberOfRates_; ++i) {
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
        for (Size k=0; k<PjPnWk_.rows(); ++k) {
            PjPnWk_[k][numberOfRates_]=0.0;
            wkaj_[k][numberOfRates_-1]=0.0;

            for (Integer j=static_cast<Integer>(numberOfRates_)-2;
                 j>=static_cast<Integer>(alive_)-1; --j)
            {
                Real sr = cs.cmSwapRate(j+1,spanningFwds_);
                Integer endIndex =
                    std::min<Integer>(j + static_cast<Integer>(spanningFwds_) + 1,
                             static_cast<Integer>(numberOfRates_));
                Real first = sr * wkaj_[k][j+1];
                Real second = cs.cmSwapAnnuity(numberOfRates_,j+1,spanningFwds_)
                * (sr+displacements_[j+1])
                *pseudo_[j+1][k];
                Real third = PjPnWk_[k][endIndex];
                PjPnWk_[k][j+1] = first
                + second
                + third;

                if (j>=static_cast<Integer>(alive_))
                {
                    wkaj_[k][j] = wkaj_[k][j+1] + PjPnWk_[k][j+1]*taus[j];

                    if (j+spanningFwds_+1 <= numberOfRates_)
                        wkaj_[k][j] -= PjPnWk_[k][endIndex]*taus[endIndex-1];
                }

            }
        }

        Real PnOverPN = cs.discountRatio(numberOfRates_, numeraire_);
        //Real PnOverPN = 1.0;

        for (Size j=alive_; j<numberOfRates_; ++j)
            for (Size k=0; k<numberOfFactors_; ++k)
                wkajN_[k][j] =  wkaj_[k][j]*PnOverPN
                    -PjPnWk_[k][numeraire_]*PnOverPN*cs.cmSwapAnnuity(numeraire_,j,spanningFwds_);



        for (Size j=alive_; j<numberOfRates_; ++j)
        {
            drifts[j]=0.0;
            for (Size k=0; k<numberOfFactors_; ++k)
            {
                drifts[j] += pseudo_[j][k]*wkajN_[k][j];
            }
            drifts[j] /= -cs.cmSwapAnnuity(numeraire_,j,spanningFwds_);
        }
    }

}
