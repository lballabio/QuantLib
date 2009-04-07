/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2009 StatPro Italia srl
 Copyright (C) 2009 Nathan Abbott

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

#include <ql/pricingengines/bond/zspread.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/solvers1d/brent.hpp>

using boost::shared_ptr;

namespace QuantLib {

    namespace {

        Real dirtyPriceF(const Bond& bond,
                         const YieldTermStructure& discCurve,
                         Date settlementDate) {
            Real NPV = CashFlows::npv(bond.cashflows(),
                                      discCurve,
                                      settlementDate,
                                      settlementDate);

            return NPV * 100.0 / bond.notional(settlementDate);
        }

        class ZSpreadFinder {
          public:
            ZSpreadFinder(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Real dirtyPrice,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   const Date& settlementDate)
            : bond_(bond), zSpread_(new SimpleQuote(0.0)),
              curve_(Handle<YieldTermStructure>(discountCurve),
                     Handle<Quote>(zSpread_), comp, freq, dc),
              dirtyPrice_(dirtyPrice),
              settlementDate_(settlementDate) {}
            Real operator()(Real zSpread) const {
                zSpread_->setValue(zSpread);
                return dirtyPrice_ - dirtyPriceF(bond_, curve_, settlementDate_);
            }
          private:
            const Bond& bond_;
            shared_ptr<SimpleQuote> zSpread_;
            ZeroSpreadedTermStructure curve_;
            Real dirtyPrice_;
            Date settlementDate_;
        };

    }

    Spread zSpreadFromCleanPrice(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Real cleanPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlement,
                   Real accuracy,
                   Size maxEvaluations) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(frequency != NoFrequency && frequency != Once,
                   "invalid frequency:" << frequency);

        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);

        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        ZSpreadFinder objective(bond,
                                discountCurve,
                                dirtyPrice,
                                dayCounter, compounding, frequency,
                                settlement);
        return solver.solve(objective, accuracy, 0.0, 0.001);
    }

    Real cleanPriceFromZSpread(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Spread zSpread,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = bond.settlementDate();

        QL_REQUIRE(freq != NoFrequency && freq != Once,
                   "invalid frequency: " << freq);

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(shared_ptr<Quote>(new
            SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                comp,
                                                freq,
                                                dc);

        Real dirtyPrice = dirtyPriceF(bond,
                                      spreadedCurve,
                                      settlementDate);
        return dirtyPrice - bond.accruedAmount(settlementDate);
    }

}
