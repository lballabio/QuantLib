/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Silvia Frasson
 Copyright (C) 2006 Mario Pucci

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

#include <ql/MarketModels/driftcalculator.hpp>
#include <vector>

namespace QuantLib {

    DriftCalculator::DriftCalculator(const Matrix& pseudo,
                                     const std::vector<Rate>& displacements,
                                     const std::vector<Time>& taus,
                                     Size numeraire,
                                     Size alive)
    : dim_(taus.size()), numeraire_(numeraire), alive_(alive),
      displacements_(displacements), oneOverTaus_(taus.size()), pseudo_(pseudo), 
      tmp_(taus.size()), e_(pseudo_.columns(), pseudo_.rows(), 0.0),
      downs_(taus.size()), ups_(taus.size())
    {
        QL_REQUIRE(dim_>0, "");
        QL_REQUIRE(displacements.size() == dim_, "");
        QL_REQUIRE(pseudo.rows()==dim_, "");
        QL_REQUIRE(pseudo.columns()>0 && pseudo.columns()<=dim_, "");
        QL_REQUIRE(alive>=0 && alive<dim_, "");
        QL_REQUIRE(numeraire_<=dim_, "");
        QL_REQUIRE(numeraire_>=alive, "");


        for (Size i=0; i<taus.size(); ++i)
            oneOverTaus_[i] = 1.0/taus[i];

        const Disposable<Matrix> pT = transpose(pseudo_);
        C_ = pseudo_*pT;
        for (Size i=alive_; i<dim_; ++i) {
            downs_[i] = std::min(i+1, numeraire_);
            ups_[i]   = std::max(i+1, numeraire_);
        }
    }

    void DriftCalculator::compute(const std::vector<Rate>& forwards,
                                  std::vector<Real>& drifts) const {

        #if defined _DEBUG
            QL_REQUIRE(forwards.size() == dim_, "");
            QL_REQUIRE(drifts.size() == dim_, "");
        #endif

        // Precompute forward term
        Size i;
        for(i=alive_; i<dim_; ++i)
            tmp_[i] = (forwards[i]+displacements_[i]) /
                      (oneOverTaus_[i]+forwards[i]);
        // Compute drifts without factor reduction,
        // using directly the covariance matrix.
        // for (Size k=down; k<=up; ++k) drifts[i] += tmp_[k] * C_[i][k];
        for (i=alive_; i<dim_; ++i) {
            drifts[i] = std::inner_product(tmp_.begin()+downs_[i],
                                           tmp_.begin()+ups_[i],
                                           C_.row_begin(i)+downs_[i], 0.0);
            if (i+1<numeraire_) drifts[i] = -drifts[i];
        }
    }

      void DriftCalculator::computeReduced(const std::vector<Rate>& forwards,
                                           Size factors,
                                           std::vector<Real>& drifts) const {

        //#if defined _DEBUG
        QL_REQUIRE(forwards.size()==dim_, "forwards.size()==dim_");
        QL_REQUIRE(drifts.size()==dim_, "drifts.size()==dim_");
        QL_REQUIRE(factors>0, "factors>0");
        QL_REQUIRE(factors<=pseudo_.columns(), "factors<=pseudo_.columns()");
        //#endif

        // Precompute forwards factor
        for(Size i=alive_; i<dim_; ++i)
            tmp_[i] = (forwards[i]+displacements_[i]) /
                      (oneOverTaus_[i]+forwards[i]);

        // Compute drifts with factor reduction,
        // using pseudo square root of the covariance matrix.
        // Divide the summation into 3 terms:

        // 1st: the drift corresponding to the numeraire P_N is zero:
        if (numeraire_>0) drifts[numeraire_-1] = 0.0;

        // 2nd: then, move backward from N-2 (included) back to alive (included):
        Integer alive = alive_;
        for (Integer i=numeraire_-2; i>=alive; --i) {
        //  for (Size r=0; r<pseudo_.columns(); ++r) {
            for (Size r=0; r<factors; ++r) {
                e_[r][i] = e_[r][i+1] + tmp_[i+1] * pseudo_[i+1][r];
            }
            drifts[i] = - std::inner_product(e_.column_begin(i)+downs_[i],
                                             e_.column_begin(i)+ups_[i],
                                             pseudo_.row_begin(i)+downs_[i],
                                             0.0);
        }

        // 3rd: now, move forward from N (included) up to to n (excluded):
        for (Size i=numeraire_; i<dim_; ++i) {
        //  for (r=0; r<pseudo_.columns(); ++r) {
            for (Size r=0; r<factors; ++r) {
                e_[r][i] = e_[r][i-1]+tmp_[i]*pseudo_[i][r];
            }
            drifts[i] = std::inner_product(e_.column_begin(i)+downs_[i],
                                           e_.column_begin(i)+ups_[i],
                                           pseudo_.row_begin(i)+downs_[i],
                                           0.0);
        }
      }

}
