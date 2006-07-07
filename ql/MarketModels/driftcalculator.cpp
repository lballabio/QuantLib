/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 Silvia Frasson
 
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
                                     const Array& displacements,
                                     const Array& taus,
                                     Size numeraire,
                                     Size alive)
    : size_(taus.size()), pseudo_(pseudo), displacements_(displacements),
      taus_(taus),  numeraire_(numeraire), alive_(alive),
      tmp_(taus.size()), downs_(taus.size()), ups_(taus.size())
    {
        QL_REQUIRE(size_>0, "");
        QL_REQUIRE(displacements.size() == size_, "");
        QL_REQUIRE(pseudo.rows()==size_, "");
        QL_REQUIRE(pseudo.columns()>0 && pseudo.columns()<=size_, "");
        QL_REQUIRE(alive>=0 && alive<size_, "");
        QL_REQUIRE(numeraire_<=size_, "");
        QL_REQUIRE(numeraire_>=alive, "");
        const Disposable<Matrix> pT = transpose(pseudo_);

        C_ = pseudo_*pT;

        for (Size i=alive_; i<size_; ++i) {
            downs_[i] = std::min(i+1, numeraire_);
            ups_[i] = std::max(i+1, numeraire_);
        }
    }


    void DriftCalculator::compute(const Array& forwards, Array& drifts) const {

#if defined _DEBUG
        QL_REQUIRE(forwards.size() == size_, "");
        QL_REQUIRE(drifts.size() == size_, "");
#endif

        for(Size k=alive_; k<size_; ++k)
            tmp_[k] = (forwards[k]+displacements_[k]) /
                (1.0/taus_[k]+forwards[k]);

        for (Size i=alive_; i<size_; ++i) {
            drifts[i] = std::inner_product(tmp_.begin()+downs_[i],
                                           tmp_.begin()+ups_[i],
                                           C_.row_begin(i)+downs_[i], 0.0);

            //for (Size k=down; k<=up; ++k) {
            //    drifts[i] += tmp_[k] * C_[i][k];
            //}
            if (i+1<numeraire_)
                drifts[i] = -drifts[i];
        }
    }

}
