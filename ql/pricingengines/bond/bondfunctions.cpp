/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Nathan Abbott
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
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

#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/instruments/bond.hpp>
#include <ql/cashflows/cashflows.hpp>

using boost::shared_ptr;

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
        return !CashFlows::isExpired(bond.cashflows(), settlement);
    }

    Leg::const_iterator BondFunctions::previousCashFlow(const Bond& bond,
                                                        Date refDate) {
        return CashFlows::previousCashFlow(bond.cashflows(), refDate);
    }

    Leg::const_iterator BondFunctions::nextCashFlow(const Bond& bond,
                                                    Date refDate) {
        return CashFlows::nextCashFlow(bond.cashflows(), refDate);
    }

    Date BondFunctions::previousCashFlowDate(const Bond& bond,
                                             Date refDate) {
        return CashFlows::previousCashFlowDate(bond.cashflows(), refDate);
    }

    Date BondFunctions::nextCashFlowDate(const Bond& bond,
                                         Date refDate) {
        return CashFlows::nextCashFlowDate(bond.cashflows(), refDate);
    }

    Real BondFunctions::previousCashFlowAmount(const Bond& bond,
                                               Date refDate) {
        return CashFlows::previousCashFlowAmount(bond.cashflows(), refDate);
    }

    Real BondFunctions::nextCashFlowAmount(const Bond& bond,
                                           Date refDate) {
        return CashFlows::nextCashFlowAmount(bond.cashflows(), refDate);
    }

    Rate BondFunctions::previousCouponRate(const Bond& bond,
                                           Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::previousCouponRate(bond.cashflows(), settlement);
    }

    Rate BondFunctions::nextCouponRate(const Bond& bond,
                                       Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::nextCouponRate(bond.cashflows(), settlement);
    }

    Real BondFunctions::accruedAmount(const Bond& bond,
                                      Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::accruedAmount(bond.cashflows(), settlement) *
            100.0 / bond.notional(settlement);
    }

    Real BondFunctions::cleanPrice(const Bond& bond,
                                   const YieldTermStructure& discountCurve,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        Real dirtyPrice = CashFlows::npv(bond.cashflows(), discountCurve,
                              settlement, settlement) *
            100.0 / bond.notional(settlement);
        return dirtyPrice - bond.accruedAmount(settlement);
    }

    Real BondFunctions::bps(const Bond& bond,
                            const YieldTermStructure& discountCurve,
                            Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::bps(bond.cashflows(), discountCurve,
                              settlement, settlement);
    }

    Rate BondFunctions::atmRate(const Bond& bond,
                                const YieldTermStructure& discountCurve,
                                Date settlement,
                                Real cleanPrice) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        Real dirtyPrice = cleanPrice==Null<Real>() ? Null<Real>() :
                          cleanPrice + bond.accruedAmount(settlement);

        return CashFlows::atmRate(bond.cashflows(), discountCurve,
                                  settlement, settlement,
                                  0,
                                  dirtyPrice);
    }

    Real BondFunctions::cleanPrice(const Bond& bond,
                                   const InterestRate& yield,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        Real dirtyPrice = CashFlows::npv(bond.cashflows(), yield, settlement) *
            100.0 / bond.notional(settlement);
        return dirtyPrice - bond.accruedAmount(settlement);
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

    Real BondFunctions::bps(const Bond& bond,
                            const InterestRate& yield,
                            Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::bps(bond.cashflows(), yield, settlement);
    }

    Real BondFunctions::bps(const Bond& bond,
                            Rate yield,
                            const DayCounter& dayCounter,
                            Compounding compounding,
                            Frequency frequency,
                            Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return bps(bond, yield, settlement);
    }

    Rate BondFunctions::yield(const Bond& bond,
                              Real cleanPrice,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              Date settlement,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);
        dirtyPrice /= 100.0 / bond.notional(settlement);

        return CashFlows::yield(bond.cashflows(), dirtyPrice,
                              dayCounter, compounding, frequency,
                              settlement,
                              0,
                              accuracy, maxIterations, guess);
    }

    Time BondFunctions::duration(const Bond& bond,
                                 const InterestRate& yield,
                                 Duration::Type type,
                                 Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::duration(bond.cashflows(), yield,
                                   type, settlement);
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
        return CashFlows::convexity(bond.cashflows(), yield,
                                    settlement);
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
        return CashFlows::basisPointValue(bond.cashflows(), yield,
                                          settlement);
    }

    Real BondFunctions::basisPointValue(const Bond& bond,
                              Rate yield,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                                        Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return CashFlows::basisPointValue(bond.cashflows(), y,
                                          settlement);
    }

    Real BondFunctions::yieldValueBasisPoint(const Bond& bond,
                                             const InterestRate& yield,
                                             Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        return CashFlows::yieldValueBasisPoint(bond.cashflows(), yield,
                                               settlement);
    }

    Real BondFunctions::yieldValueBasisPoint(const Bond& bond,
                                             Rate yield,
                                             const DayCounter& dayCounter,
                                             Compounding compounding,
                                             Frequency frequency,
                                             Date settlement) {
        InterestRate y(yield, dayCounter, compounding, frequency);
        return CashFlows::yieldValueBasisPoint(bond.cashflows(), y,
                                               settlement);
    }

    Real BondFunctions::cleanPrice(const Bond& bond,
                                   const shared_ptr<YieldTermStructure>& d,
                                   Spread zSpread,
                                   const DayCounter& dc,
                                   Compounding comp,
                                   Frequency freq,
                                   Date settlement) {
        if (settlement == Date())
            settlement = bond.settlementDate();
        Real dirtyPrice = CashFlows::npv(bond.cashflows(), d,
                                         zSpread, dc, comp, freq,
                                         settlement, settlement) *
            100.0 / bond.notional(settlement);
        return dirtyPrice - bond.accruedAmount(settlement);
    }

    Spread BondFunctions::zSpread(const Bond& bond,
                                  Real cleanPrice,
                                  const shared_ptr<YieldTermStructure>& d,
                                  const DayCounter& dayCounter,
                                  Compounding compounding,
                                  Frequency frequency,
                                  Date settlement,
                                  Real accuracy,
                                  Size maxIterations,
                                  Rate guess) {
        if (settlement == Date())
            settlement = bond.settlementDate();

        Real dirtyPrice = cleanPrice + bond.accruedAmount(settlement);
        dirtyPrice /= 100.0 / bond.notional(settlement);

        return CashFlows::zSpread(bond.cashflows(), dirtyPrice,
                                  d,
                                  dayCounter, compounding, frequency,
                                  settlement, settlement,
                                  0,
                                  accuracy, maxIterations, guess);
    }

}
