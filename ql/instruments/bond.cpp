/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005, 2006, 2007 StatPro Italia srl

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
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <ql/settings.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    namespace {

        Real dirtyPriceFromYield(
                   Real faceAmount,
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

            for (Size i=0; i<cashflows.size()-1; ++i) {
                if (cashflows[i]->hasOccurred(settlement))
                    continue;

                Date couponDate = cashflows[i]->date();
                Real amount = cashflows[i]->amount();
                if (lastDate == Date()) {
                    // first not-expired coupon
                    if (i > 0) {
                        lastDate = cashflows[i-1]->date();
                    } else {
                        shared_ptr<Coupon> coupon =
                            dynamic_pointer_cast<Coupon>(cashflows[i]);
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

            const shared_ptr<CashFlow>& redemption = cashflows.back();
            if (!redemption->hasOccurred(settlement)) {
                Date redemptionDate = redemption->date();
                Real amount = redemption->amount();
                if (lastDate == Date()) {
                    // no coupons
                    lastDate = redemptionDate - 1*Years;
                    discount *= y.discountFactor(settlement,redemptionDate,
                                                 lastDate, redemptionDate);
                } else {
                    discount *= y.discountFactor(lastDate, redemptionDate);
                }

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
              dirtyPrice_(dirtyPrice),
              compounding_(compounding), dayCounter_(dayCounter),
              frequency_(frequency), settlement_(settlement) {}
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

    //    class ZspreadFinder {
    //      public:
    //        ZspreadFinder(
    //               Real faceAmount,
    //               const Leg& cashflows,
    //               Real dirtyPrice,
    //               Compounding compounding,
    //               const DayCounter& dayCounter,
    //               Frequency frequency,
    //               const Date& settlement,
    //               const Handle<YieldTermStructure>& discountCurve)
    //        : faceAmount_(faceAmount), cashflows_(cashflows),
    //          dirtyPrice_(dirtyPrice),
    //          compounding_(compounding), dayCounter_(dayCounter),
    //          frequency_(frequency), settlement_(settlement),discountCurve_(discountCurve) {}
    //        Real operator()(Spread zSpread) const {
    //            return dirtyPrice_ - dirtyPriceFromZSpread(faceAmount_,
    //                                                       cashflows_,
    //                                                       zSpread,
    //                                                       compounding_,
    //                                                       frequency_,
    //                                                       dayCounter_,
    //                                                       settlement_,
    //                                                       discountCurve_);
    //        }
    //      private:
    //        Real faceAmount_;
    //        Leg cashflows_;
    //        Real dirtyPrice_;
    //        Compounding compounding_;
    //        DayCounter dayCounter_;
    //        Frequency frequency_;
    //        Date settlement_;
    //        Handle<YieldTermStructure> discountCurve_;
    //         };

    } // anonymous namespace ends here

    Bond::Bond(Natural settlementDays,
               const Calendar& calendar,
               Real faceAmount,
               const Date& maturityDate,
               const Date& issueDate,
               const Leg& cashflows)
    : settlementDays_(settlementDays), calendar_(calendar),
      faceAmount_(faceAmount), cashflows_(cashflows),
      maturityDate_(maturityDate), issueDate_(issueDate) {
        registerWith(Settings::instance().evaluationDate());
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
        return NPV()/faceAmount_*100.0;
    }

    Rate Bond::yield(const DayCounter& dc,
                     Compounding comp,
                     Frequency freq,
                     Real accuracy,
                     Size maxEvaluations) const {
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        YieldFinder objective(faceAmount_, cashflows_, dirtyPrice(),
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
        return dirtyPriceFromYield(faceAmount_, cashflows_, yield,
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
        YieldFinder objective(faceAmount_, cashflows_, dirtyPrice,
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

         return dirtyPriceFromZSpreadFunction(faceAmount_, cashflows_,
                                              zSpread, dc, comp, freq,
                                              settlement, bondEngine->discountCurve());
    }

    Real Bond::accruedAmount(Date settlement) const {
        if (settlement==Date())
            settlement = settlementDate();

        Leg::const_iterator cf = CashFlows::nextCashFlow(cashflows_, settlement);
        if (cf==cashflows_.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        bool firstCouponFound = false;
        Real nominal;
        Time accrualPeriod;
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
        // accruedAmount cannot throw, must return zero
        // for bond algebra to work
        //QL_ENSURE(firstCouponFound,
        //          "next cashflow (" << paymentDate << ") is not a coupon");
        return result/faceAmount_*100.0;
    }

    bool Bond::isExpired() const {
        return cashflows_.back()->hasOccurred(settlementDate());
    }

    Rate Bond::previousCoupon(Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return CashFlows::previousCouponRate(cashflows_, settlement);
    }

    Rate Bond::nextCoupon(Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return CashFlows::nextCouponRate(cashflows_, settlement);
    }

    void Bond::setupArguments(PricingEngine::arguments* args) const {
        Bond::arguments* arguments = dynamic_cast<Bond::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->settlementDate = settlementDate();
        arguments->cashflows = cashflows_;
        arguments->calendar = calendar_;
    }

    void Bond::arguments::validate() const {
        QL_REQUIRE(settlementDate != Date(), "no settlement date provided");
        for (Size i=0; i<cashflows.size(); ++i)
            QL_REQUIRE(cashflows[i], "null coupon provided");
    }

}
