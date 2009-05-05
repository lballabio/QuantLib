/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2008 Toyin Akin

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
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    // Date inspectors

    Date CashFlows::startDate(const Leg& leg) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        Date d = Date::maxDate();
        for (Size i=0; i<leg.size(); ++i) {
            shared_ptr<Coupon> c = dynamic_pointer_cast<Coupon>(leg[i]);
            if (c)
                d = std::min(d, c->accrualStartDate());
            else
                d = std::min(d, leg[i]->date());
        }
        return d;
    }

    Date CashFlows::maturityDate(const Leg& leg) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        Date d = Date::minDate();
        for (Size i=0; i<leg.size(); ++i) {
            shared_ptr<Coupon> c = dynamic_pointer_cast<Coupon>(leg[i]);
            if (c)
                d = std::max(d, c->accrualEndDate());
            else
                d = std::max(d, leg[i]->date());
        }
        return d;
    }

    bool CashFlows::isExpired(const Leg& leg,
                              Date settlementDate,
                              bool includeSettlementDateFlows)
    {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        for (Size i=leg.size(); i>0; --i)
            if (!leg[i-1]->hasOccurred(settlementDate,
                                       includeSettlementDateFlows))
                return false;
        return true;
    }

    Leg::const_iterator
    CashFlows::previousCashFlow(const Leg& leg,
                                Date settlementDate,
                                bool includeSettlementDateFlows) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        if ( ! (*leg.begin())->hasOccurred(settlementDate,
                                           includeSettlementDateFlows) )
            return leg.end();

        Leg::const_iterator i = nextCashFlow(leg, settlementDate,
                                             includeSettlementDateFlows);
        // --i is not what we're looking for since there
        // might be more than one CashFlow at (*--i)->date()
        Date beforeLastPaymentDate = (*--i)->date()-1;
        return nextCashFlow(leg, beforeLastPaymentDate,
                            includeSettlementDateFlows);
    }

    Leg::const_iterator
    CashFlows::nextCashFlow(const Leg& leg,
                            Date settlementDate,
                            bool includeSettlementDateFlows) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        Leg::const_iterator i;
        for (i = leg.begin(); i<leg.end(); ++i) {
            if ( ! (*i)->hasOccurred(settlementDate, includeSettlementDateFlows) )
                return i;
        }
        return leg.end();
    }

    Date CashFlows::previousCashFlowDate(const Leg& leg,
                                         Date settlementDate,
                                         bool includeSettlementDateFlows) {
        Leg::const_iterator cf = previousCashFlow(leg, settlementDate,
                                                  includeSettlementDateFlows);
        if (cf==leg.end()) return Date();
        return (*cf)->date();
    }

    Date CashFlows::nextCashFlowDate(const Leg& leg,
                                     Date settlementDate,
                                     bool includeSettlementDateFlows) {
        Leg::const_iterator cf = nextCashFlow(leg, settlementDate,
                                              includeSettlementDateFlows);
        if (cf==leg.end()) return Date();
        return (*cf)->date();
    }

    Real CashFlows::previousCashFlowAmount(const Leg& leg,
                                           Date settlementDate,
                                           bool includeSettlementDateFlows) {
        Leg::const_iterator cf = previousCashFlow(leg, settlementDate,
                                                  includeSettlementDateFlows);
        if (cf==leg.end()) return Real();
        return (*cf)->amount();
    }

    Real CashFlows::nextCashFlowAmount(const Leg& leg,
                                       Date settlementDate,
                                       bool includeSettlementDateFlows) {
        Leg::const_iterator cf = nextCashFlow(leg, settlementDate,
                                              includeSettlementDateFlows);
        if (cf==leg.end()) return Real();
        return (*cf)->amount();
    }

    // Coupon utility functions
    namespace {

        Rate aggregateRate(const Leg& leg,
                           Leg::const_iterator cf) {
            if (cf==leg.end()) return 0.0;

            Date paymentDate = (*cf)->date();
            bool firstCouponFound = false;
            Real nominal = 0.0;
            Time accrualPeriod = 0.0;
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

    } // anonymous namespace ends here

    Rate CashFlows::previousCouponRate(const Leg& leg,
                                       Date settlementDate,
                                       bool includeSettlementDateFlows) {
        Leg::const_iterator cf = previousCashFlow(leg, settlementDate,
                                                  includeSettlementDateFlows);
        return aggregateRate(leg, cf);
    }

    Rate CashFlows::nextCouponRate(const Leg& leg,
                                   Date settlementDate,
                                   bool includeSettlementDateFlows) {
        Leg::const_iterator cf = nextCashFlow(leg, settlementDate,
                                              includeSettlementDateFlows);
        return aggregateRate(leg, cf);
    }

    Real CashFlows::accruedAmount(const Leg& leg,
                                  Date settlementDate,
                                  bool includeSettlementDateFlows) {
        Leg::const_iterator cf = nextCashFlow(leg, settlementDate,
                                              includeSettlementDateFlows);
        if (cf==leg.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                result += cp->accruedAmount(settlementDate);
        }
        return result;
    }

    // YieldTermStructure utility functions
    namespace {

        class BPSCalculator : public AcyclicVisitor,
                              public Visitor<CashFlow>,
                              public Visitor<Coupon> {
          public:
            BPSCalculator(const YieldTermStructure& termStructure,
                          Date npvDate)
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
    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        if (settlementDate == Date())
            settlementDate = discountCurve.referenceDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        Real totalNPV = 0.0;
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows))
                totalNPV += leg[i]->amount() *
                            discountCurve.discount(leg[i]->date());
        }

        if (npvDate==Date())
            return totalNPV;
        else
            return totalNPV/discountCurve.discount(npvDate);
    }

    Real CashFlows::bps(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        if (settlementDate == Date())
            settlementDate = discountCurve.referenceDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        BPSCalculator calc(discountCurve, npvDate);
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows))
                leg[i]->accept(calc);
        }
        return basisPoint_*calc.result();
    }

    Rate CashFlows::atmRate(const Leg& cashFlows,
                            const YieldTermStructure& discountCurve,
                            Date settlementDate,
                            Date npvDate,
                            bool includeSettlementDateFlows,
                            Real npv) {
        Real bps = CashFlows::bps(cashFlows, discountCurve,
                                  settlementDate, npvDate,
                                  includeSettlementDateFlows);
        if (npv==Null<Real>())
            npv = CashFlows::npv(cashFlows, discountCurve,
                                 settlementDate, npvDate,
                                 includeSettlementDateFlows);
        return basisPoint_*npv/bps;
    }

    // IRR utility functions
    namespace {

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

        Real simpleDuration(const Leg& leg,
                            const InterestRate& y,
                            Date settlementDate,
                            Date npvDate,
                            bool includeSettlementDateFlows) {
            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

            #ifndef QL_TODAYS_PAYMENTS
            if (settlementDate == Settings::instance().evaluationDate())
                includeSettlementDateFlows = false;
            #endif

            if (npvDate == Date())
                npvDate = settlementDate;

            Real P = 0.0;
            Real dPdy = 0.0;
            const DayCounter& dc = y.dayCounter();
            for (Size i=0; i<leg.size(); ++i) {
                if (!leg[i]->hasOccurred(settlementDate,
                                         includeSettlementDateFlows)) {
                    Time t = dc.yearFraction(npvDate,
                                             leg[i]->date());
                    Real c = leg[i]->amount();
                    DiscountFactor B = y.discountFactor(t);
                    P += c * B;
                    dPdy += t * c * B;
                }
            }
            if (P == 0.0) // no cashflows
                return 0.0;
            return dPdy/P;
        }

        Real modifiedDuration(const Leg& leg,
                              const InterestRate& y,
                              Date settlementDate,
                              Date npvDate,
                              bool includeSettlementDateFlows) {
            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

            #ifndef QL_TODAYS_PAYMENTS
            if (settlementDate == Settings::instance().evaluationDate())
                includeSettlementDateFlows = false;
            #endif

            if (npvDate == Date())
                npvDate = settlementDate;

            Real P = 0.0;
            Real dPdy = 0.0;
            Rate r = y.rate();
            Natural N = y.frequency();
            const DayCounter& dc = y.dayCounter();
            for (Size i=0; i<leg.size(); ++i) {
                if (!leg[i]->hasOccurred(settlementDate,
                                         includeSettlementDateFlows)) {
                    Time t = dc.yearFraction(npvDate,
                                             leg[i]->date());
                    Real c = leg[i]->amount();
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

        Real macaulayDuration(const Leg& leg,
                              const InterestRate& y,
                              Date settlementDate,
                              Date npvDate,
                              bool includeSettlementDateFlows) {

            QL_REQUIRE(y.compounding() == Compounded,
                       "compounded rate required");

            return (1.0+y.rate()/y.frequency()) *
                modifiedDuration(leg, y,
                                 settlementDate, npvDate,
                                 includeSettlementDateFlows);
        }

        class IrrFinder : public std::unary_function<Rate, Real> {
          public:
            IrrFinder(const Leg& leg,
                      Real npv,
                      const DayCounter& dayCounter,
                      Compounding comp,
                      Frequency freq,
                      Date settlement,
                      Date npvDate,
                      bool includeSettlementDateFlows)
            : leg_(leg), npv_(npv),
              dayCounter_(dayCounter), compounding_(comp), frequency_(freq),
              settlementDate_(settlement), npvDate_(npvDate),
              includeSettlementDateFlows_(includeSettlementDateFlows) {
                if (settlementDate_ == Date())
                    settlementDate_ = Settings::instance().evaluationDate();

                #ifndef QL_TODAYS_PAYMENTS
                if (settlementDate_ == Settings::instance().evaluationDate())
                    includeSettlementDateFlows_ = false;
                #endif

                if (npvDate_ == Date())
                    npvDate_ = settlementDate_;

                checkSign();
            }
            Real operator()(Rate y) const {
                InterestRate yield(y, dayCounter_, compounding_, frequency_);
                Real NPV = CashFlows::npv(leg_, yield,
                                          settlementDate_, npvDate_,
                                          includeSettlementDateFlows_);
                return npv_ - NPV;
            }
            Real derivative(Rate y) const {
                InterestRate yield(y, dayCounter_, compounding_, frequency_);
                return modifiedDuration(leg_, yield,
                                        settlementDate_, npvDate_,
                                        includeSettlementDateFlows_);
            }
          private:
            void checkSign() const {
                // depending on the sign of the market price, check that cash
                // flows of the opposite sign have been specified (otherwise
                // IRR is nonsensical.)

                Integer lastSign = sign(-npv_),
                        signChanges = 0;
                for (Size i = 0; i < leg_.size(); ++i) {
                    if (!leg_[i]->hasOccurred(settlementDate_,
                                              includeSettlementDateFlows_)) {
                        Integer thisSign = sign(leg_[i]->amount());
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
                    Real aggregateCashFlow = npv;
                    signChanges = 0;
                    for (Size i = 0; i < leg.size(); ++i) {
                        Real nextAggregateCashFlow =
                            aggregateCashFlow + leg[i]->amount();

                        if (aggregateCashFlow * nextAggregateCashFlow < 0.0)
                            signChanges++;

                        aggregateCashFlow = nextAggregateCashFlow;
                    }
                    if (signChanges > 1)
                        QL_WARN( "danger of non-unique solution");
                };
                */
            }
            const Leg& leg_;
            Real npv_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
            Date settlementDate_, npvDate_;
            bool includeSettlementDateFlows_;
        };



    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const InterestRate& y,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        Real npv = 0.0;
        DiscountFactor discount = 1.0;
        Date lastDate = Date();

        for (Size i=0; i<leg.size(); ++i) {
            if (leg[i]->hasOccurred(settlementDate,
                                    includeSettlementDateFlows))
                continue;

            Date couponDate = leg[i]->date();
            Real amount = leg[i]->amount();
            if (lastDate == Date()) {
                // first not-expired coupon
                if (i > 0) {
                    lastDate = leg[i-1]->date();
                } else {
                    shared_ptr<Coupon> coupon =
                        boost::dynamic_pointer_cast<Coupon>(leg[i]);
                    if (coupon)
                        lastDate = coupon->accrualStartDate();
                    else
                        lastDate = couponDate - 1*Years;
                }
                if (npvDate==Date())
                    discount *= y.discountFactor(settlementDate, couponDate,
                                                 lastDate, couponDate);
                else
                    discount *= y.discountFactor(npvDate, couponDate,
                                                 lastDate, couponDate);
            } else  {
                discount *= y.discountFactor(lastDate, couponDate);
            }
            lastDate = couponDate;

            npv += amount * discount;
        }

        return npv;
    }

    Real CashFlows::npv(const Leg& leg,
                        Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        return npv(leg, InterestRate(yield, dc, comp, freq),
                   settlementDate, npvDate,
                   includeSettlementDateFlows);  
    }

    Real CashFlows::bps(const Leg& leg,
                        const InterestRate& yield,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        FlatForward flatRate(settlementDate, yield.rate(), yield.dayCounter(),
                             yield.compounding(), yield.frequency());
        return bps(leg, flatRate,
                   settlementDate, npvDate,
                   includeSettlementDateFlows);
    }

    Real CashFlows::bps(const Leg& leg,
                        Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        return bps(leg, InterestRate(yield, dc, comp, freq),
                   settlementDate, npvDate,
                   includeSettlementDateFlows);  
    }

    Rate CashFlows::yield(const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          Date settlementDate,
                          Date npvDate,
                          bool includeSettlementDateFlows,
                          Real accuracy,
                          Size maxIterations,
                          Rate guess) {
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        IrrFinder objFunction(leg, npv,
                              dayCounter, compounding, frequency,
                              settlementDate, npvDate,
                              includeSettlementDateFlows);
        return solver.solve(objFunction, accuracy, guess, guess/10.0);
    }


    Time CashFlows::duration(const Leg& leg,
                             const InterestRate& rate,
                             Duration::Type type,
                             Date settlementDate,
                             Date npvDate,
                             bool includeSettlementDateFlows) {
        switch (type) {
          case Duration::Simple:
            return simpleDuration(leg, rate,
                                  settlementDate, npvDate,
                                  includeSettlementDateFlows);
          case Duration::Modified:
            return modifiedDuration(leg, rate,
                                    settlementDate, npvDate,
                                    includeSettlementDateFlows);
          case Duration::Macaulay:
            return macaulayDuration(leg, rate,
                                    settlementDate, npvDate,
                                    includeSettlementDateFlows);
          default:
            QL_FAIL("unknown duration type");
        }
    }

    Time CashFlows::duration(const Leg& leg,
                             Rate yield,
                             const DayCounter& dc,
                             Compounding comp,
                             Frequency freq,
                             Duration::Type type,
                             Date settlementDate,
                             Date npvDate,
                             bool includeSettlementDateFlows) {
        return duration(leg, InterestRate(yield, dc, comp, freq),
                        type,
                        settlementDate, npvDate,
                        includeSettlementDateFlows);  
    }

    Real CashFlows::convexity(const Leg& leg,
                              const InterestRate& y,
                              Date settlementDate,
                              Date npvDate,
                              bool includeSettlementDateFlows) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        if (npvDate == Date())
            npvDate = settlementDate;

        const DayCounter& dc = y.dayCounter();

        Real P = 0.0;
        Real d2Pdy2 = 0.0;
        Rate r = y.rate();
        Natural N = y.frequency();
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows)) {
                Time t = dc.yearFraction(npvDate,
                                         leg[i]->date());
                Real c = leg[i]->amount();
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


    Real CashFlows::convexity(const Leg& leg,
                              Rate yield,
                              const DayCounter& dc,
                              Compounding comp,
                              Frequency freq,
                              Date settlementDate,
                              Date npvDate,
                              bool includeSettlementDateFlows) {
        return convexity(leg, InterestRate(yield, dc, comp, freq),
                         settlementDate, npvDate,
                         includeSettlementDateFlows);  
    }

    Real CashFlows::basisPointValue(const Leg& leg,
                                    const InterestRate& y,
                                    Date settlementDate,
                                    Date npvDate,
                                    bool includeSettlementDateFlows) {
        Real npv = CashFlows::npv(leg, y,
                                  settlementDate, npvDate,
                                  includeSettlementDateFlows);
        Real modifiedDuration = CashFlows::duration(leg, y,
                                                    Duration::Modified,
                                                    settlementDate, npvDate,
                                                    includeSettlementDateFlows);
        Real convexity = CashFlows::convexity(leg, y,
                                              settlementDate, npvDate,
                                              includeSettlementDateFlows);

        Real delta = -modifiedDuration*npv;
        Real gamma = (convexity/100.0)*npv;

        Real shift = 0.0001;
        delta *= shift;
        gamma *= shift*shift;

        return delta + 0.5*gamma;
    }

    Real CashFlows::basisPointValue(const Leg& leg,
                                    Rate yield,
                                    const DayCounter& dc,
                                    Compounding comp,
                                    Frequency freq,
                                    Date settlementDate,
                                    Date npvDate,
                                    bool includeSettlementDateFlows) {
        return basisPointValue(leg, InterestRate(yield, dc, comp, freq),
                               settlementDate, npvDate,
                               includeSettlementDateFlows);  
    }

    Real CashFlows::yieldValueBasisPoint(const Leg& leg,
                                         const InterestRate& y,
                                         Date settlementDate,
                                         Date npvDate,
                                         bool includeSettlementDateFlows) {
        Real npv = CashFlows::npv(leg, y,
                                  settlementDate, npvDate,
                                  includeSettlementDateFlows);
        Real modifiedDuration = CashFlows::duration(leg, y,
                                                    Duration::Modified,
                                                    settlementDate, npvDate,
                                                    includeSettlementDateFlows);

        Real shift = 0.01;
        return (1.0/(-npv*modifiedDuration))*shift;
    }

    Real CashFlows::yieldValueBasisPoint(const Leg& leg,
                                         Rate yield,
                                         const DayCounter& dc,
                                         Compounding comp,
                                         Frequency freq,
                                         Date settlementDate,
                                         Date npvDate,
                                         bool includeSettlementDateFlows) {
        return yieldValueBasisPoint(leg, InterestRate(yield, dc, comp, freq),
                                    settlementDate, npvDate,
                                    includeSettlementDateFlows);  
    }

    // Z-spread utility functions
    namespace {

        class ZSpreadFinder : public std::unary_function<Rate, Real> {
          public:
            ZSpreadFinder(const Leg& leg,
                          Real npv,
                          const shared_ptr<YieldTermStructure>& discountCurve,
                          const DayCounter& dc,
                          Compounding comp,
                          Frequency freq,
                          Date settlement,
                          Date npvDate,
                          bool includeSettlementDateFlows)
            : leg_(leg), npv_(npv),
              zSpread_(new SimpleQuote(0.0)),
              curve_(Handle<YieldTermStructure>(discountCurve),
                     Handle<Quote>(zSpread_), comp, freq, dc),
              settlementDate_(settlement), npvDate_(npvDate),
              includeSettlementDateFlows_(includeSettlementDateFlows) {
                if (settlementDate_ == Date())
                    settlementDate_ = Settings::instance().evaluationDate();

                #ifndef QL_TODAYS_PAYMENTS
                if (settlementDate_ == Settings::instance().evaluationDate())
                    includeSettlementDateFlows_ = false;
                #endif

                if (npvDate_ == Date())
                    npvDate_ = settlementDate_;
            }
            Real operator()(Rate zSpread) const {
                zSpread_->setValue(zSpread);
                Real NPV = CashFlows::npv(leg_, curve_,
                                          settlementDate_, npvDate_,
                                          includeSettlementDateFlows_);
                return npv_ - NPV;
            }
          private:
            const Leg& leg_;
            Real npv_;
            shared_ptr<SimpleQuote> zSpread_;
            ZeroSpreadedTermStructure curve_;
            Date settlementDate_, npvDate_;
            bool includeSettlementDateFlows_;
        };

    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const shared_ptr<YieldTermStructure>& discountCurve,
                        Spread zSpread,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        Date settlementDate,
                        Date npvDate,
                        bool includeSettlementDateFlows) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        #ifndef QL_TODAYS_PAYMENTS
        if (settlementDate == Settings::instance().evaluationDate())
            includeSettlementDateFlows = false;
        #endif

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(shared_ptr<Quote>(new
            SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                comp, freq, dc);
        return npv(leg, spreadedCurve,
                   settlementDate, npvDate,
                   includeSettlementDateFlows);
    }

    Spread CashFlows::zSpread(const Leg& leg,
                              Real npv,
                              const shared_ptr<YieldTermStructure>& discount,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              Date settlementDate,
                              Date npvDate,
                              bool includeSettlementDateFlows,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess) {
        Brent solver;
        solver.setMaxEvaluations(maxIterations);
        ZSpreadFinder objFunction(leg, npv,
                                  discount,
                                  dayCounter, compounding, frequency,
                                  settlementDate, npvDate,
                                  includeSettlementDateFlows);
        Real step = 0.01;
        return solver.solve(objFunction, accuracy, guess, step);
    }

}
