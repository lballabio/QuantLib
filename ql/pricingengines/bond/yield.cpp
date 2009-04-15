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
                                 Date settlement) {

            InterestRate y(yield, dayCounter, compounding, frequency);

            if (settlement == Date())
                settlement = bond.settlementDate();

            Real dirtyPrice = CashFlows::npv(bond.cashflows(), y, settlement);
            return dirtyPrice * 100.0 / bond.notional(settlement);
        }

    } // anonymous namespace ends here

    Real cleanPriceFromYield(const Bond& bond,
                             Rate yield,
                             const DayCounter& dc,
                             Compounding comp,
                             Frequency freq,
                             Date settlementDate) {
        Real dirtyPrice = dirtyPriceFromYield(bond,
                                              yield, dc, comp, freq,
                                              settlementDate);
        return dirtyPrice - bond.accruedAmount(settlementDate);
    }

    Rate yieldFromCleanPrice(
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
        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);
        dirtyPrice /= 100.0 / bond.notional(settlement);

        Natural exDividendDays = 0;

        return CashFlows::irr(bond.cashflows(), dirtyPrice,
                              dc, comp, freq,
                              settlement, exDividendDays,
                              accuracy, maxEvaluations);
    }

}
