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

#include <ql/experimental/models/cclgm1.hpp>
#include <ql/experimental/models/lgm1.hpp>

namespace QuantLib {

CcLgm1::CcLgm1(const std::vector<boost::shared_ptr<
                   Lgm<detail::LgmPiecewiseAlphaConstantKappa> > > &models,
               const std::vector<Handle<Quote> > &fxSpots,
               const std::vector<Date> &fxVolStepDates,
               const std::vector<std::vector<Real> > &fxVolatilities,
               const Matrix &correlation,
               const std::vector<Handle<YieldTermStructure> > &curves)
    : CcLgm<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
            detail::LgmPiecewiseAlphaConstantKappa>(models),
      CalibratedModel(fxSpots.size()), fxSpots_(fxSpots),
      fxVolStepDates_(fxVolStepDates), fxVolatilities_(fxVolatilities),
      correlation_(correlation), curves_(curves) {
    initialize();
}

void CcLgm1::initialize() {
    QL_REQUIRE(curves_.size() == n() + 1,
               "there must be n+1 = " << n() + 1 << " curves, "
                                      << curves_.size() << " given.");
    QL_REQUIRE(fxSpots_.size() == n(),
               "there must be n = " << n() << " fx spots, " << fxSpots_.size()
                                    << " given.");

    QL_REQUIRE(fxVolatilities_.size() == n(),
               "there must be n = " << n() << " fx volatility vectors, "
                                    << fxVolatilities_.size() << " given.");

    for (Size i = 0; i < n(); ++i)
        QL_REQUIRE(fxVolatilities_[i].size() == fxVolStepDates_.size() + 1,
                   "the must be k+1 = "
                       << fxVolStepDates_.size() + 1
                       << " fx volatilities given, but for fx pair " << i
                       << " there are " << fxVolatilities_[i].size() << ".");

    fxVolStepTimesArray_ = Array(fxVolStepDates_.size());
    updateTimes();

    for (Size i = 0; i < n(); ++i) {
        arguments_[i] =
            PiecewiseConstantParameter(fxVolStepTimes_, NoConstraint());
        for (Size j = 0; j < fxVolatilities_[i].size(); ++j) {
            arguments_[i].setParam(j, fxVolatilities_[i][j]);
        }
    }

    for (Size i = 0; i < n() + 1; ++i) {
        if (curves_[i].empty()) {
            curves_[i] = model(i)->termStructure();
        } else {
            registerWith(curves_[i]);
        }
    }

    std::vector<boost::shared_ptr<detail::LgmFxParametrization<
        detail::LgmFxPiecewiseSigma> > > fxParametrizations;
    std::vector<boost::shared_ptr<detail::LgmParametrization<
        detail::LgmPiecewiseAlphaConstantKappa> > > lgmParametrizations;
    for (Size i = 0; i < n(); ++i) {
        fxParametrizations.push_back(
            boost::make_shared<detail::LgmFxPiecewiseSigma>(
                fxVolStepTimesArray_, arguments_[i].params()));
    }
    for (Size i = 0; i < n() + 1; ++i) {
        lgmParametrizations.push_back(model(i)->parametrization());
    }

    setParametrization(boost::make_shared<detail::CcLgmPiecewise>(
        fxParametrizations, lgmParametrizations, correlation_));

    process_ = boost::make_shared<
        CcLgmProcess<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
                     detail::LgmPiecewiseAlphaConstantKappa> >(
        parametrization(), fxSpots_, curves_);
    registerWith(process_);
}

void CcLgm1::updateTimes() const {
    fxVolStepTimes_.clear();
    int j = 0;
    for (std::vector<Date>::const_iterator i = fxVolStepDates_.begin();
         i != fxVolStepDates_.end(); ++i, ++j) {
        fxVolStepTimes_.push_back(
            model(0)->termStructure()->timeFromReference(*i));
        fxVolStepTimesArray_[j] = fxVolStepTimes_[j];
        if (j == 0)
            QL_REQUIRE(fxVolStepTimes_[0] > 0.0,
                       "fx volsteptimes must be positive ("
                           << fxVolStepTimes_[0] << ")");
        else
            QL_REQUIRE(fxVolStepTimes_[j] > fxVolStepTimes_[j - 1],
                       "fx volsteptimes must be increasing ("
                           << fxVolStepTimes_[j - 1] << "@" << (j - 1) << ", "
                           << fxVolStepTimes_[j] << "@" << j << ")");
    }
    if (stateProcess() != NULL)
        boost::static_pointer_cast<
            CcLgmProcess<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
                         detail::LgmPiecewiseAlphaConstantKappa> >(
            stateProcess())
            ->flushCache();
    if (parametrization() != NULL)
        parametrization()->update();
}
} // namespace QuantLib
