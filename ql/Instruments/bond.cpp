/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/bond.hpp>
#include <ql/settings.hpp>
#include <ql/CashFlows/coupon.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace {

        Real dirtyPriceFromYield(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const boost::shared_ptr<CashFlow>& redemption,
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

            for (Size i=0; i<cashflows.size(); ++i) {
                // discard expired coupons
                #if QL_TODAYS_PAYMENTS
                if (cashflows[i]->date() < settlement)
                #else
                if (cashflows[i]->date() <= settlement)
                #endif
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

            #if QL_TODAYS_PAYMENTS
            if (redemption->date() >= settlement) {
            #else
            if (redemption->date() > settlement) {
            #endif
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

            return price;
        }

        class YieldFinder {
          public:
            YieldFinder(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   const boost::shared_ptr<CashFlow>& redemption,
                   Real dirtyPrice,
                   Compounding compounding,
                   const DayCounter& dayCounter,
                   Frequency frequency,
                   const Date& settlement)
            : cashflows_(cashflows), redemption_(redemption),
              dirtyPrice_(dirtyPrice), compounding_(compounding),
              dayCounter_(dayCounter), frequency_(frequency),
              settlement_(settlement) {}
            Real operator()(Real yield) const {
                return dirtyPrice_ - dirtyPriceFromYield(cashflows_,
                                                         redemption_,
                                                         yield,
                                                         compounding_,
                                                         frequency_,
                                                         dayCounter_,
                                                         settlement_);
            }
          private:
            std::vector<boost::shared_ptr<CashFlow> > cashflows_;
            boost::shared_ptr<CashFlow> redemption_;
            Real dirtyPrice_;
            Compounding compounding_;
            DayCounter dayCounter_;
            Frequency frequency_;
            Date settlement_;
        };

    }


    Bond::Bond(const DayCounter& dayCount, const Calendar& calendar,
               BusinessDayConvention businessDayConvention,
               Integer settlementDays,
               const Handle<YieldTermStructure>& discountCurve)
    : settlementDays_(settlementDays), calendar_(calendar),
      businessDayConvention_(businessDayConvention), dayCount_(dayCount),
      frequency_(NoFrequency), discountCurve_(discountCurve) {
        registerWith(Settings::instance().evaluationDateGuard());
        registerWith(discountCurve_);
    }

    Date Bond::settlementDate() const {
        // usually, the settlement is at T+n...
        Date d = calendar_.advance(Settings::instance().evaluationDate(),
                                   settlementDays_, Days);
        // ...but the bond won't be traded until the issue date.
        return std::max(d, issueDate_);
    }

    Real Bond::cleanPrice() const {
        return dirtyPrice() - accruedAmount(settlementDate());
    }

    Real Bond::dirtyPrice() const {
        calculate();
        return NPV_;
    }

    Real Bond::yield(Compounding compounding,
                     Real accuracy, Size maxEvaluations) const {
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        YieldFinder objective(cashFlows_, redemption_, dirtyPrice(),
                              compounding, dayCount_, frequency_,
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
        return dirtyPriceFromYield(cashFlows_, redemption_, yield,
                                   compounding, frequency_, dayCount_,
                                   settlement);
    }

    Real Bond::yield(Real cleanPrice, Compounding compounding,
                     Date settlement,
                     Real accuracy, Size maxEvaluations) const {
        if (settlement == Date())
            settlement = settlementDate();
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        Real dirtyPrice = cleanPrice + accruedAmount(settlement);
        YieldFinder objective(cashFlows_, redemption_, dirtyPrice,
                              compounding, dayCount_, frequency_, settlement);
        return solver.solve(objective, accuracy, 0.02, 0.0, 1.0);
    }

    Real Bond::accruedAmount(Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();

        for (Size i = 0; i < cashFlows_.size(); ++i) {
            // the first coupon paying after d is the one we're after
            #if QL_TODAYS_PAYMENTS
            if (cashFlows_[i]->date() >= settlement) {
            #else
            if (cashFlows_[i]->date() > settlement) {
            #endif
                boost::shared_ptr<Coupon> coupon =
                    boost::dynamic_pointer_cast<Coupon>(cashFlows_[i]);
                if (coupon)
                    return coupon->accruedAmount(settlement);
                else
                    return 0.0;
            }
        }
        return 0.0;
    }

    bool Bond::isExpired() const {
        #if QL_TODAYS_PAYMENTS
        return maturityDate_ < settlementDate();
        #else
        return maturityDate_ <= settlementDate();
        #endif
    }

    void Bond::performCalculations() const {

        QL_REQUIRE(!discountCurve_.empty(), "no term structure set");

        Date settlement = settlementDate();
		NPV_ = 0.0;

        // add the discounted cash flows including redemption
        for (Size i=0; i<cashFlows_.size(); i++) {

            Date d = cashFlows_[i]->date();

            #if QL_TODAYS_PAYMENTS
            if (d >= settlement) {
            #else
            if (d > settlement) {
            #endif
                NPV_ += cashFlows_[i]->amount() * discountCurve_->discount(d);
            }
        }
        NPV_ += redemption_->amount() *
                discountCurve_->discount(redemption_->date());

        // adjust to bond settlement
        NPV_ /= discountCurve_->discount(settlement);
    }

}
