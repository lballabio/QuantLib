/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/termstructures/yieldcurves/flatforward.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/cashflows/couponpricer.hpp>

namespace QuantLib {

    namespace {

        // utility functions

        template <class T>
        Integer sign(T x) {
            static T zero = T();
            if (x == zero)
                return 0;
            else if (x > zero)
                return 1;
            else
                return -1;
        }

        class irrFinder : public std::unary_function<Rate,Real> {
          public:
            irrFinder(const Leg& cashflows,
                      Real marketPrice,
                      const DayCounter& dayCounter,
                      Compounding compounding,
                      Frequency frequency,
                      Date settlementDate)
            : cashflows_(cashflows), marketPrice_(marketPrice),
              dayCounter_(dayCounter), compounding_(compounding),
              frequency_(frequency), settlementDate_(settlementDate) {}
            Real operator()(Rate guess) const {
                InterestRate y(guess, dayCounter_, compounding_, frequency_);
                Real NPV = CashFlows::npv(cashflows_,y,settlementDate_);
                return marketPrice_ - NPV;
            }
          private:
            const Leg& cashflows_;
            Real marketPrice_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
            Date settlementDate_;
        };

        Real simpleDuration(const Leg& cashflows,
                            const InterestRate& rate,
                            Date settlementDate) {

            Real P = 0.0;
            Real tP = 0.0;

            for (Size i=0; i<cashflows.size(); ++i) {
                if (!cashflows[i]->hasOccurred(settlementDate)) {
                    Time t =
                        rate.dayCounter().yearFraction(settlementDate,
                                                       cashflows[i]->date());
                    Real c = cashflows[i]->amount();
                    DiscountFactor B = rate.discountFactor(t);

                    P += c * B;
                    tP += t * c * B;
                }
            }

            if (P == 0.0)
                // no cashflows
                return 0.0;

            return tP/P;
        }

        Real modifiedDuration(const Leg& cashflows,
                              const InterestRate& rate,
                              Date settlementDate) {

            Real P = 0.0;
            Real dPdy = 0.0;
            Rate y = Rate(rate);
            Integer N = rate.frequency();

            for (Size i=0; i<cashflows.size(); ++i) {
                if (!cashflows[i]->hasOccurred(settlementDate)) {
                    Time t =
                        rate.dayCounter().yearFraction(settlementDate,
                                                       cashflows[i]->date());
                    Real c = cashflows[i]->amount();
                    DiscountFactor B = rate.discountFactor(t);

                    P += c * B;
                    switch (rate.compounding()) {
                      case Simple:
                        dPdy -= c * B*B*t;
                        break;
                      case Compounded:
                        dPdy -= c * B*t/(1+y/N);
                        break;
                      case Continuous:
                        dPdy -= c * B*t;
                        break;
                      case SimpleThenCompounded:
                      default:
                        QL_FAIL("unsupported compounding type");
                    }
                }
            }

            if (P == 0.0)
                // no cashflows
                return 0.0;

            return -dPdy/P;
        }

        Real macaulayDuration(const Leg& cashflows,
                              const InterestRate& rate,
                              Date settlementDate) {

            Rate y = Rate(rate);
            Integer N = rate.frequency();

            QL_REQUIRE(rate.compounding() == Compounded,
                       "compounded rate required");
            QL_REQUIRE(N >= 1, "unsupported frequency");

            return (1+y/N)*modifiedDuration(cashflows,rate,settlementDate);
        }

    }

    Leg::const_iterator CashFlows::lastCashFlow(const Leg& leg,
                                                const Date& refDate) {
        Date d = (refDate==Date() ?
                  Settings::instance().evaluationDate() :
                  refDate);

        if ( ! (*leg.begin())->hasOccurred(d) )
            return leg.end();

        Leg::const_iterator i;
        for (i = leg.begin()+1; i<leg.end(); ++i) {
            if ( ! (*i)->hasOccurred(d) )
                return --i;
        }
        return leg.end();
    }

