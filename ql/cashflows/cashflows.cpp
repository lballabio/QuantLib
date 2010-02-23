/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2007, 2008, 2009, 2010 Ferdinando Ametrano
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
                              bool includeSettlementDateFlows,
                              Date settlementDate)
    {
        if (leg.empty())
            return true;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        for (Size i=leg.size(); i>0; --i)
            if (!leg[i-1]->hasOccurred(settlementDate,
                                       includeSettlementDateFlows))
                return false;
        return true;
    }

    Leg::const_reverse_iterator
    CashFlows::previousCashFlow(const Leg& leg,
                                bool includeSettlementDateFlows,
                                Date settlementDate) {
        if (leg.empty())
            return leg.rend();

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_reverse_iterator i;
        for (i = leg.rbegin(); i<leg.rend(); ++i) {
            if ( (*i)->hasOccurred(settlementDate, includeSettlementDateFlows) )
                return i;
        }
        return leg.rend();
    }

    Leg::const_iterator
    CashFlows::nextCashFlow(const Leg& leg,
                            bool includeSettlementDateFlows,
                            Date settlementDate) {
        if (leg.empty())
            return leg.end();

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_iterator i;
        for (i = leg.begin(); i<leg.end(); ++i) {
            if ( ! (*i)->hasOccurred(settlementDate, includeSettlementDateFlows) )
                return i;
        }
        return leg.end();
    }

    Date CashFlows::previousCashFlowDate(const Leg& leg,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.rend())
            return Date();

        return (*cf)->date();
    }

    Date CashFlows::nextCashFlowDate(const Leg& leg,
                                     bool includeSettlementDateFlows,
                                     Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.end())
            return Date();

        return (*cf)->date();
    }

    Real CashFlows::previousCashFlowAmount(const Leg& leg,
                                           bool includeSettlementDateFlows,
                                           Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.rend())
            return Real();

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.rend() && (*cf)->date()==paymentDate; ++cf)
            result += (*cf)->amount();
        return result;
    }

    Real CashFlows::nextCashFlowAmount(const Leg& leg,
                                       bool includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.end())
            return Real();

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf)
            result += (*cf)->amount();
        return result;
    }

    // Coupon utility functions
    namespace {

        template<typename Iter>
        Rate aggregateRate(const Leg& leg,
                           Iter first,
                           Iter last) {
            if (first==last) return 0.0;

            Date paymentDate = (*first)->date();
            bool firstCouponFound = false;
            Real nominal = 0.0;
            Time accrualPeriod = 0.0;
            DayCounter dc;
            Rate result = 0.0;
            for (; first<last && (*first)->date()==paymentDate; ++first) {
                shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*first);
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
                      "no coupon paid at cashflow date " << paymentDate);
            return result;
        }

    } // anonymous namespace ends here

    Rate CashFlows::previousCouponRate(const Leg& leg,
                                       bool includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        return aggregateRate<Leg::const_reverse_iterator>(leg, cf, leg.rend());
    }

    Rate CashFlows::nextCouponRate(const Leg& leg,
                                   bool includeSettlementDateFlows,
                                   Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        return aggregateRate<Leg::const_iterator>(leg, cf, leg.end());
    }

    Date CashFlows::accrualStartDate(const Leg& leg,
                                     bool includeSettlementDateFlows,
                                     Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualStartDate();
        }
        return Date();
    }

    Date CashFlows::accrualEndDate(const Leg& leg,
                                   bool includeSettlementDateFlows,
                                   Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualEndDate();
        }
        return Date();
    }

    Date CashFlows::referencePeriodStart(const Leg& leg,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->referencePeriodStart();
        }
        return Date();
    }

    Date CashFlows::referencePeriodEnd(const Leg& leg,
                                       bool includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->referencePeriodEnd();
        }
        return Date();
    }

    Time CashFlows::accrualPeriod(const Leg& leg,
                                  bool includeSettlementDateFlows,
                                  Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualPeriod();
        }
        return 0;
    }

    BigInteger CashFlows::accrualDays(const Leg& leg,
                                      bool includeSettlementDateFlows,
                                      Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualDays();
        }
        return 0;
    }

    Time CashFlows::accruedPeriod(const Leg& leg,
                                  bool includeSettlementDateFlows,
                                  Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accruedPeriod(settlementDate);
        }
        return 0;
    }

    BigInteger CashFlows::accruedDays(const Leg& leg,
                                      bool includeSettlementDateFlows,
                                      Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accruedDays(settlementDate);
        }
        return 0;
    }

    Real CashFlows::accruedAmount(const Leg& leg,
                                  bool includeSettlementDateFlows,
                                  Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
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
            BPSCalculator(const YieldTermStructure& discountCurve,
                          Date npvDate)
            : discountCurve_(discountCurve), npvDate_(npvDate), result_(0.0) {
                QL_REQUIRE(npvDate_!=Date(), "null npv date");
            }
            void visit(Coupon& c) {
                result_ += c.nominal() *
                           c.accrualPeriod() *
                           discountCurve_.discount(c.date());
            }
            void visit(CashFlow&) {}
            Real result() const {
                return result_/discountCurve_.discount(npvDate_);
            }
          private:
            const YieldTermStructure& discountCurve_;
            Date npvDate_;
            Real result_;
        };

        const Spread basisPoint_ = 1.0e-4;
    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real totalNPV = 0.0;
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows))
                totalNPV += leg[i]->amount() *
                            discountCurve.discount(leg[i]->date());
        }

        return totalNPV/discountCurve.discount(npvDate);
    }

    Real CashFlows::bps(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        BPSCalculator calc(discountCurve, npvDate);
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows))
                leg[i]->accept(calc);
        }
        return basisPoint_*calc.result();
    }

    Rate CashFlows::atmRate(const Leg& leg,
                            const YieldTermStructure& discountCurve,
                            bool includeSettlementDateFlows,
                            Date settlementDate,
                            Date npvDate,
                            Real npv) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real bps = CashFlows::bps(leg, discountCurve,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
        if (npv==Null<Real>())
            npv = CashFlows::npv(leg, discountCurve,
                                 includeSettlementDateFlows,
                                 settlementDate, npvDate);
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
                            bool includeSettlementDateFlows,
                            Date settlementDate,
                            Date npvDate) {
            if (leg.empty())
                return 0.0;

            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

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
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
            if (leg.empty())
                return 0.0;

            QL_REQUIRE(settlementDate!=Date(), "null settlement date");

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
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {

            QL_REQUIRE(y.compounding() == Compounded,
                       "compounded rate required");

            return (1.0+y.rate()/y.frequency()) *
                modifiedDuration(leg, y,
                                 includeSettlementDateFlows,
                                 settlementDate, npvDate);
        }

        class IrrFinder : public std::unary_function<Rate, Real> {
          public:
            IrrFinder(const Leg& leg,
                      Real npv,
                      const DayCounter& dayCounter,
                      Compounding comp,
                      Frequency freq,
                      bool includeSettlementDateFlows,
                      Date settlementDate,
                      Date npvDate)
            : leg_(leg), npv_(npv),
              dayCounter_(dayCounter), compounding_(comp), frequency_(freq),
              includeSettlementDateFlows_(includeSettlementDateFlows),
              settlementDate_(settlementDate),
              npvDate_(npvDate) {

                QL_REQUIRE(!leg.empty(), "empty leg");

            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

                if (npvDate == Date())
                    npvDate = settlementDate;

                checkSign();
            }
            Real operator()(Rate y) const {
                InterestRate yield(y, dayCounter_, compounding_, frequency_);
                Real NPV = CashFlows::npv(leg_, yield,
                                          includeSettlementDateFlows_,
                                          settlementDate_, npvDate_);
                return npv_ - NPV;
            }
            Real derivative(Rate y) const {
                InterestRate yield(y, dayCounter_, compounding_, frequency_);
                return modifiedDuration(leg_, yield,
                                        includeSettlementDateFlows_,
                                        settlementDate_, npvDate_);
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
            bool includeSettlementDateFlows_;
            Date settlementDate_, npvDate_;
        };



    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const InterestRate& y,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

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
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        return npv(leg, InterestRate(yield, dc, comp, freq),
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    Real CashFlows::bps(const Leg& leg,
                        const InterestRate& yield,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        FlatForward flatRate(settlementDate, yield.rate(), yield.dayCounter(),
                             yield.compounding(), yield.frequency());
        return bps(leg, flatRate,
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    Real CashFlows::bps(const Leg& leg,
                        Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        return bps(leg, InterestRate(yield, dc, comp, freq),
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    Rate CashFlows::yield(const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          bool includeSettlementDateFlows,
                          Date settlementDate,
                          Date npvDate,
                          Real accuracy,
                          Size maxIterations,
                          Rate guess) {
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        IrrFinder objFunction(leg, npv,
                              dayCounter, compounding, frequency,
                              includeSettlementDateFlows,
                              settlementDate, npvDate);
        return solver.solve(objFunction, accuracy, guess, guess/10.0);
    }


    Time CashFlows::duration(const Leg& leg,
                             const InterestRate& rate,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate,
                             Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        switch (type) {
          case Duration::Simple:
            return simpleDuration(leg, rate,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
          case Duration::Modified:
            return modifiedDuration(leg, rate,
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
          case Duration::Macaulay:
            return macaulayDuration(leg, rate,
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
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
                             bool includeSettlementDateFlows,
                             Date settlementDate,
                             Date npvDate) {
        return duration(leg, InterestRate(yield, dc, comp, freq),
                        type,
                        includeSettlementDateFlows,
                        settlementDate, npvDate);
    }

    Real CashFlows::convexity(const Leg& leg,
                              const InterestRate& y,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

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
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
        return convexity(leg, InterestRate(yield, dc, comp, freq),
                         includeSettlementDateFlows,
                         settlementDate, npvDate);
    }

    Real CashFlows::basisPointValue(const Leg& leg,
                                    const InterestRate& y,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real npv = CashFlows::npv(leg, y,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
        Real modifiedDuration = CashFlows::duration(leg, y,
                                                    Duration::Modified,
                                                    includeSettlementDateFlows,
                                                    settlementDate, npvDate);
        Real convexity = CashFlows::convexity(leg, y,
                                              includeSettlementDateFlows,
                                              settlementDate, npvDate);
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
                                    bool includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate) {
        return basisPointValue(leg, InterestRate(yield, dc, comp, freq),
                               includeSettlementDateFlows,
                               settlementDate, npvDate);
    }

    Real CashFlows::yieldValueBasisPoint(const Leg& leg,
                                         const InterestRate& y,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate,
                                         Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real npv = CashFlows::npv(leg, y,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
        Real modifiedDuration = CashFlows::duration(leg, y,
                                                    Duration::Modified,
                                                    includeSettlementDateFlows,
                                                    settlementDate, npvDate);

        Real shift = 0.01;
        return (1.0/(-npv*modifiedDuration))*shift;
    }

    Real CashFlows::yieldValueBasisPoint(const Leg& leg,
                                         Rate yield,
                                         const DayCounter& dc,
                                         Compounding comp,
                                         Frequency freq,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate,
                                         Date npvDate) {
        return yieldValueBasisPoint(leg, InterestRate(yield, dc, comp, freq),
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
    }

    // Z-spread utility functions
    namespace {

        class ZSpreadFinder : public std::unary_function<Rate, Real> {
          public:
            ZSpreadFinder(const Leg& leg,
                          const shared_ptr<YieldTermStructure>& discountCurve,
                          Real npv,
                          const DayCounter& dc,
                          Compounding comp,
                          Frequency freq,
                          bool includeSettlementDateFlows,
                          Date settlementDate,
                          Date npvDate)
            : leg_(leg), npv_(npv), zSpread_(new SimpleQuote(0.0)),
              curve_(Handle<YieldTermStructure>(discountCurve),
                     Handle<Quote>(zSpread_), comp, freq, dc),
              includeSettlementDateFlows_(includeSettlementDateFlows),
              settlementDate_(settlementDate),
              npvDate_(npvDate) {
                QL_REQUIRE(!leg.empty(), "empty leg");

                if (settlementDate == Date())
                    settlementDate = Settings::instance().evaluationDate();

                if (npvDate == Date())
                    npvDate = settlementDate;
            }
            Real operator()(Rate zSpread) const {
                zSpread_->setValue(zSpread);
                Real NPV = CashFlows::npv(leg_, curve_,
                                          includeSettlementDateFlows_,
                                          settlementDate_, npvDate_);
                return npv_ - NPV;
            }
          private:
            const Leg& leg_;
            Real npv_;
            shared_ptr<SimpleQuote> zSpread_;
            ZeroSpreadedTermStructure curve_;
            bool includeSettlementDateFlows_;
            Date settlementDate_, npvDate_;
        };

    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const shared_ptr<YieldTermStructure>& discountCurve,
                        Spread zSpread,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(shared_ptr<Quote>(new
            SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                comp, freq, dc);
        return npv(leg, spreadedCurve,
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    Spread CashFlows::zSpread(const Leg& leg,
                              Real npv,
                              const shared_ptr<YieldTermStructure>& discount,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Brent solver;
        solver.setMaxEvaluations(maxIterations);
        ZSpreadFinder objFunction(leg,
                                  discount,
                                  npv,
                                  dayCounter, compounding, frequency, includeSettlementDateFlows,
                                  settlementDate, npvDate);
        Real step = 0.01;
        return solver.solve(objFunction, accuracy, guess, step);
    }

}
