
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/CashFlows/basispointsensitivity.hpp>

namespace QuantLib {

    void BPSCalculator::visit(Coupon& c) {
        // add BPS of the coupon
        result_ += c.accrualPeriod() *
                   c.nominal() *
            termStructure_->discount(c.date());
    }

    void BPSCalculator::visit(CashFlow&) {}

    Real BasisPointSensitivity(
                         const std::vector<boost::shared_ptr<CashFlow> >& leg,
                         const RelinkableHandle<TermStructure>& ts) {
        Date settlement = ts->referenceDate();
        BPSCalculator calc(ts);
        for (Size i=0; i<leg.size(); i++)
            #if QL_TODAYS_PAYMENTS
            if (leg[i]->date() >= settlement)
            #else
            if (leg[i]->date() > settlement)
            #endif
                leg[i]->accept(calc);
        return calc.result();
    }


    Real BPSBasketCalculator::sensfactor(const Date& date) const {
        Time t = termStructure_->dayCounter().yearFraction(
                                         termStructure_->referenceDate(),date);
        // Based on 1st derivative of zero coupon rate
        Rate r = termStructure_->zeroCoupon(date,basis_);
        return -QL_POW(1.0+r/basis_,-1.0-t*basis_)*t;
    }

    void BPSBasketCalculator::visit(Coupon& c) {
        Date today = termStructure_->todaysDate(),
             accrualStart = c.accrualStartDate(),
             accrualEnd = c.accrualEndDate(),
             payment = c.date();
        if (accrualStart > today) {
            Real bps = sensfactor(accrualStart);
            result_[accrualStart] += bps*c.nominal()/10000.0;
        }
        if (accrualEnd >= today) {
            Real bps = -sensfactor(accrualEnd);
            DiscountFactor dfs = 1.0, dfe;
            if (accrualStart > today)
                dfs = termStructure_->discount(accrualStart);
            dfe = termStructure_->discount(accrualEnd);
            result_[accrualEnd] += bps*c.nominal()*(dfs/dfe)/10000.0;
        }
        if (payment > today) {
            Real bps = sensfactor(payment);
            result_[payment] += bps*c.amount()/10000.0;
        }
    }

    void BPSBasketCalculator::visit(FixedRateCoupon& c) {
        Date today = termStructure_->todaysDate(),
             payment = c.date();
        if (payment > today) {
            Real bps = sensfactor(payment);
            result_[payment] -= bps*c.amount()/10000.0;
        }
    }

    void BPSBasketCalculator::visit(CashFlow&) {
        // fall-back for all non-coupons; do nothing
    }

    TimeBasket BasisPointSensitivityBasket(
                         const std::vector<boost::shared_ptr<CashFlow> >& leg,
                         const RelinkableHandle<TermStructure>& ts,
                         Integer basis) {
        Date settlement = ts->referenceDate();
        BPSBasketCalculator calc(ts,basis);
        for (Size i=0; i<leg.size(); i++)
            #if QL_TODAYS_PAYMENTS
            if (leg[i]->date() >= settlement)
            #else
            if (leg[i]->date() > settlement)
            #endif
                leg[i]->accept(calc);
        return calc.result();
    }

}

