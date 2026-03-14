/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Allen Kuo
 Copyright (C) 2022 Marcin Rybacki

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

#include <ql/instruments/bondforward.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/cashflow.hpp>

namespace QuantLib {

    BondForward::BondForward(
                    const Date& valueDate,
                    const Date& maturityDate,
                    Position::Type type,
                    Real strike,
                    Natural settlementDays,
                    const DayCounter& dayCounter,
                    const Calendar& calendar,
                    BusinessDayConvention businessDayConvention,
                    const ext::shared_ptr<Bond>& bond,
                    const Handle<YieldTermStructure>& discountCurve,
                    const Handle<YieldTermStructure>& incomeDiscountCurve)
    : Forward(dayCounter, calendar, businessDayConvention, settlementDays,
              ext::shared_ptr<Payoff>(new ForwardTypePayoff(type,strike)),
              valueDate, maturityDate, discountCurve), bond_(bond) {

        incomeDiscountCurve_ = incomeDiscountCurve;
        registerWith(incomeDiscountCurve_);
        registerWith(bond);
    }


    Real BondForward::cleanForwardPrice() const {
        return forwardValue() - bond_->accruedAmount(maturityDate_);
    }


    Real BondForward::forwardPrice() const {
        return forwardValue();
    }


    Real BondForward::spotIncome(
        const Handle<YieldTermStructure>& incomeDiscountCurve) const {

        Real income = 0.0;
        Date settlement = settlementDate();
        Leg cf = bond_->cashflows();

        /*
          the following assumes
          1. cashflows are in ascending order !
          2. considers as income: all coupons paid between settlementDate()
          and contract delivery/maturity date
        */
        for (auto& i : cf) {
            if (!i->hasOccurred(settlement, false)) {
                if (i->hasOccurred(maturityDate_, false)) {
                    income += i->amount() * incomeDiscountCurve->discount(i->date());
                } else {
                    break;
                }
            }
        }

        return income;
    }


    Real BondForward::spotValue() const { 
        return bond_->dirtyPrice();
    }


    void BondForward::performCalculations() const {

        underlyingSpotValue_ = spotValue();
        underlyingIncome_    = spotIncome(incomeDiscountCurve_);

        Forward::performCalculations();
    }

}

