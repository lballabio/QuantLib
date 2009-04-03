/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2009 StatPro Italia srl

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

namespace QuantLib {

    Real cleanPriceFromZSpread(
                   const Bond& bond,
                   const boost::shared_ptr<YieldTermStructure>& discountCurve,
                   Spread zSpread,
                   const DayCounter& dc,
                   Compounding comp,
                   Frequency freq,
                   Date settlementDate) {
        Real dirtyPrice = dirtyPriceFromZSpread(bond, discountCurve,
                                                zSpread, dc, comp, freq,
                                                settlementDate);
        return dirtyPrice - bond.accruedAmount(settlementDate);
    }

    Real dirtyPriceFromZSpread(
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
                   "invalid frequency:" << freq);

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(
                          boost::shared_ptr<Quote>(new SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                comp, freq, dc);

        Real NPV = CashFlows::npv(bond.cashflows(),
                                  spreadedCurve,
                                  settlementDate,
                                  settlementDate);

        return NPV * 100.0 / bond.notional(settlementDate);
    }

}

