/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ferdinando Ametrano
 Copyright (C) 2008 Toyin Akin

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <utility>

namespace QuantLib {

    // Date inspectors

    Date CashFlows::startDate(const Leg& leg) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        Date d = Date::maxDate();
        for (const auto& i : leg) {
            ext::shared_ptr<Coupon> c = ext::dynamic_pointer_cast<Coupon>(i);
            if (c != nullptr)
                d = std::min(d, c->accrualStartDate());
            else
                d = std::min(d, i->date());
        }
        return d;
    }

    Date CashFlows::maturityDate(const Leg& leg) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        Date d = Date::minDate();
        for (const auto& i : leg) {
            ext::shared_ptr<Coupon> c = ext::dynamic_pointer_cast<Coupon>(i);
            if (c != nullptr)
                d = std::max(d, c->accrualEndDate());
            else
                d = std::max(d, i->date());
        }
        return d;
    }

    bool CashFlows::isExpired(const Leg& leg,
                              const ext::optional<bool>& includeSettlementDateFlows,
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
                                const ext::optional<bool>& includeSettlementDateFlows,
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
                            const ext::optional<bool>& includeSettlementDateFlows,
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
                                         const ext::optional<bool>& includeSettlementDateFlows,
                                         Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.rend())
            return {};

        return (*cf)->date();
    }

    Date CashFlows::nextCashFlowDate(const Leg& leg,
                                     const ext::optional<bool>& includeSettlementDateFlows,
                                     Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.end())
            return {};

        return (*cf)->date();
    }

    Real CashFlows::previousCashFlowAmount(const Leg& leg,
                                           const ext::optional<bool>& includeSettlementDateFlows,
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
                                       const ext::optional<bool>& includeSettlementDateFlows,
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
        Rate aggregateRate(Iter first,
                           const Iter& last) {
            if (first==last) return 0.0;

            Date paymentDate = (*first)->date();
            bool firstCouponFound = false;
            Real nominal = 0.0;
            Time accrualPeriod = 0.0;
            DayCounter dc;
            Rate result = 0.0;
            for (; first<last && (*first)->date()==paymentDate; ++first) {
                ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*first);
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
                                       const ext::optional<bool>& includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        return aggregateRate(cf, leg.rend());
    }

    Rate CashFlows::nextCouponRate(const Leg& leg,
                                   const ext::optional<bool>& includeSettlementDateFlows,
                                   Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        return aggregateRate(cf, leg.end());
    }

    Real CashFlows::nominal(const Leg& leg,
                            const ext::optional<bool>& includeSettlementDateFlows,
                            Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->nominal();
        }
        return 0.0;
    }

    Date CashFlows::accrualStartDate(const Leg& leg,
                                     const ext::optional<bool>& includeSettlementDateFlows,
                                     Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end())
            return {};

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->accrualStartDate();
        }
        return {};
    }

    Date CashFlows::accrualEndDate(const Leg& leg,
                                   const ext::optional<bool>& includeSettlementDateFlows,
                                   Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end())
            return {};

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->accrualEndDate();
        }
        return {};
    }

    Date CashFlows::referencePeriodStart(const Leg& leg,
                                         const ext::optional<bool>& includeSettlementDateFlows,
                                         Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end())
            return {};

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->referencePeriodStart();
        }
        return {};
    }

    Date CashFlows::referencePeriodEnd(const Leg& leg,
                                       const ext::optional<bool>& includeSettlementDateFlows,
                                       Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end())
            return {};

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->referencePeriodEnd();
        }
        return {};
    }

    Time CashFlows::accrualPeriod(const Leg& leg,
                                  const ext::optional<bool>& includeSettlementDateFlows,
                                  Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->accrualPeriod();
        }
        return 0;
    }

    Date::serial_type CashFlows::accrualDays(const Leg& leg,
                                             const ext::optional<bool>& includeSettlementDateFlows,
                                             Date settlementDate) {
        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->accrualDays();
        }
        return 0;
    }

    Time CashFlows::accruedPeriod(const Leg& leg,
                                  const ext::optional<bool>& includeSettlementDateFlows,
                                  Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->accruedPeriod(settlementDate);
        }
        return 0;
    }

    Date::serial_type CashFlows::accruedDays(const Leg& leg,
                                             const ext::optional<bool>& includeSettlementDateFlows,
                                             Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
                return cp->accruedDays(settlementDate);
        }
        return 0;
    }

    Real CashFlows::accruedAmount(const Leg& leg,
                                  const ext::optional<bool>& includeSettlementDateFlows,
                                  Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        auto cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        if (cf==leg.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(*cf);
            if (cp != nullptr)
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
            explicit BPSCalculator(const YieldTermStructure& discountCurve)
            : discountCurve_(discountCurve) {}
            void visit(Coupon& c) override {
                Real bps = c.nominal() *
                           c.accrualPeriod() *
                           discountCurve_.discount(c.date());
                bps_ += bps;
            }
            void visit(CashFlow& cf) override {
                nonSensNPV_ += cf.amount() * 
                               discountCurve_.discount(cf.date());
            }
            Real bps() const { return bps_; }
            Real nonSensNPV() const { return nonSensNPV_; }
          private:
            const YieldTermStructure& discountCurve_;
            Real bps_ = 0.0, nonSensNPV_ = 0.0;
        };

        const Spread basisPoint_ = 1.0e-4;
    } // anonymous namespace ends here

    Real CashFlows::npv(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        const ext::optional<bool>& includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real totalNPV = 0.0;
        for (const auto& i : leg) {
            if (!i->hasOccurred(settlementDate, includeSettlementDateFlows) &&
                !i->tradingExCoupon(settlementDate))
                totalNPV += i->amount() * discountCurve.discount(i->date());
        }

        return totalNPV/discountCurve.discount(npvDate);
    }

    Real CashFlows::bps(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        const ext::optional<bool>& includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        BPSCalculator calc(discountCurve);
        for (const auto& i : leg) {
            if (!i->hasOccurred(settlementDate, includeSettlementDateFlows) &&
                !i->tradingExCoupon(settlementDate))
                i->accept(calc);
        }
        return basisPoint_*calc.bps()/discountCurve.discount(npvDate);
    }

    std::pair<Real, Real> CashFlows::npvbps(const Leg& leg,
                                            const YieldTermStructure& discountCurve,
                                            const ext::optional<bool>& includeSettlementDateFlows,
                                            Date settlementDate,
                                            Date npvDate) {
        Real npv = 0.0;
        Real bps = 0.0;

        if (leg.empty()) {
            return { npv, bps };
        }

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        for (const auto& i : leg) {
            CashFlow& cf = *i;
            if (!cf.hasOccurred(settlementDate,
                                includeSettlementDateFlows) &&
                !cf.tradingExCoupon(settlementDate)) {
                ext::shared_ptr<Coupon> cp = ext::dynamic_pointer_cast<Coupon>(i);
                Real df = discountCurve.discount(cf.date());
                npv += cf.amount() * df;
                if (cp != nullptr)
                    bps += cp->nominal() * cp->accrualPeriod() * df;
            }
        }
        DiscountFactor d = discountCurve.discount(npvDate);
        npv /= d;
        bps = basisPoint_ * bps / d;

        return { npv, bps };
    }

    Rate CashFlows::atmRate(const Leg& leg,
                            const YieldTermStructure& discountCurve,
                            const ext::optional<bool>& includeSettlementDateFlows,
                            Date settlementDate,
                            Date npvDate,
                            Real targetNpv) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real npv = 0.0;
        BPSCalculator calc(discountCurve);
        for (const auto& i : leg) {
            CashFlow& cf = *i;
            if (!cf.hasOccurred(settlementDate,
                                includeSettlementDateFlows) &&
                !cf.tradingExCoupon(settlementDate)) {
                npv += cf.amount() *
                       discountCurve.discount(cf.date());
                cf.accept(calc);
            }
        }

        if (targetNpv==Null<Real>())
            targetNpv = npv - calc.nonSensNPV();
        else {
            targetNpv *= discountCurve.discount(npvDate);
            targetNpv -= calc.nonSensNPV();
        }

        if (targetNpv==0.0)
            return 0.0;

        Real bps = calc.bps();
        QL_REQUIRE(bps!=0.0, "null bps: impossible atm rate");

        return targetNpv/bps;
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

        // helper fucntion used to calculate Time-To-Discount for each stage when calculating discount factor stepwisely
        Time getStepwiseDiscountTime(const ext::shared_ptr<QuantLib::CashFlow>& cashFlow,
                                     const DayCounter& dc,
                                     Date npvDate,
                                     Date lastDate) {
            Date cashFlowDate = cashFlow->date();
            Date refStartDate, refEndDate;
            ext::shared_ptr<Coupon> coupon =
                    ext::dynamic_pointer_cast<Coupon>(cashFlow);
            if (coupon != nullptr) {
                refStartDate = coupon->referencePeriodStart();
                refEndDate = coupon->referencePeriodEnd();
            } else {
                if (lastDate == npvDate) {
                    // we don't have a previous coupon date,
                    // so we fake it
                    refStartDate = cashFlowDate - 1*Years;
                } else  {
                    refStartDate = lastDate;
                }
                refEndDate = cashFlowDate;
            }

            if ((coupon != nullptr) && lastDate != coupon->accrualStartDate()) {
                Time couponPeriod = dc.yearFraction(coupon->accrualStartDate(),
                                                cashFlowDate, refStartDate, refEndDate);
                Time accruedPeriod = dc.yearFraction(coupon->accrualStartDate(),
                                                lastDate, refStartDate, refEndDate);
                return couponPeriod - accruedPeriod;
            } else {
                return dc.yearFraction(lastDate, cashFlowDate,
                                       refStartDate, refEndDate);
            }
        }

        Real simpleDuration(const Leg& leg,
                            const InterestRate& y,
                            const ext::optional<bool>& includeSettlementDateFlows,
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
            Time t = 0.0;
            Date lastDate = npvDate;
            const DayCounter& dc = y.dayCounter();
            for (const auto& i : leg) {
                if (i->hasOccurred(settlementDate, includeSettlementDateFlows))
                    continue;

                Real c = i->amount();
                if (i->tradingExCoupon(settlementDate)) {
                    c = 0.0;
                }

                t += getStepwiseDiscountTime(i, dc, npvDate, lastDate);
                DiscountFactor B = y.discountFactor(t);
                P += c * B;
                dPdy += t * c * B;

                lastDate = i->date();
            }
            if (P == 0.0) // no cashflows
                return 0.0;
            return dPdy/P;
        }

        Real modifiedDuration(const Leg& leg,
                              const InterestRate& y,
                              const ext::optional<bool>& includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
            if (leg.empty())
                return 0.0;

            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

            if (npvDate == Date())
                npvDate = settlementDate;

            Real P = 0.0;
            Time t = 0.0;
            Real dPdy = 0.0;
            Rate r = y.rate();
            Natural N = y.frequency();
            Date lastDate = npvDate;
            const DayCounter& dc = y.dayCounter();
            for (const auto& i : leg) {
                if (i->hasOccurred(settlementDate, includeSettlementDateFlows))
                    continue;

                Real c = i->amount();
                if (i->tradingExCoupon(settlementDate)) {
                    c = 0.0;
                }

                t += getStepwiseDiscountTime(i, dc, npvDate, lastDate);
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
                  case CompoundedThenSimple:
                    if (t>1.0/N)
                        dPdy -= c * B*B * t;
                    else
                        dPdy -= c * t * B/(1+r/N);
                    break;
                  default:
                    QL_FAIL("unknown compounding convention (" <<
                            Integer(y.compounding()) << ")");
                }
                lastDate = i->date();
            }

            if (P == 0.0) // no cashflows
                return 0.0;
            return -dPdy/P; // reverse derivative sign
        }

        Real macaulayDuration(const Leg& leg,
                              const InterestRate& y,
                              const ext::optional<bool>& includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {

            QL_REQUIRE(y.compounding() == Compounded,
                       "compounded rate required");

            return (1.0+y.rate()/Integer(y.frequency())) *
                modifiedDuration(leg, y,
                                 includeSettlementDateFlows,
                                 settlementDate, npvDate);
        }

        struct CashFlowLater {
            bool operator()(const ext::shared_ptr<CashFlow> &c,
                            const ext::shared_ptr<CashFlow> &d) {
                return c->date() > d->date();
            }
        };

    } // anonymous namespace ends here

    CashFlows::IrrFinder::IrrFinder(const Leg& leg,
                                    Real npv,
                                    DayCounter dayCounter,
                                    Compounding comp,
                                    Frequency freq,
                                    const ext::optional<bool>& includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate)
    : leg_(leg), npv_(npv), dayCounter_(std::move(dayCounter)), compounding_(comp),
      frequency_(freq), includeSettlementDateFlows_(includeSettlementDateFlows),
      settlementDate_(settlementDate), npvDate_(npvDate) {

        if (settlementDate_ == Date())
            settlementDate_ = Settings::instance().evaluationDate();

        if (npvDate_ == Date())
            npvDate_ = settlementDate_;

        checkSign();
    }

    Real CashFlows::IrrFinder::operator()(Rate y) const {
        InterestRate yield(y, dayCounter_, compounding_, frequency_);
        Real NPV = CashFlows::npv(leg_, yield,
                                  includeSettlementDateFlows_,
                                  settlementDate_, npvDate_);
        return npv_ - NPV;
    }

    Real CashFlows::IrrFinder::derivative(Rate y) const {
        InterestRate yield(y, dayCounter_, compounding_, frequency_);
        return modifiedDuration(leg_, yield,
                                includeSettlementDateFlows_,
                                settlementDate_, npvDate_);
    }

    void CashFlows::IrrFinder::checkSign() const {
        // depending on the sign of the market price, check that cash
        // flows of the opposite sign have been specified (otherwise
        // IRR is nonsensical.)

        Integer lastSign = sign(Real(-npv_)),
                signChanges = 0;
        for (const auto& i : leg_) {
            if (!i->hasOccurred(settlementDate_, includeSettlementDateFlows_) &&
                !i->tradingExCoupon(settlementDate_)) {
                Integer thisSign = sign(i->amount());
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

    Real CashFlows::npv(const Leg& leg,
                        const InterestRate& y,
                        const ext::optional<bool>& includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

#if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(std::adjacent_find(leg.begin(), leg.end(),
                                      CashFlowLater()) == leg.end(),
                   "cashflows must be sorted in ascending order w.r.t. their payment dates");
#endif

        Real npv = 0.0;
        DiscountFactor discount = 1.0;
        Date lastDate = npvDate;
        const DayCounter& dc = y.dayCounter();
        for (const auto& i : leg) {
            if (i->hasOccurred(settlementDate, includeSettlementDateFlows))
                continue;

            Real amount = i->amount();
            if (i->tradingExCoupon(settlementDate)) {
                amount = 0.0;
            }

            DiscountFactor b = y.discountFactor(getStepwiseDiscountTime(i, dc, npvDate, lastDate));
            discount *= b;
            lastDate = i->date();

            npv += amount * discount;
        }

        return npv;
    }

    Real CashFlows::npv(const Leg& leg,
                        Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        const ext::optional<bool>& includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        return npv(leg, InterestRate(yield, dc, comp, freq),
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    Real CashFlows::bps(const Leg& leg,
                        const InterestRate& yield,
                        const ext::optional<bool>& includeSettlementDateFlows,
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
                        const ext::optional<bool>& includeSettlementDateFlows,
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
                          const ext::optional<bool>& includeSettlementDateFlows,
                          Date settlementDate,
                          Date npvDate,
                          Real accuracy,
                          Size maxIterations,
                          Rate guess) {
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        return CashFlows::yield<NewtonSafe>(solver, leg, npv, dayCounter,
                                            compounding, frequency,
                                            includeSettlementDateFlows,
                                            settlementDate, npvDate,
                                            accuracy, guess);
    }


    Time CashFlows::duration(const Leg& leg,
                             const InterestRate& rate,
                             Duration::Type type,
                             const ext::optional<bool>& includeSettlementDateFlows,
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
                             const ext::optional<bool>& includeSettlementDateFlows,
                             Date settlementDate,
                             Date npvDate) {
        return duration(leg, InterestRate(yield, dc, comp, freq),
                        type,
                        includeSettlementDateFlows,
                        settlementDate, npvDate);
    }

    Real CashFlows::convexity(const Leg& leg,
                              const InterestRate& y,
                              const ext::optional<bool>& includeSettlementDateFlows,
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
        Time t = 0.0;
        Real d2Pdy2 = 0.0;
        Rate r = y.rate();
        Natural N = y.frequency();
        Date lastDate = npvDate;
        for (const auto& i : leg) {
            if (i->hasOccurred(settlementDate, includeSettlementDateFlows))
                continue;

            Real c = i->amount();
            if (i->tradingExCoupon(settlementDate)) {
                c = 0.0;
            }

            t += getStepwiseDiscountTime(i, dc, npvDate, lastDate);
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
              case CompoundedThenSimple:
                if (t>1.0/N)
                    d2Pdy2 += c * 2.0*B*B*B*t*t;
                else
                    d2Pdy2 += c * B*t*(N*t+1)/(N*(1+r/N)*(1+r/N));
                break;
              default:
                QL_FAIL("unknown compounding convention (" <<
                        Integer(y.compounding()) << ")");
            }
            lastDate = i->date();
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
                              const ext::optional<bool>& includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
        return convexity(leg, InterestRate(yield, dc, comp, freq),
                         includeSettlementDateFlows,
                         settlementDate, npvDate);
    }

    Real CashFlows::basisPointValue(const Leg& leg,
                                    const InterestRate& y,
                                    const ext::optional<bool>& includeSettlementDateFlows,
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
                                    const ext::optional<bool>& includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate) {
        return basisPointValue(leg, InterestRate(yield, dc, comp, freq),
                               includeSettlementDateFlows,
                               settlementDate, npvDate);
    }

    Real CashFlows::yieldValueBasisPoint(const Leg& leg,
                                         const InterestRate& y,
                                         const ext::optional<bool>& includeSettlementDateFlows,
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
                                         const ext::optional<bool>& includeSettlementDateFlows,
                                         Date settlementDate,
                                         Date npvDate) {
        return yieldValueBasisPoint(leg, InterestRate(yield, dc, comp, freq),
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
    }

    // Z-spread utility functions
    Real CashFlows::npv(const Leg& leg,
                        const ext::shared_ptr<YieldTermStructure>& discountCurve,
                        Spread zSpread,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        const ext::optional<bool>& includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(ext::shared_ptr<Quote>(new
            SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                comp, freq);

        return npv(leg, spreadedCurve,
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    Spread CashFlows::zSpread(const Leg& leg,
                              Real npv,
                              const ext::shared_ptr<YieldTermStructure>& discount,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              const ext::optional<bool>& includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        auto zSpreadQuote = ext::make_shared<SimpleQuote>();
        ZeroSpreadedTermStructure spreadedCurve(Handle<YieldTermStructure>(discount),
                                                Handle<Quote>(zSpreadQuote),
                                                compounding,
                                                frequency);
        auto objFunction = [&](Rate zSpread) {
            zSpreadQuote->setValue(zSpread);
            Real NPV = CashFlows::npv(leg, spreadedCurve,
                                      includeSettlementDateFlows,
                                      settlementDate, npvDate);
            return npv - NPV;
        };

        Brent solver;
        solver.setMaxEvaluations(maxIterations);
        Real step = 0.01;
        return solver.solve(objFunction, accuracy, guess, step);
    }

}
