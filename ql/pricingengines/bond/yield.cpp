/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2008 Simon Ibbotson

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

#include <ql/pricingengines/bond/yield.hpp>
#include <ql/instruments/bond.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/math/solvers1d/brent.hpp>

using boost::shared_ptr;

namespace QuantLib {

    namespace {

        Real dirtyPriceFromYield(const Bond& bond,
                                 Rate yield,
                                 const DayCounter& dayCounter,
                                 Compounding compounding,
                                 Frequency frequency,
                                 const Date& settlement) {

            InterestRate y(yield, dayCounter, compounding, frequency);

            Real price = 0.0;
            DiscountFactor discount = 1.0;
            Date lastDate = Date();

            const Leg& cashflows = bond.cashflows();
            for (Size i=0; i<cashflows.size(); ++i) {
                if (cashflows[i]->hasOccurred(settlement))
                    continue;

                Date couponDate = cashflows[i]->date();
                Real amount = cashflows[i]->amount();
                if (lastDate == Date()) {
                    // first not-expired coupon
                    if (i > 0) {
                        lastDate = cashflows[i-1]->date();
                    } else {
                        shared_ptr<Coupon> coupon =
                            boost::dynamic_pointer_cast<Coupon>(cashflows[i]);
                        if (coupon)
                            lastDate = coupon->accrualStartDate();
                        else
                            lastDate = couponDate - 1*Years;
                    }
                    discount *= y.discountFactor(settlement, couponDate,
                                                 lastDate, couponDate);
                } else  {
                    discount *= y.discountFactor(lastDate, couponDate);
                }
                lastDate = couponDate;

                price += amount * discount;
            }

            return price * 100.0 / bond.notional(settlement);
        }


        class YieldFinder {
          public:
            YieldFinder(const Bond& bond,
                        Real dirtyPrice,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        const Date& settlement)
            : bond_(bond),
              dirtyPrice_(dirtyPrice),compounding_(compounding),
              dayCounter_(dayCounter), frequency_(frequency),
              settlement_(settlement) {}
            Real operator()(Real yield) const {
                return dirtyPrice_ - dirtyPriceFromYield(bond_,
                                                         yield,
                                                         dayCounter_,
                                                         compounding_,
                                                         frequency_,
                                                         settlement_);
            }
          private:
            const Bond& bond_;
            Real dirtyPrice_;
            Compounding compounding_;
            DayCounter dayCounter_;
            Frequency frequency_;
            Date settlement_;
        };

    } // anonymous namespace ends here

    Spread yieldFromCleanPrice(
                   const Bond& bond,
                   Real cleanPrice,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Date settlement,
                   Real accuracy,
                   Size maxEvaluations) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(freq != NoFrequency && freq != Once,
                   "invalid frequency:" << freq);

        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);

        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        YieldFinder objective(bond,
                              dirtyPrice,
                              dc, comp, freq,
                              settlement);
        Real guess = 0.02;
        Rate yieldMin = 0.0;
        Rate yieldMax = 1.0;
        return solver.solve(objective, accuracy, guess, yieldMin, yieldMax);
    }

    Real cleanPriceFromYield(const Bond& bond,
                             Rate yield,
                             const DayCounter& dc,
                             Compounding comp,
                             Frequency freq,
                             Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = bond.settlementDate();

        QL_REQUIRE(freq != NoFrequency && freq != Once,
                   "invalid frequency: " << freq);

        Real dirtyPrice = dirtyPriceFromYield(bond,
                                              yield, dc, comp, freq,
                                              settlementDate);
        return dirtyPrice - bond.accruedAmount(settlementDate);
    }

}