    Leg::const_iterator CashFlows::nextCashFlow(const Leg& leg,
                                                const Date& refDate) {
        Date d = (refDate==Date() ?
                  Settings::instance().evaluationDate() :
                  refDate);
        Leg::const_iterator i;
        for (i = leg.begin(); i<leg.end(); ++i) {
            // the first coupon paying after d is the one we're after
            if ( ! (*i)->hasOccurred(d) )
                //return boost::dynamic_pointer_cast<Coupon>(leg[i]);
                return i;
        }
        return leg.end();
    }

    Rate CashFlows::previousCouponRate(const Leg& leg,
                                   const Date& refDate) {
        Leg::const_iterator cf = lastCashFlow(leg, refDate);
        if (cf==leg.end()) return 0.0;

        boost::shared_ptr<Coupon> cp=boost::dynamic_pointer_cast<Coupon>(*cf);
        if (cp) return cp->rate();
        else    return 0.0;
    }

    Rate CashFlows::currentCouponRate(const Leg& leg,
                                      const Date& refDate) {
        Leg::const_iterator cf = nextCashFlow(leg, refDate);
        if (cf==leg.end()) return 0.0;

        boost::shared_ptr<Coupon> cp=boost::dynamic_pointer_cast<Coupon>(*cf);
        if (cp) return cp->rate();
        else    return 0.0;
    }

    Date CashFlows::startDate(const Leg& cashflows) {
        Date d = Date::maxDate();
        for (Size i=0; i<cashflows.size(); ++i) {
            boost::shared_ptr<Coupon> c =
                boost::dynamic_pointer_cast<Coupon>(cashflows[i]);
            if (c)
                d = std::min(d, c->accrualStartDate());
        }
        QL_REQUIRE(d != Date::maxDate(),
                   "not enough information available");
        return d;
    }

    Date CashFlows::maturityDate(const Leg& cashflows) {
        Date d = Date::minDate();
        for (Size i=0; i<cashflows.size(); ++i)
            d = std::max(d, cashflows[i]->date());
        QL_REQUIRE(d != Date::minDate(), "no cashflows");
        return d;
    }

    Real CashFlows::npv(const Leg& cashflows,
                        const YieldTermStructure& discountCurve,
                        const Date& settlementDate,
                        const Date& npvDate,
                        Integer exDividendDays) {
        Date d = settlementDate != Date() ?
                 settlementDate :
                 discountCurve.referenceDate();

        Real totalNPV = 0.0;
        for (Size i=0; i<cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(d+exDividendDays))
                totalNPV += cashflows[i]->amount() *
                            discountCurve.discount(cashflows[i]->date());
        }

