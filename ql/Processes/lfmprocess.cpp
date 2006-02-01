/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/schedule.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Processes/eulerdiscretization.hpp>
#include <ql/Processes/lfmprocess.hpp>

namespace QuantLib {

    LiborForwardModelProcess::LiborForwardModelProcess(
                                        Size size,
                                        const boost::shared_ptr<Xibor>& index)
    : StochasticProcess(boost::shared_ptr<discretization>(
                                                    new EulerDiscretization)),
      size_         (size),
      index_(index),
      initialValues_    (size_),
      fixingTimes_      (size_),
      fixingDates_      (size_),
      accrualStartTimes_(size),
      accrualEndTimes_  (size),
      accrualPeriod_    (size_) {

        const DayCounter dayCounter = index_->dayCounter();
        const std::vector<boost::shared_ptr<CashFlow> > flows = cashFlows();

        QL_REQUIRE(size_ == flows.size(), "wrong number of cashflows");

        const Date settlement = index_->termStructure()->referenceDate();
        const Date startDate =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                  flows[0])->fixingDate();

        for (Size i = 0; i < size_; ++i) {
            const boost::shared_ptr<FloatingRateCoupon> coupon =
               boost::dynamic_pointer_cast<FloatingRateCoupon>(flows[i]);

            QL_REQUIRE(coupon->date() == coupon->accrualEndDate(),
                       "irregular coupon types are not suppported");

            initialValues_[i] = coupon->rate();
            accrualPeriod_[i] = coupon->accrualPeriod();

            fixingDates_[i] = coupon->fixingDate();
            fixingTimes_[i] =
                dayCounter.yearFraction(startDate, coupon->fixingDate());
            accrualStartTimes_[i] =
                dayCounter.yearFraction(settlement,coupon->accrualStartDate());
            accrualEndTimes_[i]   =
                dayCounter.yearFraction(settlement,coupon->accrualEndDate());
        }
    }

    Disposable<Array> LiborForwardModelProcess::drift(Time t,
                                                      const Array& x) const {
        Array f(size_, 0.0);
        Matrix covariance(this->covariance(t, x, 1.0));

        const Size m = nextIndexReset(t);

        for (Size k=m; k<size_; ++k) {
            for (Size i=m; i<=k; ++i) {
                f[k]+= accrualPeriod_[i]*x[i]*covariance[i][k]
                    / (1+accrualPeriod_[i]*x[i]);
            }
            f[k]-=0.5*covariance[k][k];
        }

        return f;
    }

    Disposable<Matrix>
    LiborForwardModelProcess::diffusion(Time t, const Array& x) const {
        return lfmParam_->diffusion(t, x);
    }

    Disposable<Matrix> LiborForwardModelProcess::covariance(
        Time t, const Array& x, Time dt) const {
        return lfmParam_->covariance(t, x)*dt;
    }

    Disposable<Array> LiborForwardModelProcess::apply(
        const Array& x0, const Array& dx) const {
        Array tmp(size_);
        for (Size k=0; k<size_; ++k) {
            tmp[k] = x0[k] * std::exp(dx[k]);
        }
        return tmp;
    }

    Disposable<Array> LiborForwardModelProcess::evolve(
                                             Time t0, const Array& x0,
                                             Time dt, const Array& dw) const {
        // predictor-corrector step to reduce discretization errors
        Array rnd_0     = stdDeviation(t0, x0, dt)*dw;
        Array drift_0   = discretization_->drift(*this, t0, x0, dt);

        return apply(x0, ( drift_0 + discretization_
                ->drift(*this,t0,apply(x0, drift_0 + rnd_0),dt) )*0.5 + rnd_0);
    }

    Disposable<Array> LiborForwardModelProcess::initialValues() const {
        Array tmp = initialValues_;
        return tmp;
    }

    void LiborForwardModelProcess::setCovarParam(
             const boost::shared_ptr<LfmCovarianceParameterization> & param) {
        lfmParam_ = param;
    }

    boost::shared_ptr<LfmCovarianceParameterization>
    LiborForwardModelProcess::covarParam() const {
        return lfmParam_;
    }

    boost::shared_ptr<Xibor>
    LiborForwardModelProcess::index() const {
        return index_;
    }

    std::vector<boost::shared_ptr<CashFlow> >
    LiborForwardModelProcess::cashFlows(Real amount) const {
        const Date refDate = index_->termStructure()->referenceDate();

        return FloatingRateCouponVector(
                   Schedule(index_->calendar(),
                            refDate,
                            refDate + Period(index_->tenor().length()*size_,
                                             index_->tenor().units()),
                            index_->frequency(),
                            index_->businessDayConvention()
                            ),
                   index_->businessDayConvention(),
                   std::vector<Real>(1, amount),
                   index_,
                   index_->settlementDays(),
                   std::vector<Spread>(),
                   index_->dayCounter()
                   );
    }

    Size LiborForwardModelProcess::size() const {
        return size_;
    }

    Size LiborForwardModelProcess::factors() const {
        return lfmParam_->factors();
    }

    const std::vector<Time> & LiborForwardModelProcess::fixingTimes() const {
        return fixingTimes_;
    }

    const std::vector<Date> & LiborForwardModelProcess::fixingDates() const {
        return fixingDates_;
    }

    const std::vector<Time> &
    LiborForwardModelProcess::accrualStartTimes() const {
        return accrualStartTimes_;
    }

    const std::vector<Time> &
    LiborForwardModelProcess::accrualEndTimes() const {
        return accrualEndTimes_;
    }

    Size LiborForwardModelProcess::nextIndexReset(Time t) const {
        return std::upper_bound(fixingTimes_.begin(), fixingTimes_.end(), t)
                 - fixingTimes_.begin();
    }

    std::vector<DiscountFactor> LiborForwardModelProcess::discountBond(
        const std::vector<Rate> & rates) const {

        std::vector<DiscountFactor> discountFactors(size_);
        discountFactors[0] = 1.0/(1.0 + rates[0]*accrualPeriod_[0]);

        for (Size i = 1; i < size_; ++i) {
            discountFactors[i] =
                discountFactors[i-1]/(1.0 + rates[i]*accrualPeriod_[i]);
        }

        return discountFactors;
    }

}

