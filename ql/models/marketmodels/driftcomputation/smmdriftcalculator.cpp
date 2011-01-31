/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Mark Joshi

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

#include <ql/models/marketmodels/driftcomputation/smmdriftcalculator.hpp>
#include <ql/models/marketmodels/curvestates/coterminalswapcurvestate.hpp>

namespace QuantLib {

    SMMDriftCalculator::SMMDriftCalculator(
                                    const Matrix& pseudo,
                                    const std::vector<Spread>& displacements,
                                    const std::vector<Time>& taus,
                                    Size numeraire,
                                    Size alive)
    : numberOfRates_(taus.size()), numberOfFactors_(pseudo.columns()),
      isFullFactor_(numberOfFactors_==numberOfRates_ ? true : false),
      numeraire_(numeraire), alive_(alive),
      displacements_(displacements), oneOverTaus_(taus.size()),
      pseudo_(pseudo),
      tmp_(taus.size(), 0.0),
      // zero initialization required for (used by) the last element
      wkaj_(pseudo_.columns(), pseudo_.rows(), 0.0),
      wkpj_(pseudo_.columns(), pseudo_.rows()+1, 0.0),
      wkajshifted_(pseudo_.columns(), pseudo_.rows(), 0.0)
      /*,
      downs_(taus.size()), ups_(taus.size())*/ {

        // Check requirements
        QL_REQUIRE(numberOfRates_>0, "Dim out of range");
        QL_REQUIRE(displacements.size() == numberOfRates_,
            "Displacements out of range");
        QL_REQUIRE(pseudo.rows()==numberOfRates_,
            "pseudo.rows() not consistent with dim");
        QL_REQUIRE(pseudo.columns()>0 && pseudo.columns()<=numberOfRates_,
            "pseudo.rows() not consistent with pseudo.columns()");
        QL_REQUIRE(alive<numberOfRates_, "Alive out of bounds");
        QL_REQUIRE(numeraire_<=numberOfRates_, "Numeraire larger than dim");
        QL_REQUIRE(numeraire_>=alive, "Numeraire smaller than alive");

        // Precompute 1/taus
        for (Size i=0; i<taus.size(); ++i)
            oneOverTaus_[i] = 1.0/taus[i];

        // Compute covariance matrix from pseudoroot
        const Disposable<Matrix> pT = transpose(pseudo_);
        C_ = pseudo_*pT;

        // Compute lower and upper extrema for (non reduced) drift calculation
        //for (Size i=alive_; i<numberOfRates_; ++i) {
        //    downs_[i] = std::min(i+1, numeraire_);
        //    ups_[i]   = std::max(i+1, numeraire_);
        //}
    }

    void SMMDriftCalculator::compute(const CoterminalSwapCurveState& cs,
                                     std::vector<Real>& drifts) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(drifts.size()==cs.numberOfRates(),
                   "drifts.size() <> numberOfRates");
        #endif

        // Compute drifts with factor reduction,
        // using the pseudo square root of the covariance matrix.

        const std::vector<Rate>& SR=cs.coterminalSwapRates();
        // calculates and stores wkaj_, wkpj1_
        // assuming terminal bond measure
        // eq 5.4-5.7
        const std::vector<Time>& taus=cs.rateTaus();
        for (Size k=0; k<numberOfFactors_; ++k) {
                // taken care in the constructor
                // wkpj1_[k][numberOfRates_-1]= 0.0;
                // wkaj_[k][numberOfRates_-1] = 0.0;
            for (Integer j=numberOfRates_-2; j>=static_cast<Integer>(alive_)-1; --j) {
                 // < W(k) | P(j+1)/P(n) > =
                 // = SR(j+1) a(j+1,k) A(j+1) / P(n) + SR(j+1) < W(k) | A(j+1)/P(n) >
                Real annuity = cs.coterminalSwapAnnuity(numberOfRates_,j+1);
                wkpj_[k][j+1]= SR[j+1] *
                            ( pseudo_[j+1][k] * annuity +  wkaj_[k][j+1] )+
                            pseudo_[j+1][k]*displacements_[j+1]* annuity;

                if (j >=static_cast<Integer>(alive_))
                    wkaj_[k][j] = wkpj_[k][j+1]*taus[j ]+wkaj_[k][j+1];
            }
          }


        double numeraireRatio = cs.discountRatio(numberOfRates_,numeraire_);

// change to work for general numeraire
        for (Size k=0; k<numberOfFactors_; ++k) {
            // compute < Wk, PN/pn>
            for (Size j=alive_; j<numberOfRates_; ++j)
            {
                wkajshifted_[k][j] = -wkaj_[k][j]/cs.coterminalSwapAnnuity(numberOfRates_,j)
                                    + wkpj_[k][numeraire_]
                                                *numeraireRatio;
            }
        }

        // eq 5.3 (in log coordinates)
        for (Size j=alive_; j<numberOfRates_; ++j) {
            drifts[j] = 0.0;
            for (Size k=0; k<numberOfFactors_; ++k) {
                drifts[j] += wkajshifted_[k][j]*pseudo_[j][k];
            }
        }

    }

}
