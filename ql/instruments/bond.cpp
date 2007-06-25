/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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
#include <ql/termstructures/yieldcurves/zerospreadedtermstructure.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    namespace {

        Real dirtyPriceFromYield(
                   Real faceAmount,
                   const Leg& cashflows,
                   Rate yield,
                   Compounding compounding,
                   Frequency frequency,
                   const DayCounter& dayCounter,
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

            const boost::shared_ptr<CashFlow>& redemption = cashflows.back();
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
                   Compounding compounding,
                   const DayCounter& dayCounter,
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
                                                         compounding_,
                                                         frequency_,
                                                         dayCounter_,
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
                            Compounding comp,
                            Frequency freq,
                            const DayCounter& dc,
                            const Date& settlement,
                            const Handle<YieldTermStructure>& discountCurve) {

            QL_REQUIRE(freq != NoFrequency && freq != Once,
                       "invalid frequency:" << freq);

            Handle<Quote> zSpreadQuoteHandle(boost::shared_ptr<Quote>(new
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
               Real faceAmount,
               const Calendar& calendar,
               const DayCounter& paymentDayCounter,
               BusinessDayConvention paymentConvention,
               const Handle<YieldTermStructure>& discountCurve)
    : settlementDays_(settlementDays),  faceAmount_(faceAmount),
      calendar_(calendar), paymentDayCounter_(paymentDayCounter),
      paymentConvention_(paymentConvention), discountCurve_(discountCurve),
      frequency_(NoFrequency) {
        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve_);
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
        calculate();
        // !!!
        return NPV_/faceAmount_*100.0;
    }

    Rate Bond::yield(Compounding compounding,
                     Real accuracy, Size maxEvaluations) const {
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        YieldFinder objective(faceAmount_, cashflows_, dirtyPrice(),
                              compounding, paymentDayCounter_, frequency_,
                              settlementDate());
        return solver.solve(objective, accuracy, 0.02, 0.0, 1.0);
    }

    Real Bond::cleanPrice(Rate yield, Compounding compounding,
                          Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return dirtyPrice(yield,compounding,settlement)
             - accruedAmount(settlement);
    }

    Real Bond::dirtyPrice(Rate yield, Compounding compounding,
                          Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return dirtyPriceFromYield(faceAmount_, cashflows_, yield,
                                   compounding, frequency_, paymentDayCounter_,
                                   settlement);
    }

    Rate Bond::yield(Real cleanPrice, Compounding compounding,
                     Date settlement,
                     Real accuracy, Size maxEvaluations) const {
        if (settlement == Date())
            settlement = settlementDate();
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        Real dirtyPrice = cleanPrice + accruedAmount(settlement);
        YieldFinder objective(faceAmount_, cashflows_, dirtyPrice,
                              compounding, paymentDayCounter_, frequency_,
                              settlement);
        return solver.solve(objective, accuracy, 0.02, 0.0, 1.0);
    }

    Real Bond::cleanPriceFromZSpread(Spread zSpread,
                                     Compounding comp,
                                     Frequency freq,
                                     DayCounter dc,
                                     Date settlement) const {
        Real p = dirtyPriceFromZSpread(zSpread, comp, freq, dc, settlement);
        return p - accruedAmount(settlement);
    }

    Real Bond::dirtyPriceFromZSpread(Spread zSpread,
                                     Compounding comp,
                                     Frequency freq,
                                     DayCounter dc,
                                     Date settlement) const {
         if (settlement == Date())
             settlement = settlementDate();
         return dirtyPriceFromZSpreadFunction(faceAmount_, cashflows_,
                                              zSpread, comp, freq, dc,
                                              settlement, discountCurve_);
    }

    Real Bond::accruedAmount(Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();

        for (Size i = 0; i<cashflows_.size(); ++i) {
            // the first coupon paying after d is the one we're after
            if (!cashflows_[i]->hasOccurred(settlement)) {
                boost::shared_ptr<Coupon> coupon =
                    boost::dynamic_pointer_cast<Coupon>(cashflows_[i]);
                if (coupon)
                    // !!!
                    return coupon->accruedAmount(settlement)/faceAmount_*100.0;
                else
                    return 0.0;
            }
        }
        return 0.0;
    }

    bool Bond::isExpired() const {
        return cashflows_.back()->hasOccurred(settlementDate());
    }

    Real Bond::previousCoupon(Date settlement) const {
        return CashFlows::previousCouponRate(cashflows_, settlement);
    }

    Real Bond::currentCoupon(Date settlement) const {
        return CashFlows::currentCouponRate(cashflows_, settlement);
    }

    void Bond::performCalculations() const {

        if (engine_) {
            Instrument::performCalculations();
        } else {
            QL_REQUIRE(!discountCurve_.empty(),
                       "no discounting term structure set");
            NPV_= CashFlows::npv(cashflows_, discountCurve_, settlementDate(),
                                                             settlementDate());
        }
    }


    void Bond::setupArguments(PricingEngine::arguments* args) const {
        Bond::arguments* arguments = dynamic_cast<Bond::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->settlementDate = settlementDate();
        arguments->cashflows = cashflows_;
        arguments->calendar = calendar_;
        arguments->paymentConvention = paymentConvention_;
        arguments->paymentDayCounter = paymentDayCounter_;
        arguments->frequency = frequency_;
    }

    void Bond::arguments::validate() const {
        QL_REQUIRE(settlementDate != Date(), "no settlement date provided");
        for (Size i=0; i<cashflows.size(); ++i)
            QL_REQUIRE(cashflows[i], "null coupon provided");
    }

}

