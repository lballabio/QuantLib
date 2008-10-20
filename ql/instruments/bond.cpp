/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007, 2008 Ferdinando Ametrano
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

#include <ql/instruments/bond.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/comparison.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <ql/settings.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    namespace {

        Real dirtyPriceFromYield(Real faceAmount,
                                 const Leg& cashflows,
                                 Rate yield,
                                 const DayCounter& dayCounter,
                                 Compounding compounding,
                                 Frequency frequency,
                                 const Date& settlement) {
            if (frequency == NoFrequency || frequency == Once)
                frequency = Annual;

            InterestRate y(yield, dayCounter, compounding, frequency);

            Real price = 0.0;
            DiscountFactor discount = 1.0;
            Date lastDate = Date();

            for (Size i=0; i<cashflows.size(); ++i) {
                if (cashflows[i]->hasOccurred(settlement))
                    continue;

                Date couponDate = cashflows[i]->date();
                Real amount = cashflows[i]->amount();
                if (lastDate == Date()) {
                    // first not-expired coupon
                    if (i > 0) {
                        lastDate = cashflows[i-1]->date();
                    } else {
                        boost::shared_ptr<Coupon> coupon =
                            boost::dynamic_pointer_cast<Coupon>(cashflows[i]);
                        if (coupon)
                            lastDate = coupon->accrualStartDate();
                        else
                            lastDate = couponDate - 1*Years;
                    }
                    discount *= y.discountFactor(settlement,couponDate,
                                                 lastDate, couponDate);
                } else  {
                    discount *= y.discountFactor(lastDate, couponDate);
                }
                lastDate = couponDate;

                price += amount * discount;
            }

            return price/faceAmount*100.0;
        }


        class YieldFinder {
          public:
            YieldFinder(
                   Real faceAmount,
                   const Leg& cashflows,
                   Real dirtyPrice,
                   const DayCounter& dayCounter,
                   Compounding compounding,
                   Frequency frequency,
                   const Date& settlement)
            : faceAmount_(faceAmount), cashflows_(cashflows),
              dirtyPrice_(dirtyPrice),compounding_(compounding),
              dayCounter_(dayCounter), frequency_(frequency),
              settlement_(settlement) {}
            Real operator()(Real yield) const {
                return dirtyPrice_ - dirtyPriceFromYield(faceAmount_,
                                                         cashflows_,
                                                         yield,
                                                         dayCounter_,
                                                         compounding_,
                                                         frequency_,
                                                         settlement_);
            }
          private:
            Real faceAmount_;
            Leg cashflows_;
            Real dirtyPrice_;
            Compounding compounding_;
            DayCounter dayCounter_;
            Frequency frequency_;
            Date settlement_;
        };


        Real dirtyPriceFromZSpreadFunction(
                            Real faceAmount,
                            const Leg& cashflows,
                            Spread zSpread,
                            const DayCounter& dc,
                            Compounding comp,
                            Frequency freq,
                            const Date& settlement,
                            const Handle<YieldTermStructure>& discountCurve) {

            QL_REQUIRE(freq != NoFrequency && freq != Once,
                       "invalid frequency:" << freq);

            Handle<Quote> zSpreadQuoteHandle(shared_ptr<Quote>(new
                SimpleQuote(zSpread)));

            ZeroSpreadedTermStructure spreadedCurve(discountCurve,
                                                    zSpreadQuoteHandle,
                                                    comp, freq, dc);
            Real price = 0.0;
            for (Size i=0; i<cashflows.size(); ++i) {
                if (cashflows[i]->hasOccurred(settlement))
                    continue;

                Date couponDate = cashflows[i]->date();
                Real amount = cashflows[i]->amount();
                price += amount * spreadedCurve.discount(couponDate);
            }
            price /= spreadedCurve.discount(settlement);
            return price/faceAmount*100.0;
        }

    } // anonymous namespace ends here



    Bond::Bond(Natural settlementDays,
               const Calendar& calendar,
               const Date& issueDate,
               const Leg& coupons)
      : settlementDays_(settlementDays), calendar_(calendar),
        cashflows_(coupons), issueDate_(issueDate) {

        if (!coupons.empty()) {
            std::sort(cashflows_.begin(), cashflows_.end(),
                      earlier_than<boost::shared_ptr<CashFlow> >());

            maturityDate_ = coupons.back()->date();

            addRedemptionsToCashflows();
        }

        registerWith(Settings::instance().evaluationDate());
    }

    Bond::Bond(Natural settlementDays,
               const Calendar& calendar,
               Real faceAmount,
               const Date& maturityDate,
               const Date& issueDate,
               const Leg& cashflows)
    : settlementDays_(settlementDays), calendar_(calendar),
      cashflows_(cashflows), maturityDate_(maturityDate),
      issueDate_(issueDate) {

        if (!cashflows.empty()) {

            notionals_.resize(2);
            notionalSchedule_.resize(2);

            notionalSchedule_[0] = Date();
            notionals_[0] = faceAmount;

            notionalSchedule_[1] = maturityDate;
            notionals_[1] = 0.0;

            redemptions_.push_back(cashflows.back());

            std::sort(cashflows_.begin(), --cashflows_.end(),
                      earlier_than<boost::shared_ptr<CashFlow> >());
        }

        registerWith(Settings::instance().evaluationDate());
    }

    Real Bond::notional(Date d) const {
        if (d == Date())
            d = settlementDate();

        if (d > notionalSchedule_.back()) {
            // after maturity
            return 0.0;
        }

        // After the check above, d is between the schedule
        // boundaries.  We search starting from the second notional
        // date, since the first is null.  After the call to
        // lower_bound, *i is the earliest date which is greater or
        // equal than d.  Its index is greater or equal to 1.
        std::vector<Date>::const_iterator i =
            std::lower_bound(++notionalSchedule_.begin(),
                             notionalSchedule_.end(), d);
        Size index = std::distance(notionalSchedule_.begin(), i);

        if (d < notionalSchedule_[index]) {
            // no doubt about what to return
            return notionals_[index-1];
        } else {
            // d is equal to a redemption date.
            #if defined(QL_TODAYS_PAYMENTS)
            // We consider today's payment as pending; the bond still
            // has the previous notional
            return notionals_[index-1];
            #else
            // today's payment has occurred; the bond already changed
            // notional.
            return notionals_[index];
            #endif
        }
    }

    const boost::shared_ptr<CashFlow>& Bond::redemption() const {
        QL_REQUIRE(redemptions_.size() == 1,
                   "multiple redemption cash flows given");
        return redemptions_.back();
    }

    Date Bond::settlementDate(const Date& date) const {
        Date d = (date==Date() ?
                  Settings::instance().evaluationDate() :
                  date);

        // usually, the settlement is at T+n...
        Date settlement = calendar_.advance(d, settlementDays_, Days);
        // ...but the bond won't be traded until the issue date (if given.)
        if (issueDate_ == Date())
            return settlement;
        else
            return std::max(settlement, issueDate_);
    }

    Real Bond::cleanPrice() const {
        return dirtyPrice() - accruedAmount(settlementDate());
    }

    Real Bond::dirtyPrice() const {
        return settlementValue()/notional(settlementDate())*100.0;
    }

    Real Bond::settlementValue() const {
        calculate();
        QL_REQUIRE(settlementValue_ != Null<Real>(),
                   "settlement value not provided");
        return settlementValue_;
    }

    Real Bond::settlementValue(Real cleanPrice) const {
        Real dirtyPrice = cleanPrice + accruedAmount(settlementDate());
        return dirtyPrice/100.0 * notional(settlementDate());
    }


    Rate Bond::yield(const DayCounter& dc,
                     Compounding comp,
                     Frequency freq,
                     Real accuracy,
                     Size maxEvaluations) const {
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        YieldFinder objective(notional(settlementDate()), cashflows_,
                              dirtyPrice(),
                              dc, comp, freq,
                              settlementDate());
        return solver.solve(objective, accuracy, 0.02, 0.0, 1.0);
    }

    Real Bond::cleanPrice(Rate yield,
                          const DayCounter& dc,
                          Compounding comp,
                          Frequency freq,
                          Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return dirtyPrice(yield, dc, comp, freq, settlement)
             - accruedAmount(settlement);
    }

    Real Bond::dirtyPrice(Rate yield,
                          const DayCounter& dc,
                          Compounding comp,
                          Frequency freq,
                          Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return dirtyPriceFromYield(notional(settlement), cashflows_, yield,
                                   dc, comp, freq,
                                   settlement);
    }

    Rate Bond::yield(Real cleanPrice,
                     const DayCounter& dc,
                     Compounding comp,
                     Frequency freq,
                     Date settlement,
                     Real accuracy,
                     Size maxEvaluations) const {
        if (settlement == Date())
            settlement = settlementDate();
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        Real dirtyPrice = cleanPrice + accruedAmount(settlement);
        YieldFinder objective(notional(settlement), cashflows_, dirtyPrice,
                              dc, comp, freq,
                              settlement);
        return solver.solve(objective, accuracy, 0.02, 0.0, 1.0);
    }

    Real Bond::cleanPriceFromZSpread(Spread zSpread,
                                     const DayCounter& dc,
                                     Compounding comp,
                                     Frequency freq,
                                     Date settlement) const {
        Real p = dirtyPriceFromZSpread(zSpread, dc, comp, freq, settlement);
        return p - accruedAmount(settlement);
    }

    Real Bond::dirtyPriceFromZSpread(Spread zSpread,
                                     const DayCounter& dc,
                                     Compounding comp,
                                     Frequency freq,
                                     Date settlement) const {
         if (settlement == Date())
             settlement = settlementDate();
         QL_REQUIRE(engine_, "null pricing engine");

         shared_ptr<DiscountingBondEngine> bondEngine =
             dynamic_pointer_cast<DiscountingBondEngine>(engine_);
         QL_REQUIRE(bondEngine, "engine not compatible with calculation");

         return dirtyPriceFromZSpreadFunction(notional(settlement), cashflows_,
                                              zSpread, dc, comp, freq,
                                              settlement,
                                              bondEngine->discountCurve());
    }

    Real Bond::accruedAmount(Date settlement) const {
        if (settlement==Date())
            settlement = settlementDate();

        Leg::const_iterator cf =
            CashFlows::nextCashFlow(cashflows_, settlement);
        if (cf==cashflows_.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        bool firstCouponFound = false;
        Real nominal = Null<Real>();
        Time accrualPeriod = Null<Time>();
        DayCounter dc;
        Rate result = 0.0;
        for (; cf<cashflows_.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp) {
                if (firstCouponFound) {
                    QL_REQUIRE(nominal       == cp->nominal() &&
                               accrualPeriod == cp->accrualPeriod() &&
                               dc            == cp->dayCounter(),
                               "cannot aggregate accrued amount of two "
                               "different coupons on " << paymentDate);
                } else {
                    firstCouponFound = true;
                    nominal = cp->nominal();
                    accrualPeriod = cp->accrualPeriod();
                    dc = cp->dayCounter();
                }
                result += cp->accruedAmount(settlement);
            }
        }
        return result/notional(settlement)*100.0;
    }

    bool Bond::isExpired() const {
        return cashflows_.back()->hasOccurred(settlementDate());
    }

    Rate Bond::nextCoupon(Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return CashFlows::nextCouponRate(cashflows_, settlement);
    }

    Rate Bond::previousCoupon(Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return CashFlows::previousCouponRate(cashflows_, settlement);
    }

    void Bond::setupExpired() const {
        Instrument::setupExpired();
        settlementValue_ = 0.0;
    }

    void Bond::setupArguments(PricingEngine::arguments* args) const {
        Bond::arguments* arguments = dynamic_cast<Bond::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->settlementDate = settlementDate();
        arguments->cashflows = cashflows_;
        arguments->calendar = calendar_;
    }

    void Bond::fetchResults(const PricingEngine::results* r) const {

        Instrument::fetchResults(r);

        const Bond::results* results =
            dynamic_cast<const Bond::results*>(r);
        QL_ENSURE(results != 0, "wrong result type");

        settlementValue_ = results->settlementValue;
    }

    void Bond::addRedemptionsToCashflows(const std::vector<Real>& redemptions) {
        // First, we gather the notional information from the cashflows
        calculateNotionalsFromCashflows();
        // Then, we create the redemptions based on the notional
        // information and we add them to the cashflows vector after
        // the coupons.
        redemptions_.clear();
        for (Size i=1; i<notionalSchedule_.size(); ++i) {
            Real R = i < redemptions.size() ? redemptions[i] :
                     !redemptions.empty()   ? redemptions.back() :
                                              100.0;
            Real amount = (R/100.0)*(notionals_[i-1]-notionals_[i]);
            boost::shared_ptr<CashFlow> redemption(
                            new SimpleCashFlow(amount, notionalSchedule_[i]));
            cashflows_.push_back(redemption);
            redemptions_.push_back(redemption);
        }
        // stable_sort now moves the redemptions to the right places
        // while ensuring that they follow coupons with the same date.
        std::stable_sort(cashflows_.begin(), cashflows_.end(),
                         earlier_than<boost::shared_ptr<CashFlow> >());
    }

    void Bond::setSingleRedemption(Real notional,
                                   Real redemption,
                                   const Date& date) {
        notionals_.resize(2);
        notionalSchedule_.resize(2);
        redemptions_.clear();

        notionalSchedule_[0] = Date();
        notionals_[0] = notional;

        notionalSchedule_[1] = date;
        notionals_[1] = 0.0;

        boost::shared_ptr<CashFlow> redemptionCashflow(
                         new SimpleCashFlow(notional*redemption/100.0, date));
        cashflows_.push_back(redemptionCashflow);
        redemptions_.push_back(redemptionCashflow);
    }


    void Bond::calculateNotionalsFromCashflows() {
        notionalSchedule_.clear();
        notionals_.clear();

        Date lastPaymentDate = Date();
        notionalSchedule_.push_back(Date());
        for (Size i=0; i<cashflows_.size(); ++i) {
            boost::shared_ptr<Coupon> coupon =
                boost::dynamic_pointer_cast<Coupon>(cashflows_[i]);
            if (!coupon)
                continue;

            Real notional = coupon->nominal();
            // we add the notional only if it is the first one...
            if (notionals_.empty()) {
                notionals_.push_back(coupon->nominal());
                lastPaymentDate = coupon->date();
            } else if (!close(notional, notionals_.back())) {
                // ...or if it has changed.
                QL_REQUIRE(notional > notionals_.back(),
                           "increasing coupon notionals");
                notionals_.push_back(coupon->nominal());
                // in this case, we also add the last valid date for
                // the previous one...
                notionalSchedule_.push_back(lastPaymentDate);
                // ...and store the candidate for this one.
                lastPaymentDate = coupon->date();
            } else {
                // otherwise, we just extend the valid range of dates
                // for the current notional.
                lastPaymentDate = coupon->date();
            }
        }
        QL_REQUIRE(!notionals_.empty(), "no coupons provided");
        notionals_.push_back(0.0);
        notionalSchedule_.push_back(lastPaymentDate);
    }


    void Bond::arguments::validate() const {
        QL_REQUIRE(settlementDate != Date(), "no settlement date provided");
        QL_REQUIRE(!cashflows.empty(), "no cash flow provided");
        for (Size i=0; i<cashflows.size(); ++i)
            QL_REQUIRE(cashflows[i], "null cash flow provided");
    }

}
