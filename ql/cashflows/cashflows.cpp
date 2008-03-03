/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
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
#include <ql/termstructures/yield/flatforward.hpp>
//#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/cashflows/couponpricer.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    namespace {

        // utility functions

        Rate couponRate(const Leg& leg,
                        Leg::const_iterator cf) {
            if (cf==leg.end()) return 0.0;

            Date paymentDate = (*cf)->date();
            bool firstCouponFound = false;
            Real nominal = Null<Real>();
            Time accrualPeriod = Null<Time>();
            DayCounter dc;
            Rate result = 0.0;
            for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
                shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
                if (cp) {
                    if (firstCouponFound) {
                        QL_REQUIRE(nominal       == cp->nominal() &&
                                   accrualPeriod == cp->accrualPeriod() &&
                                   dc            == cp->dayCounter(),
                                   "cannot aggregate two different coupons on "
                                   << paymentDate);
                    } else {
                        firstCouponFound = true;
                        nominal = cp->nominal();
                        accrualPeriod = cp->accrualPeriod();
                        dc = cp->dayCounter();
                    }
                    result += cp->rate();
                }
            }
            QL_ENSURE(firstCouponFound,
                      "next cashflow (" << paymentDate << ") is not a coupon");
            return result;
        }

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


        Real simpleDuration(const Leg& cashflows,
                            const InterestRate& y,
                            const Date& settlementDate) {
            Real P = 0.0;
            Real dPdy = 0.0;
            const DayCounter& dc = y.dayCounter();
            for (Size i=0; i<cashflows.size(); ++i) {
                if (!cashflows[i]->hasOccurred(settlementDate)) {
                    Time t = dc.yearFraction(settlementDate,
                                             cashflows[i]->date());
                    Real c = cashflows[i]->amount();
                    DiscountFactor B = y.discountFactor(t);
                    P += c * B;
                    dPdy += t * c * B;
                }
            }
            if (P == 0.0) // no cashflows
                return 0.0;
            return dPdy/P;
        }

        Real modifiedDuration(const Leg& cashflows,
                              const InterestRate& y,
                              const Date& settlementDate) {
            Real P = 0.0;
            Real dPdy = 0.0;
            Rate r = y.rate();
            Natural N = y.frequency();
            const DayCounter& dc = y.dayCounter();
            for (Size i=0; i<cashflows.size(); ++i) {
                if (!cashflows[i]->hasOccurred(settlementDate)) {
                    Time t = dc.yearFraction(settlementDate,
                                             cashflows[i]->date());
                    Real c = cashflows[i]->amount();
                    DiscountFactor B = y.discountFactor(t);

                    P += c * B;
                    switch (y.compounding()) {
                      case Simple:
                        dPdy -= c * B*B * t;
                        break;
                      case Compounded:
                        dPdy -= c * t * B/(1+r/N);
                        break;
                      case Continuous:
                        dPdy -= c * B * t;
                        break;
                      case SimpleThenCompounded:
                        if (t<=1.0/N)
                            dPdy -= c * B*B * t;
                        else
                            dPdy -= c * t * B/(1+r/N);
                        break;
                      default:
                        QL_FAIL("unknown compounding convention (" <<
                                Integer(y.compounding()) << ")");
                    }
                }
            }

            if (P == 0.0) // no cashflows
                return 0.0;
            return -dPdy/P; // reverse derivative sign
        }

        Real macaulayDuration(const Leg& cashflows,
                              const InterestRate& y,
                              const Date& settlementDate) {

            QL_REQUIRE(y.compounding() == Compounded,
                       "compounded rate required");

            return (1.0+y.rate()/y.frequency()) *
                modifiedDuration(cashflows, y, settlementDate);
        }

        class IrrFinder : public std::unary_function<Rate, Real> {
          public:
            IrrFinder(const Leg& cashflows,
                      Real marketPrice,
                      const DayCounter& dayCounter,
                      Compounding compounding,
                      Frequency frequency,
                      Date settlementDate)
            : cashflows_(cashflows), marketPrice_(marketPrice),
              dayCounter_(dayCounter), compounding_(compounding),
              frequency_(frequency), settlementDate_(settlementDate) {}
            Real operator()(Rate x) const {
                InterestRate y(x, dayCounter_, compounding_, frequency_);
                Real NPV = CashFlows::npv(cashflows_, y, settlementDate_);
                return marketPrice_ - NPV;
            }
            Real derivative(Rate x) const {
                InterestRate y(x, dayCounter_, compounding_, frequency_);
                return modifiedDuration(cashflows_, y, settlementDate_);
            }
          private:
            const Leg& cashflows_;
            Real marketPrice_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
            Date settlementDate_;
        };

        class BPSCalculator : public AcyclicVisitor,
                              public Visitor<CashFlow>,
                              public Visitor<Coupon> {
          public:
            BPSCalculator(const YieldTermStructure& termStructure,
                          const Date& npvDate)
            : termStructure_(termStructure), npvDate_(npvDate), result_(0.0) {}
            void visit(Coupon& c) {
                result_ += c.accrualPeriod() *
                           c.nominal() *
                           termStructure_.discount(c.date());
            }
            void visit(CashFlow&) {}
            Real result() const {
                if (npvDate_==Date())
                    return result_;
                else
                    return result_/termStructure_.discount(npvDate_);
            }
          private:
            const YieldTermStructure& termStructure_;
            Date npvDate_;
            Real result_;
        };

        const Spread basisPoint_ = 1.0e-4;
    }

    Leg::const_iterator CashFlows::previousCashFlow(const Leg& leg,
                                                    Date refDate) {
        if (refDate==Date())
            refDate = Settings::instance().evaluationDate();

        if ( ! (*leg.begin())->hasOccurred(refDate) )
            return leg.end();

        Leg::const_iterator i = nextCashFlow(leg, refDate);
        Date beforeLastPaymentDate = (*--i)->date()-1;
        return nextCashFlow(leg, beforeLastPaymentDate);
    }

    Leg::const_iterator CashFlows::nextCashFlow(const Leg& leg,
                                                Date refDate) {
        if (refDate==Date())
            refDate = Settings::instance().evaluationDate();

        Leg::const_iterator i;
        for (i = leg.begin(); i<leg.end(); ++i) {
            // the first coupon paying after d is the one we're after
            if ( ! (*i)->hasOccurred(refDate) )
                return i;
        }
        return leg.end();
    }

    Rate CashFlows::previousCouponRate(const Leg& leg,
                                       const Date& refDate) {
        Leg::const_iterator cf = previousCashFlow(leg, refDate);
        return couponRate(leg, cf);
    }

    Rate CashFlows::nextCouponRate(const Leg& leg,
                                   const Date& refDate) {
        Leg::const_iterator cf = nextCashFlow(leg, refDate);
        return couponRate(leg, cf);
    }

    Date CashFlows::startDate(const Leg& cashflows) {
        Date d = Date::maxDate();
        for (Size i=0; i<cashflows.size(); ++i) {
            shared_ptr<Coupon> c =
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
                        Date settlementDate,
                        const Date& npvDate,
                        Natural exDividendDays) {
        if (settlementDate==Date())
            settlementDate = discountCurve.referenceDate();

        Real totalNPV = 0.0;
        for (Size i=0; i<cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(settlementDate+exDividendDays))
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
        if (settlementDate==Date())
            settlementDate = Settings::instance().evaluationDate();

        FlatForward flatRate(settlementDate, irr.rate(), irr.dayCounter(),
                             irr.compounding(), irr.frequency());
        return npv(cashflows, flatRate, settlementDate, settlementDate);
    }

    Real CashFlows::bps(const Leg& cashflows,
                        const YieldTermStructure& discountCurve,
                        Date settlementDate,
                        const Date& npvDate,
                        Natural exDividendDays) {
        if (settlementDate==Date())
            settlementDate = discountCurve.referenceDate();

        BPSCalculator calc(discountCurve, npvDate);
        for (Size i=0; i<cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(settlementDate+exDividendDays))
                cashflows[i]->accept(calc);
        }
        return basisPoint_*calc.result();
    }

    Real CashFlows::bps(const Leg& cashflows,
                        const InterestRate& irr,
                        Date settlementDate) {
        if (settlementDate==Date())
            settlementDate = Settings::instance().evaluationDate();

        FlatForward flatRate(settlementDate, irr.rate(), irr.dayCounter(),
                             irr.compounding(), irr.frequency());
        return bps(cashflows, flatRate, settlementDate, settlementDate);
    }

    Rate CashFlows::atmRate(const Leg& cashFlows,
                            const YieldTermStructure& discountCurve,
                            const Date& settlementDate,
                            const Date& npvDate,
                            Natural exDividendDays,
                            Real npv) {
        Real bps = CashFlows::bps(cashFlows, discountCurve, settlementDate,
                                  npvDate, exDividendDays);
        if (npv==Null<Real>())
            npv = CashFlows::npv(cashFlows, discountCurve, settlementDate,
                                 npvDate, exDividendDays);
        return basisPoint_*npv/bps;
    }

    Rate CashFlows::irr(const Leg& cashflows,
                        Real marketPrice,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        Date settlementDate,
                        Real accuracy,
                        Size maxIterations,
                        Rate guess) {

        if (settlementDate==Date())
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

        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        return solver.solve(IrrFinder(cashflows, marketPrice, dayCounter,
                                      compounding, frequency, settlementDate),
                            accuracy, guess, guess/10.0);
    }

    Time CashFlows::duration(const Leg& cashflows,
                             const InterestRate& rate,
                             Duration::Type type,
                             Date settlementDate) {
        if (settlementDate==Date())
            settlementDate = Settings::instance().evaluationDate();

        switch (type) {
          case Duration::Simple:
            return simpleDuration(cashflows, rate, settlementDate);
          case Duration::Modified:
            return modifiedDuration(cashflows, rate, settlementDate);
          case Duration::Macaulay:
            return macaulayDuration(cashflows, rate, settlementDate);
          default:
            QL_FAIL("unknown duration type");
        }
    }

    Real CashFlows::convexity(const Leg& cashflows,
                              const InterestRate& y,
                              Date settlementDate) {
        if (settlementDate==Date())
            settlementDate = Settings::instance().evaluationDate();

        const DayCounter& dc = y.dayCounter();

        Real P = 0.0;
        Real d2Pdy2 = 0.0;
        Rate r = y.rate();
        Natural N = y.frequency();
        for (Size i=0; i<cashflows.size(); ++i) {
            if (!cashflows[i]->hasOccurred(settlementDate)) {
                Time t = dc.yearFraction(settlementDate,
                                         cashflows[i]->date());
                Real c = cashflows[i]->amount();
                DiscountFactor B = y.discountFactor(t);
                P += c * B;
                switch (y.compounding()) {
                  case Simple:
                    d2Pdy2 += c * 2.0*B*B*B*t*t;
                    break;
                  case Compounded:
                    d2Pdy2 += c * B*t*(N*t+1)/(N*(1+r/N)*(1+r/N));
                    break;
                  case Continuous:
                    d2Pdy2 += c * B*t*t;
                    break;
                  case SimpleThenCompounded:
                    if (t<=1.0/N)
                        d2Pdy2 += c * 2.0*B*B*B*t*t;
                    else
                        d2Pdy2 += c * B*t*(N*t+1)/(N*(1+r/N)*(1+r/N));
                    break;
                  default:
                    QL_FAIL("unknown compounding convention (" <<
                            Integer(y.compounding()) << ")");
                }
            }
        }

        if (P == 0.0)
            // no cashflows
            return 0.0;

        return d2Pdy2/P;
    }

}
