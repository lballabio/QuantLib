/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

#include <ql/experimental/models/longstaffschwartzproxypathpricer.hpp>

namespace QuantLib {

LongstaffSchwartzProxyPathPricer::LongstaffSchwartzProxyPathPricer(
    const TimeGrid &times,
    const boost::shared_ptr<EarlyExercisePathPricer<PathType> > &pricer,
    const boost::shared_ptr<YieldTermStructure> &termStructure)
    : LongstaffSchwartzPathPricer<PathType>(times, pricer, termStructure),
      coeffItm_(times.size() - 1), coeffOtm_(times.size() - 1) {}

void LongstaffSchwartzProxyPathPricer::post_processing(
    const Size i, const std::vector<StateType> &state,
    const std::vector<Real> &price, const std::vector<Real> &exercise) {

    std::vector<StateType> x_itm, x_otm;
    std::vector<Real> y_itm, y_otm;

    cutoff_ = -QL_MAX_REAL;

    for (Size j = 0; j < state.size(); ++j) {
        if (exercise[j] > 0.0) {
            x_itm.push_back(state[j]);
            y_itm.push_back(price[j]);
        } else {
            x_otm.push_back(state[j]);
            y_otm.push_back(price[j]);
            if(state[j]>cutoff_)
                cutoff_ = state[j];
        }
    }

    if (v_.size() <= x_itm.size()) {
        coeffItm_[i - 1] =
            GeneralLinearLeastSquares(x_itm, y_itm, v_).coefficients();
    } else {
        // see longstaffschwartzpricer.hpp
        coeffItm_[i - 1] = Array(v_.size(), 0.0);
    }

    if (v_.size() <= x_otm.size()) {
        coeffOtm_[i - 1] =
            GeneralLinearLeastSquares(x_otm, y_otm, v_).coefficients();
    } else {
        // see longstaffschwartzpricer.hpp
        coeffOtm_[i - 1] = Array(v_.size(), 0.0);
    }
}

} // namespace QuantLib
