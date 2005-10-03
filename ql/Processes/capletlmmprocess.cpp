/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

#include <ql/Processes/capletlmmprocess.hpp>
#include <ql/schedule.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Processes/eulerdiscretization.hpp>

namespace QuantLib {

    CapletLiborMarketModelProcess::CapletLiborMarketModelProcess(
                Size len,
                const boost::shared_ptr<Xibor>& index,
                const boost::shared_ptr<CapletVolatilityStructure>& capletVol,
                const Matrix& volaComp)
    : StochasticProcess(boost::shared_ptr<discretization>(
                                                    new EulerDiscretization)),
          len_          (len),
          factors_      (volaComp.columns() ? volaComp.columns() : Size(1)),
          lambda_       (len_-1, factors_),
          lambdaSquare_ (len_-1, len_-1),

          index_(index),
          initialValues_(len_),

          delta_        (len_),
          fixingDates_  (len_),
          accrualPeriod_(len_) {

        const Matrix& components =
            (!volaComp.empty() ? volaComp : Matrix(len_-1, 1, 1.0));

        QL_REQUIRE(   components.rows() == len_-1
                   && components.columns() == factors_,
                   "wrong dimension of the correlation matrix");

        DayCounter dayCounter = index_->dayCounter();
        Date refDate = index_->termStructure()->referenceDate();

        std::vector<boost::shared_ptr<CashFlow> > cashFlows =
            FloatingRateCouponVector(
                Schedule(index_->calendar(),
                         refDate,
                         refDate + Period(index_->tenor().length()*len_+1,
                                          index_->tenor().units()),
                         index_->frequency(),
                         index_->businessDayConvention()
                         ),
                index_->businessDayConvention(),
                std::vector<Real>(1, 1.0),
                index_,
                index_->settlementDays(),
                std::vector<Spread>(),
                dayCounter);

        QL_REQUIRE(len_+1 == cashFlows.size(), "wrong number of cashflows");

        const Date startDate =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                  cashFlows[0])->fixingDate();

        Real tmp = 0.0;
        for (Size i = 0; i < len_; ++i) {
            boost::shared_ptr<FloatingRateCoupon> coupon =
               boost::dynamic_pointer_cast<FloatingRateCoupon>(cashFlows[i]);

            initialValues_[i] = coupon->rate();
            fixingDates_[i]   = coupon->fixingDate();
            accrualPeriod_[i] = coupon->accrualPeriod();
            delta_[i]         =
                dayCounter.yearFraction(coupon->fixingDate(),
                    boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                cashFlows[i+1])->fixingDate());

            m_.insert(std::map<Time, Size, std::less<Time> >::value_type(
                 dayCounter.yearFraction(startDate, coupon->fixingDate()), i));

            if (i > 0) {
                const Volatility vol = capletVol->volatility(
                    coupon->fixingDate(), coupon->rate());

                const Time volT_i = capletVol->dayCounter().yearFraction(
                    startDate, coupon->fixingDate());

                const Volatility l = vol*vol*volT_i - tmp;
                tmp += l;

                const Volatility lambda = std::sqrt(l / delta_[i-1]);
                for (Size q=0; q<factors_; ++q) {
                    lambda_[i-1][q] = components[i-1][q] * lambda;
                }
            }
        }

        lambdaSquare_ = lambda_ * transpose(lambda_);
    }

    Disposable<Array> CapletLiborMarketModelProcess::drift(
                                               Time t, const Array& x) const {
        Array f(size(), 0.0);
        const Size m = nextResetDate(t);

        for (Size k=0; k<size(); ++k) {
            for (Size i=m; i<=k; ++i) {
                f[k]+= delta_[i]*x[i]*lambdaSquare_[i-m][k-m]
                       / (1+delta_[i]*x[i]);
            }
            f[k]-=0.5*lambdaSquare_[k-m][k-m];
        }

        return f;
    }

    Disposable<Matrix> CapletLiborMarketModelProcess::diffusion(
                                               Time t, const Array&) const {

        Matrix tmp(size(), factors(), 0.0);
        const Size m = nextResetDate(t);

        for (Size k=m; k<size(); ++k) {
            for (Size q=0; q<factors_; ++q) {
                tmp[k][q] = lambda_[k-m][q];
            }
        }
        return tmp;
    }


    Disposable<Array> CapletLiborMarketModelProcess::apply(
                                     const Array& x0, const Array& dx) const {
        Array tmp(size());
        for (Size k=0; k<size(); ++k) {
            tmp[k] = x0[k] * std::exp(dx[k]);
        }
        return tmp;
    }

    Disposable<Array> CapletLiborMarketModelProcess::evolve(
                                             Time t0, const Array& x0,
                                             Time dt, const Array& dw) const {
        // predictor-corrector step to reduce discretization errors
        Array rnd_0     = stdDeviation(t0, x0, dt)*dw;
        Array drift_0   = discretization_->drift(*this, t0, x0, dt);

        return apply(x0, ( drift_0 + discretization_
                ->drift(*this,t0,apply(x0, drift_0 + rnd_0),dt) )*0.5 + rnd_0);
    }

    Size CapletLiborMarketModelProcess::size() const {
        return len_;
    }

    Size CapletLiborMarketModelProcess::factors() const {
        return factors_;
    }

    Disposable<Array> CapletLiborMarketModelProcess::initialValues() const {
        Array tmp = initialValues_;
        return tmp;
    }

    Size CapletLiborMarketModelProcess::nextResetDate(Time t) const {
        return m_.upper_bound(t)->second;
    }

    DiscountFactor CapletLiborMarketModelProcess::discountBond(
        const std::vector<Rate>& rates, Size j) const {
        DiscountFactor d = 1.0;
        for (Size i = 0; i <= j; ++i) {
            d/= 1.0 + rates[i]*accrualPeriod_[i];
        }

        return d;
    }

    Time CapletLiborMarketModelProcess::accrualPeriod(Size i) const {
        return accrualPeriod_[i];
    }

    Volatility CapletLiborMarketModelProcess::lambda(Size i, Size j) const {
        return Volatility(lambda_[i][j]);
    }

    std::vector<Time> CapletLiborMarketModelProcess::fixingTimes() const {
        std::vector<Time> tmp;
        std::map <Time, Size, std::less<Time> >::const_iterator iter;

        for (iter = m_.begin(); iter != m_.end(); ++iter) {
            tmp.push_back(iter->first);
        }

        return tmp;
    }

}