        if (npvDate==Date())
            return totalNPV;
        else
            return totalNPV/discountCurve.discount(npvDate);
    }

    Real CashFlows::npv(const Leg& cashflows,
                        const InterestRate& irr,
                        Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();
        FlatForward flatRate(settlementDate, irr.rate(), irr.dayCounter(),
                                 irr.compounding(), irr.frequency());
        return npv(cashflows, flatRate, settlementDate, settlementDate);
    }

    Real CashFlows::bps(const Leg& cashflows,
                        const YieldTermStructure& discountCurve,
                        const Date& settlementDate,
                        const Date& npvDate,
                        Integer exDividendDays) {

        Date d = settlementDate;
        if (d==Date())
            d = discountCurve.referenceDate();

        BPSCalculator calc(discountCurve, npvDate);
        for (Size i=0; i<cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(d+exDividendDays))
                cashflows[i]->accept(calc);
        }
        return basisPoint_*calc.result();
    }

    Real CashFlows::bps(const Leg& cashflows,
                        const InterestRate& irr,
                        Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();
        FlatForward flatRate(settlementDate, irr.rate(), irr.dayCounter(),
                                 irr.compounding(), irr.frequency());
        return bps(cashflows, flatRate, settlementDate, settlementDate);
    }

    Rate CashFlows::irr(const Leg& cashflows,
                        Real marketPrice,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        Date settlementDate,
                        Real tolerance,
                        Size maxIterations,
                        Rate guess) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        // depending on the sign of the market price, check that cash
        // flows of the opposite sign have been specified (otherwise
        // IRR is nonsensical.)

        Integer lastSign = sign(-marketPrice),
                signChanges = 0;
        for (Size i = 0; i < cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(settlementDate)) {
                Integer thisSign = sign(cashflows[i]->amount());
                if (lastSign * thisSign < 0) // sign change
                    signChanges++;

                if (thisSign != 0)
                    lastSign = thisSign;
            }
        }
        QL_REQUIRE(signChanges > 0,
                  "the given cash flows cannot result in the given market "
                   "price due to their sign");

        /* The following is commented out due to the lack of a QL_WARN macro
        if (signChanges > 1) {    // Danger of non-unique solution
                                  // Check the aggregate cash flows (Norstrom)
            Real aggregateCashFlow = marketPrice;
            signChanges = 0;
            for (Size i = 0; i < cashflows.size(); ++i) {
                Real nextAggregateCashFlow =
                    aggregateCashFlow + cashflows[i]->amount();

                if (aggregateCashFlow * nextAggregateCashFlow < 0.0)
                    signChanges++;

                aggregateCashFlow = nextAggregateCashFlow;
            }
            if (signChanges > 1)
                QL_WARN( "danger of non-unique solution");
        };
        */

        Brent solver;
        solver.setMaxEvaluations(maxIterations);
        return solver.solve(irrFinder(cashflows, marketPrice, dayCounter,
                                      compounding, frequency, settlementDate),
                            tolerance, guess, guess/10.0);
    }

    Time CashFlows::duration(const Leg& cashflows,
                             const InterestRate& rate,
                             Duration::Type type,
                             Date settlementDate) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        switch (type) {
          case Duration::Simple:
            return simpleDuration(cashflows,rate,settlementDate);
          case Duration::Modified:
            return modifiedDuration(cashflows,rate,settlementDate);
          case Duration::Macaulay:
            return macaulayDuration(cashflows,rate,settlementDate);
          default:
            QL_FAIL("unknown duration type");
        }
    }

    Real CashFlows::convexity(const Leg& cashflows,
                              const InterestRate& rate,
                              Date settlementDate) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        DayCounter dayCounter = rate.dayCounter();

        Real P = 0.0;
        Real d2Pdy2 = 0.0;
        Rate y = Rate(rate);
        Integer N = rate.frequency();

        for (Size i=0; i<cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(settlementDate)) {
                Time t = dayCounter.yearFraction(settlementDate,
                                                 cashflows[i]->date());
                Real c = cashflows[i]->amount();
                DiscountFactor B = rate.discountFactor(t);

                P += c * B;
                switch (rate.compounding()) {
                  case Simple:
                    d2Pdy2 += c * 2.0*B*B*B*t*t;
                    break;
                  case Compounded:
                    d2Pdy2 += c * B*t*(N*t+1)/(N*(1+y/N)*(1+y/N));
                    break;
                  case Continuous:
                    d2Pdy2 += c * B*t*t;
                    break;
                  case SimpleThenCompounded:
                  default:
                    QL_FAIL("unsupported compounding type");
                }
            }
        }

        if (P == 0.0)
            // no cashflows
            return 0.0;

        return d2Pdy2/P;
    }

    Rate CashFlows::atmRate(const Leg& cashFlows,
                            const YieldTermStructure& discountCurve,
                            const Date& settlementDate,
                            const Date& npvDate,
                            Integer exDividendDays,
                            Real npv) {
        Real bps = CashFlows::bps(cashFlows, discountCurve, settlementDate,
                                  npvDate, exDividendDays);
        if (npv==Null<Real>())
            npv = CashFlows::npv(cashFlows, discountCurve, settlementDate,
                                  npvDate, exDividendDays);
        return basisPoint_*npv/bps;
    }
 
//===========================================================================//
//                              BPSCalculator                                //
//===========================================================================// 

    void BPSCalculator::visit(Coupon& c)  {
        result_ += c.accrualPeriod() *
                   c.nominal() *
                   termStructure_.discount(c.date());
    }
    Real BPSCalculator::result() const {
        if (npvDate_==Date())
            return result_;
        else
            return result_/termStructure_.discount(npvDate_);
    }
}
