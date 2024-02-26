/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Nathan Abbott
 Copyright (C) 2007, 2008, 2009, 2010 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2008 Simon Ibbotson
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007, 2008, 2009 StatPro Italia srl
 Copyright (C) 2004 Jeff Yu

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

#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>

namespace QuantLib {

    Date BondFunctions::startDate(const Bond& bond) {
        return CashFlows::startDate(bond.cashflows());
    }

    Date BondFunctions::maturityDate(const Bond& bond) {
        return CashFlows::maturityDate(bond.cashflows());
    }

    bool BondFunctions::isTradable(const Bond& bond,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return bond.notional(settlement)!=0.0;
    }

    Leg::const_reverse_iterator
    BondFunctions::previousCashFlow(const Bond& bond,
                                    Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::previousCashFlow(bond.cashflows(),
                                           false, settlement);
    }

    Leg::const_iterator BondFunctions::nextCashFlow(const Bond& bond,
                                                    Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::nextCashFlow(bond.cashflows(),
                                       false, settlement);
    }

    Date BondFunctions::previousCashFlowDate(const Bond& bond,
                                             Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::previousCashFlowDate(bond.cashflows(),
                                               false, settlement);
    }

    Date BondFunctions::nextCashFlowDate(const Bond& bond,
                                         Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::nextCashFlowDate(bond.cashflows(),
                                           false, settlement);
    }

    Real BondFunctions::previousCashFlowAmount(const Bond& bond,
                                               Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::previousCashFlowAmount(bond.cashflows(),
                                                 false, settlement);
    }

    Real BondFunctions::nextCashFlowAmount(const Bond& bond,
                                           Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::nextCashFlowAmount(bond.cashflows(),
                                             false, settlement);
    }

    Rate BondFunctions::previousCouponRate(const Bond& bond,
                                           Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::previousCouponRate(bond.cashflows(),
                                             false, settlement);
    }

    Rate BondFunctions::nextCouponRate(const Bond& bond,
                                       Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        return CashFlows::nextCouponRate(bond.cashflows(),
                                         false, settlement);
    }

    Date BondFunctions::accrualStartDate(const Bond& bond,
                                         Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::accrualStartDate(bond.cashflows(),
                                           false, settlement);
    }

    Date BondFunctions::accrualEndDate(const Bond& bond,
                                       Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::accrualEndDate(bond.cashflows(),
                                         false, settlement);
    }

    Date BondFunctions::referencePeriodStart(const Bond& bond,
                                             Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::referencePeriodStart(bond.cashflows(),
                                               false, settlement);
    }

    Date BondFunctions::referencePeriodEnd(const Bond& bond,
                                           Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::referencePeriodEnd(bond.cashflows(),
                                             false, settlement);
    }

    Time BondFunctions::accrualPeriod(const Bond& bond,
                                      Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::accrualPeriod(bond.cashflows(),
                                        false, settlement);
    }

    Date::serial_type BondFunctions::accrualDays(const Bond& bond,
                                                 Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::accrualDays(bond.cashflows(),
                                      false, settlement);
    }

    Time BondFunctions::accruedPeriod(const Bond& bond,
                                      Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::accruedPeriod(bond.cashflows(),
                                        false, settlement);
    }

    Date::serial_type BondFunctions::accruedDays(const Bond& bond,
                                                 Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::accruedDays(bond.cashflows(),
                                      false, settlement);
    }

    Real BondFunctions::accruedAmount(const Bond& bond,
                                      Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        if (!BondFunctions::isTradable(bond, settlement))
            return 0.0;

        return CashFlows::accruedAmount(bond.cashflows(),
                                        false, settlement) *
            100.0 / bond.notional(settlement);
    }



    Real BondFunctions::cleanPrice(const Bond& bond,
                                   const YieldTermStructure& discountCurve,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " settlement date (maturity being " <<
                   bond.maturityDate() << ")");

