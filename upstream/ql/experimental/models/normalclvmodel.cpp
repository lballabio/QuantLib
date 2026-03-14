/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file normalclvmodel.cpp
*/

#include <ql/exercise.hpp>
#include <ql/experimental/models/normalclvmodel.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/methods/finitedifferences/utilities/gbsmrndcalculator.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/ornsteinuhlenbeckprocess.hpp>
#include <utility>


namespace QuantLib {

    NormalCLVModel::NormalCLVModel(const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
                                   ext::shared_ptr<OrnsteinUhlenbeckProcess> ouProcess,
                                   const std::vector<Date>& maturityDates,
                                   Size lagrangeOrder,
                                   Real pMax,
                                   Real pMin)
    : x_(M_SQRT2 * GaussHermiteIntegration(lagrangeOrder).x()),
      sigma_((pMax != Null<Real>()) ?
                 x_.back() / InverseCumulativeNormal()(pMax) :
                 (pMin != Null<Real>()) ? x_.front() / InverseCumulativeNormal()(pMin) : Real(1.0)),
      bsProcess_(bsProcess), ouProcess_(std::move(ouProcess)), maturityDates_(maturityDates),
      rndCalculator_(ext::make_shared<GBSMRNDCalculator>(bsProcess)),
      maturityTimes_(maturityDates.size()) {

        registerWith(bsProcess_);
        registerWith(ouProcess_);

        for (Size i=0; i < maturityTimes_.size(); ++i) {
            maturityTimes_[i] = bsProcess_->time(maturityDates[i]);
            QL_REQUIRE(i==0 || maturityTimes_[i-1] < maturityTimes_[i],
                    "dates must be sorted");
        }
    }

    Real NormalCLVModel::cdf(const Date& d, Real k) const {
        return rndCalculator_->cdf(k, bsProcess_->time(d));
    }


    Real NormalCLVModel::invCDF(const Date& d, Real q) const {
        return rndCalculator_->invcdf(q, bsProcess_->time(d));
    }

    Array NormalCLVModel::collocationPointsX(const Date& d) const {
        const Time t = bsProcess_->time(d);

        const Real expectation
            = ouProcess_->expectation(0.0, ouProcess_->x0(), t);
        const Real stdDeviation
            = ouProcess_->stdDeviation(0.0, ouProcess_->x0(), t);

        return expectation + stdDeviation*x_;
    }

    Array NormalCLVModel::collocationPointsY(const Date& d) const {
        Array s(x_.size());

        CumulativeNormalDistribution N;
        for (Size i=0, n=s.size(); i < n; ++i) {
            s[i] = invCDF(d, N(x_[i]/sigma_));
        }

        return s;
    }


    std::function<Real(Time, Real)> NormalCLVModel::g() const {
        calculate();
        return g_;
    }

    NormalCLVModel::MappingFunction::MappingFunction(
        const NormalCLVModel& model)
    : y_(model.x_.size()),
      sigma_(model.sigma_),
      ouProcess_(model.ouProcess_),
      data_(ext::make_shared<InterpolationData>(model)) {

        for (Size i=0; i < data_->s_.columns(); ++i) {
            const Array y = model.collocationPointsY(model.maturityDates_[i]);
            std::copy(y.begin(), y.end(), data_->s_.column_begin(i));
        }

        for (Size i=0; i < data_->s_.rows(); ++i) {
            data_->interpl_.emplace_back(data_->t_.begin(), data_->t_.end(),
                                         data_->s_.row_begin(i));
        }
    }


    Real NormalCLVModel::MappingFunction::operator()(Time t, Real x) const {
        for (Size i=0; i < y_.size(); ++i) {
            y_[i] = data_->interpl_[i](t, true);
        }

        const Real expectation
            = ouProcess_->expectation(0.0, ouProcess_->x0(), t);
        const Real stdDeviation
            = ouProcess_->stdDeviation(0.0, ouProcess_->x0(), t);

        const Real r = sigma_*(x-expectation)/stdDeviation;

        return data_->lagrangeInterpl_.value(y_, r);
    }

    void NormalCLVModel::performCalculations() const {
        g_ = std::function<Real(Time, Real)>(MappingFunction(*this));
    }
}
