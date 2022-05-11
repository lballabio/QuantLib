/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/legacy/libormarketmodels/lfmprocess.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>
#include <utility>

namespace QuantLib {

    LiborForwardModelProcess::LiborForwardModelProcess(Size size, ext::shared_ptr<IborIndex> index)
    : StochasticProcess(ext::shared_ptr<discretization>(new EulerDiscretization)), size_(size),
      index_(std::move(index)), initialValues_(size_), fixingTimes_(size_), fixingDates_(size_),
      accrualStartTimes_(size), accrualEndTimes_(size), accrualPeriod_(size_), m1(size_),
      m2(size_) {

        const DayCounter dayCounter = index_->dayCounter();
        const Leg flows = cashFlows();

        QL_REQUIRE(size_ == flows.size(), "wrong number of cashflows");

        Date settlement = index_->forwardingTermStructure()->referenceDate();
        const Date startDate =
            ext::dynamic_pointer_cast<IborCoupon>(flows[0])->fixingDate();

        for (Size i = 0; i < size_; ++i) {
            const ext::shared_ptr<IborCoupon> coupon =
               ext::dynamic_pointer_cast<IborCoupon>(flows[i]);

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

    Array LiborForwardModelProcess::drift(Time t,
                                          const Array& x) const {
        Array f(size_, 0.0);
        Matrix covariance(lfmParam_->covariance(t, x));

        const Size m = nextIndexReset(t);

        for (Size k=m; k<size_; ++k) {
            m1[k] = accrualPeriod_[k]*x[k]/(1+accrualPeriod_[k]*x[k]);
            f[k]  = std::inner_product(m1.begin()+m, m1.begin()+k+1,
                                       covariance.column_begin(k)+m,0.0)
                    - 0.5*covariance[k][k];
        }

        return f;
    }

    Matrix LiborForwardModelProcess::diffusion(Time t, const Array& x) const {
        return lfmParam_->diffusion(t, x);
    }

    Matrix LiborForwardModelProcess::covariance(Time t, const Array& x, Time dt) const {
        return lfmParam_->covariance(t, x)*dt;
    }

    Array LiborForwardModelProcess::apply(const Array& x0, const Array& dx) const {
        Array tmp(size_);

        for (Size k=0; k<size_; ++k) {
            tmp[k] = x0[k] * std::exp(dx[k]);
        }

        return tmp;
    }

    Array LiborForwardModelProcess::evolve(Time t0, const Array& x0,
                                           Time dt, const Array& dw) const {
        /* predictor-corrector step to reduce discretization errors.

           Short - but slow - solution would be

           Array rnd_0     = stdDeviation(t0, x0, dt)*dw;
           Array drift_0   = discretization_->drift(*this, t0, x0, dt);

           return apply(x0, ( drift_0 + discretization_
                ->drift(*this,t0,apply(x0, drift_0 + rnd_0),dt) )*0.5 + rnd_0);

           The following implementation does the same but is faster.
        */

        const Size m   = nextIndexReset(t0);
        const Real sdt = std::sqrt(dt);

        Array f(x0);
        Matrix diff       = lfmParam_->diffusion(t0, x0);
        Matrix covariance = lfmParam_->covariance(t0, x0);

        for (Size k=m; k<size_; ++k) {
            const Real y = accrualPeriod_[k]*x0[k];
            m1[k] = y/(1+y);
            const Real d = (
                std::inner_product(m1.begin()+m, m1.begin()+k+1,
                                   covariance.column_begin(k)+m,0.0)
                -0.5*covariance[k][k]) * dt;

            const Real r = std::inner_product(
                diff.row_begin(k), diff.row_end(k), dw.begin(), 0.0)*sdt;

            const Real x = y*std::exp(d + r);
            m2[k] = x/(1+x);
            f[k] = x0[k] * std::exp(0.5*(d+
                 (std::inner_product(m2.begin()+m, m2.begin()+k+1,
                                     covariance.column_begin(k)+m,0.0)
                  -0.5*covariance[k][k])*dt)+ r);
        }

        return f;
    }

    Array LiborForwardModelProcess::initialValues() const {
        return initialValues_;
    }

    void LiborForwardModelProcess::setCovarParam(
             const ext::shared_ptr<LfmCovarianceParameterization> & param) {
        lfmParam_ = param;
    }

    ext::shared_ptr<LfmCovarianceParameterization>
    LiborForwardModelProcess::covarParam() const {
        return lfmParam_;
    }

    ext::shared_ptr<IborIndex>
    LiborForwardModelProcess::index() const {
        return index_;
    }

    Leg
    LiborForwardModelProcess::cashFlows(Real amount) const {
        Date refDate = index_->forwardingTermStructure()->referenceDate();
        Schedule schedule(refDate,
                          refDate + Period(index_->tenor().length()*size_,
                                           index_->tenor().units()),
                          index_->tenor(), index_->fixingCalendar(),
                          index_->businessDayConvention(),
                          index_->businessDayConvention(),
                          DateGeneration::Forward, false);
        return IborLeg(schedule,index_)
            .withNotionals(amount)
            .withPaymentDayCounter(index_->dayCounter())
            .withPaymentAdjustment(index_->businessDayConvention())
            .withFixingDays(index_->fixingDays());
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

