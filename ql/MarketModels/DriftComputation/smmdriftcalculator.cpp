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

#include <ql/MarketModels/DriftComputation/smmdriftcalculator.hpp>

namespace QuantLib {

    SMMDriftCalculator::SMMDriftCalculator(const Matrix& pseudo,
                                     const std::vector<Spread>& displacements,
                                     const std::vector<Time>& taus,
                                     Size numeraire,
                                     Size alive)
    : nRates_(taus.size()), nFactors_(pseudo.columns()),
      isFullFactor_(nFactors_==nRates_ ? true : false),
      numeraire_(numeraire), alive_(alive),
      displacements_(displacements), oneOverTaus_(taus.size()),
      pseudo_(pseudo),
      tmp_(taus.size(), 0.0),
      // zero initialization required for (used by) the last element
      wkaj_(pseudo_.columns(), pseudo_.rows(), 0.0),
      wkpj1_(pseudo_.columns(), pseudo_.rows(), 0.0),
      downs_(taus.size()), ups_(taus.size()) {

        // Check requirements
        QL_REQUIRE(nRates_>0, "Dim out of range");
        QL_REQUIRE(displacements.size() == nRates_,
            "Displacements out of range");
        QL_REQUIRE(pseudo.rows()==nRates_,
            "pseudo.rows() not consistent with dim");
        QL_REQUIRE(pseudo.columns()>0 && pseudo.columns()<=nRates_,
            "pseudo.rows() not consistent with pseudo.columns()");
        QL_REQUIRE(alive<nRates_, "Alive out of bounds");
        QL_REQUIRE(numeraire_<=nRates_, "Numeraire larger than dim");
        QL_REQUIRE(numeraire_>=alive, "Numeraire smaller than alive");

        // Precompute 1/taus
        for (Size i=0; i<taus.size(); ++i)
            oneOverTaus_[i] = 1.0/taus[i];

        // Compute covariance matrix from pseudoroot
        const Disposable<Matrix> pT = transpose(pseudo_);
        C_ = pseudo_*pT;

        // Compute lower and upper extrema for (non reduced) drift calculation
        for (Size i=alive_; i<nRates_; ++i) {
            downs_[i] = std::min(i+1, numeraire_);
            ups_[i]   = std::max(i+1, numeraire_);
        }
    }

    void SMMDriftCalculator::compute(const CurveState& cs,
                                  std::vector<Real>& drifts) const {

        // Compute drifts with factor reduction,
        // using the pseudo square root of the covariance matrix.

        // Precompute forwards factor
        for (Size i=alive_; i<nRates_; ++i)
            tmp_[i] = (forwards[i]+displacements_[i]) /
                (oneOverTaus_[i]+forwards[i]);

        // calculates and stores wkaj_, wkpj1_
        // assuming terminal bond measure
        // eq 5.4-5.7
        for (Size k=0; k<nFactors_; ++k) {
                // taken care in the constructor
                //wkpj1_[k][nRates_-1]= 0.0;
                //wkaj_[k][nRates_-1] = 0.0;
            for (Integer j=nRates_-2; j>=static_cast<Integer>(alive_); --j) {
                wkpj1_= 0.0; //fill in here
                wkaj_ = 0.0; //fill in here
            }
        }

        const std::vector<Real>& a = cs.coterminalSwapAnnuities();
        const std::vector<DiscountFactor>& d = cs.discountRatios();

        // we have computed cross variation with aj/pn
        // we want cross variation with aj/pN

        // we need to subtract < Wk, PN/pn> (pn/pN)*(Aj/pN)
        numeraireRatio = d[nRates_] / d[numeraire_];
        

  for (Size k=0; k<nFactors_; ++k) {
      // compute < Wk, PN/pn> 

      for (Size j=alive_; j<nRates_; ++j) 
      {
        wkajN_[k][j] = wkaj_[k][j] - wkpNn[k]*numeraireRatio*//(Aj/pN);
           
      }

   }

        // eq 5.3 (in log coordinates)
        for (Size j=alive_; j<nRates_; ++j) {
            drifts[j] = 0.0;
            for (Size k=0; k<nFactors_; ++k) {
                drifts[j] += wkajN_[k][j]*pseudo_[j][k];
            }
            drift[j] *= -d[numeraire_]/a[j];
        }

    }

}
