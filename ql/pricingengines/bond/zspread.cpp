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
#include <ql/instruments/bond.hpp>
#include <ql/cashflows/cashflows.hpp>

using boost::shared_ptr;

namespace QuantLib {

    namespace {

        Real dirtyPriceFromZSpread(
                       const Bond& bond,
                       const shared_ptr<YieldTermStructure>& discountCurve,
                       Spread zSpread,
                       const DayCounter& dc,
                       Compounding comp,
                       Frequency freq,
                       Date settlementDate) {
            if (settlementDate == Date())
                settlementDate = bond.settlementDate();

            Natural exDividendDays = 0;

            Real NPV = CashFlows::npv(bond.cashflows(),
                                      discountCurve,
                                      zSpread, dc, comp, freq,
                                      settlementDate,
                                      settlementDate,
                                      exDividendDays);

            return NPV * 100.0 / bond.notional(settlementDate);
        }

    } // anonymous namespace ends here

    Real cleanPriceFromZSpread(
                   const Bond& bond,
                   const shared_ptr<YieldTermStructure>& discountCurve,
                   Spread zSpread,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Date settlementDate) {
        Real dirtyPrice = dirtyPriceFromZSpread(bond,
                                                discountCurve,
                                                zSpread,
                                                dc, comp, freq,
                                                settlementDate);
        return dirtyPrice - bond.accruedAmount(settlementDate);
    }

    Spread zSpreadFromCleanPrice(
                   const Bond& bond,
                   const shared_ptr<YieldTermStructure>& discountCurve,
                   Real cleanPrice,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Date settlement,
                   Real accuracy,
                   Size maxEvaluations) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);

        dirtyPrice /= 100.0 / bond.notional(settlement);

        Natural exDividendDays = 0;

        return CashFlows::zSpread(bond.cashflows(), dirtyPrice,
                                  discountCurve,
                                  dc, comp, freq,
                                  settlement, settlement, exDividendDays,
                                  accuracy, maxEvaluations);
    }

}