        Real dirtyPrice = CashFlows::npv(bond.cashflows(), discountCurve,
                                         false, settlement) *
            100.0 / bond.notional(settlement);
        return dirtyPrice - bond.accruedAmount(settlement);
    }

    Real BondFunctions::bps(const Bond& bond,
                            const YieldTermStructure& discountCurve,
                            Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::bps(bond.cashflows(), discountCurve,
                              false, settlement) *
            100.0 / bond.notional(settlement);
    }

    Rate BondFunctions::atmRate(const Bond& bond,
                                const YieldTermStructure& discountCurve,
                                Date settlement,
                                Real cleanPrice) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        Real dirtyPrice = cleanPrice==Null<Real>() ? Null<Real>() :
                          cleanPrice + bond.accruedAmount(settlement);
        Real currentNotional = bond.notional(settlement);
        Real npv = dirtyPrice==Null<Real>() ? Null<Real>() :
                                              dirtyPrice/100.0 * currentNotional;

        return CashFlows::atmRate(bond.cashflows(), discountCurve,
                                  false, settlement, settlement,
                                  npv);
    }

    Real BondFunctions::cleanPrice(const Bond& bond,
                                   const InterestRate& yield,
                                   Date settlement) {
        return dirtyPrice(bond, yield, settlement) - bond.accruedAmount(settlement);
    }

    Real BondFunctions::cleanPrice(const Bond& bond,
                                   Rate yield,
                                   const DayCounter& dayCounter,
                                   Compounding compounding,
                                   Frequency frequency,
                                   Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return cleanPrice(bond, y, settlement);
    }

    Real BondFunctions::dirtyPrice(const Bond& bond,
                                   const InterestRate& yield,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        Real dirtyPrice = CashFlows::npv(bond.cashflows(), yield,
                                         false, settlement) *
            100.0 / bond.notional(settlement);
        return dirtyPrice;
    }

    Real BondFunctions::dirtyPrice(const Bond& bond,
                                   Rate yield,
                                   const DayCounter& dayCounter,
                                   Compounding compounding,
                                   Frequency frequency,
                                   Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return dirtyPrice(bond, y, settlement);
    }

    Real BondFunctions::bps(const Bond& bond,
                            const InterestRate& yield,
                            Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::bps(bond.cashflows(), yield,
                              false, settlement) *
            100.0 / bond.notional(settlement);
    }

    Real BondFunctions::bps(const Bond& bond,
                            Rate yield,
                            const DayCounter& dayCounter,
                            Compounding compounding,
                            Frequency frequency,
                            Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return bps(bond, y, settlement);
    }

    Rate BondFunctions::yield(const Bond& bond,
                              Real price,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              Date settlement,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess,
                              Bond::Price::Type priceType) {
        return yield(bond, Bond::Price(price, priceType), dayCounter, compounding, frequency,
                     settlement, accuracy, maxIterations, guess);
    }
    Rate BondFunctions::yield(const Bond& bond,
                              Bond::Price price,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              Date settlement,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess) {
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        return yield<NewtonSafe>(solver, bond, price, dayCounter,
                                 compounding, frequency, settlement,
                                 accuracy, guess);
    }

    Time BondFunctions::duration(const Bond& bond,
                                 const InterestRate& yield,
                                 Duration::Type type,
                                 Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::duration(bond.cashflows(), yield,
                                   type,
                                   false, settlement);
    }

    Time BondFunctions::duration(const Bond& bond,
                                 Rate yield,
                                 const DayCounter& dayCounter,
                                 Compounding compounding,
                                 Frequency frequency,
                                 Duration::Type type,
                                 Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return duration(bond, y, type, settlement);
    }

    Real BondFunctions::convexity(const Bond& bond,
                                  const InterestRate& yield,
                                  Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::convexity(bond.cashflows(), yield,
                                    false, settlement);
    }

    Real BondFunctions::convexity(const Bond& bond,
                                  Rate yield,
                                  const DayCounter& dayCounter,
                                  Compounding compounding,
                                  Frequency frequency,
                                  Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return convexity(bond, y, settlement);
    }

    Real BondFunctions::basisPointValue(const Bond& bond,
                                        const InterestRate& yield,
                                        Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::basisPointValue(bond.cashflows(), yield,
                                          false, settlement);
    }

    Real BondFunctions::basisPointValue(const Bond& bond,
                              Rate yield,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                                        Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return basisPointValue(bond, y, settlement);
    }

    Real BondFunctions::yieldValueBasisPoint(const Bond& bond,
                                             const InterestRate& yield,
                                             Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        return CashFlows::yieldValueBasisPoint(bond.cashflows(), yield,
                                               false, settlement);
    }

    Real BondFunctions::yieldValueBasisPoint(const Bond& bond,
                                             Rate yield,
                                             const DayCounter& dayCounter,
                                             Compounding compounding,
                                             Frequency frequency,
                                             Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return yieldValueBasisPoint(bond, y, settlement);
    }

    Real BondFunctions::cleanPrice(const Bond& bond,
                                   const ext::shared_ptr<YieldTermStructure>& d,
                                   Spread zSpread,
                                   const DayCounter& dc,
                                   Compounding comp,
                                   Frequency freq,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        Real dirtyPrice = CashFlows::npv(bond.cashflows(), d,
                                         zSpread, dc, comp, freq,
                                         false, settlement) *
            100.0 / bond.notional(settlement);
        return dirtyPrice - bond.accruedAmount(settlement);
    }

    Spread BondFunctions::zSpread(const Bond& bond,
                                  Real cleanPrice,
                                  const ext::shared_ptr<YieldTermStructure>& d,
                                  const DayCounter& dayCounter,
                                  Compounding compounding,
                                  Frequency frequency,
                                  Date settlement,
                                  Real accuracy,
                                  Size maxIterations,
                                  Rate guess) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        QL_REQUIRE(BondFunctions::isTradable(bond, settlement),
                   "non tradable at " << settlement <<
                   " (maturity being " << bond.maturityDate() << ")");

        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);
        dirtyPrice /= 100.0 / bond.notional(settlement);

        return CashFlows::zSpread(bond.cashflows(),
                                  d,
                                  dirtyPrice,
                                  dayCounter, compounding, frequency,
                                  false, settlement, settlement,
                                  accuracy, maxIterations, guess);
    }

}
