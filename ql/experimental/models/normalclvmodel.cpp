/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file normalclvmodel.cpp
*/

#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/experimental/models/normalclvmodel.hpp>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

namespace QuantLib {

    NormalCLVModel::NormalCLVModel(
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
        const boost::shared_ptr<OrnsteinUhlenbeckProcess>& ouProcess,
        const std::vector<Date>& maturityDates,
        Size lagrangeOrder, Real pMax, Real pMin)
    : x_(M_SQRT2*GaussHermiteIntegration(lagrangeOrder).x()),
      sigma_( (pMax != Null<Real>())
              ? x_.back() / InverseCumulativeNormal()(pMax)
              : (pMin != Null<Real>())
                  ? x_.front() / InverseCumulativeNormal()(pMin)
                  : 1.0),
      bsProcess_    (bsProcess),
      ouProcess_    (ouProcess),
      maturityDates_(maturityDates),
      pricingEngine_(boost::make_shared<AnalyticEuropeanEngine>(bsProcess_)) {

        registerWith(bsProcess_);
        registerWith(ouProcess_);

        maturityTimes_.reserve(maturityDates.size());
        std::transform(maturityDates_.begin(), maturityDates_.end(),
            std::back_inserter(maturityTimes_),
            boost::bind(&GeneralizedBlackScholesProcess::time,
                        bsProcess_, _1));
        std::sort(maturityTimes_.begin(), maturityTimes_.end());
    }

    Real NormalCLVModel::cdf(const Date& d, Real k) const {
        //k = std::max(k, 1e-3*bsProcess_->x0());

        const Handle<BlackVolTermStructure> volTS
            = bsProcess_->blackVolatility();

        const Real dk = 1e-3*k;
        const Real dvol_dk
            = (volTS->blackVol(d, k+dk) - volTS->blackVol(d, k-dk)) / (2*dk);

        const DiscountFactor df
            = bsProcess_->riskFreeRate()->discount(d);

        const boost::shared_ptr<Exercise> exercise
            = boost::make_shared<EuropeanExercise>(d);

        if (bsProcess_->x0() <=k) {
            VanillaOption option(
                boost::make_shared<PlainVanillaPayoff>(Option::Call, k),
                exercise);
            option.setPricingEngine(pricingEngine_);

            return 1.0 + (  option.strikeSensitivity()
                          + option.vega() * dvol_dk) /df;
        }
        else {
            VanillaOption option(
                boost::make_shared<PlainVanillaPayoff>(Option::Put, k),
                exercise);
            option.setPricingEngine(pricingEngine_);

            return (  option.strikeSensitivity()
                    + option.vega() * dvol_dk) /df;

        }
    }


    Real NormalCLVModel::invCDF(const Date& d, Real q) const {
        const Real fwd = bsProcess_->x0()
            / bsProcess_->riskFreeRate()->discount(d, true)
            * bsProcess_->dividendYield()->discount(d, true);

        const Volatility atmVol =
            bsProcess_->blackVolatility()->blackVol(d, fwd, true);

        const Real atmX = InverseCumulativeNormal()(q);
        const Time t = bsProcess_->time(d);

        const Real guess = fwd*std::exp(atmVol*atmX*std::sqrt(t));


        Real lower = guess;
        while (guess/lower < 65535.0 && cdf(d, lower) > q)
            lower*=0.5;

        Real upper = guess;
        while (upper/guess < 65535.0 && cdf(d, upper) < q) upper*=2;

        QL_REQUIRE(guess/lower < 65535.0 && upper/guess < 65535.0,
                "Could not find an start interval with ("
                << lower << ", " << upper << ") -> ("
                << cdf(d, lower) << ", " << cdf(d, upper) << ")");

        return Brent().solve(
            compose(std::bind2nd(std::minus<Real>(), q),
                boost::function<Real(Real)>(
                    boost::bind(&NormalCLVModel::cdf, this, d, _1))),
            1e-10, 0.5*(lower+upper), lower, upper);
    }

    Disposable<Array> NormalCLVModel::collocationPointsX(const Date& d) const {
        const Time t = bsProcess_->time(d);

        const Real expectation
            = ouProcess_->expectation(0.0, ouProcess_->x0(), t);
        const Real stdDeviation
            = ouProcess_->stdDeviation(0.0, ouProcess_->x0(), t);

        return expectation + stdDeviation*x_;
    }

    Disposable<Array> NormalCLVModel::collocationPointsY(const Date& d) const {
        Array s(x_.size());

        CumulativeNormalDistribution N;
        for (Size i=0, n=s.size(); i < n; ++i) {
            s[i] = invCDF(d, N(x_[i]/sigma_));
        }

        return s;
    }


    boost::function<Real(Time, Real)> NormalCLVModel::g() const {
        calculate();
        return g_;
    }

    NormalCLVModel::MappingFunction::MappingFunction(
        const NormalCLVModel& model)
    : y_(model.x_.size()),
      sigma_(model.sigma_),
      ouProcess_(model.ouProcess_),
      data_(boost::make_shared<InterpolationData>(model)) {

        for (Size i=0; i < data_->s_.columns(); ++i) {
            const Array y = model.collocationPointsY(model.maturityDates_[i]);
            std::copy(y.begin(), y.end(), data_->s_.column_begin(i));
        }

        for (Size i=0; i < data_->s_.rows(); ++i) {
            data_->interpl_.push_back(
                LinearInterpolation(data_->t_.begin(), data_->t_.end(),
                                    data_->s_.row_begin(i)));
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
        g_ = boost::function<Real(Time, Real)>(MappingFunction(*this));
    }
}
