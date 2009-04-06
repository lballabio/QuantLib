/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2007, 2008 Ferdinando Ametrano
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

namespace QuantLib {

    Real cleanPriceFromZSpread(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Spread zSpread,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate) {
        Real dirtyPrice = dirtyPriceFromZSpread(bond, discountCurve,
                                                zSpread, dayCounter,
                                                compounding, frequency,
                                                settlementDate);
        return dirtyPrice - bond.accruedAmount(settlementDate);
    }

    Real dirtyPriceFromZSpread(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Spread zSpread,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate) {

        if (settlementDate == Date())
            settlementDate = bond.settlementDate();

        QL_REQUIRE(frequency != NoFrequency && frequency != Once,
                   "invalid frequency:" << frequency);

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(
                          boost::shared_ptr<Quote>(new SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                compounding,
                                                frequency,
                                                dayCounter);

        Real NPV = CashFlows::npv(bond.cashflows(),
                                  spreadedCurve,
                                  settlementDate,
                                  settlementDate);

        return NPV * 100.0 / bond.notional(settlementDate);
    }


    namespace {

        class ZSpreadFinder {
          public:
            ZSpreadFinder(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Real dirtyPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   const Date& settlementDate)
            : bond_(bond), discountCurve_(discountCurve),
              dirtyPrice_(dirtyPrice), dayCounter_(dayCounter),
              compounding_(compounding), frequency_(frequency),
              settlementDate_(settlementDate) {}
            Real operator()(Real zSpread) const {
                return dirtyPrice_ - dirtyPriceFromZSpread(bond_,
                                                           discountCurve_,
                                                           zSpread,
                                                           dayCounter_,
                                                           compounding_,
                                                           frequency_,
                                                           settlementDate_);
            }
          private:
            const Bond& bond_;
            boost::shared_ptr<YieldTermStructure> discountCurve_;
            Real dirtyPrice_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
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
                   Date settlementDate,
                   Real accuracy,
                   Size maxEvaluations) {

        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlementDate);
        return zSpreadFromDirtyPrice(bond, discountCurve, dirtyPrice,
                                     dayCounter, compounding,
                                     frequency, settlementDate,
                                     accuracy, maxEvaluations);
    }


    Spread zSpreadFromDirtyPrice(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Real dirtyPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   Date settlementDate,
                   Real accuracy,
                   Size maxEvaluations) {
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);

        ZSpreadFinder objective(bond, discountCurve, dirtyPrice,
                                dayCounter, compounding,
                                frequency, settlementDate);
        return solver.solve(objective, accuracy, 0.0, 0.001);
    }

}

