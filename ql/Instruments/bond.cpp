/*
 Copyright (C) 2004 Jeff Yu
 Copyright (C) 2004 M-Dimension Consulting Inc.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/bond.hpp>
#include <ql/settings.hpp>
#include <ql/CashFlows/coupon.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace {

        Real dirtyPriceFromYield(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   Real redemption,
                   Rate yield,
                   const DayCounter& dayCounter,
                   const Date& settlement) {

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

                Date nextDate = cashflows[i]->date();
                Real amount = cashflows[i]->amount();
                if (i == cashflows.size()-1)
                    amount += redemption;

                Time t1, t2;
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
                            lastDate = nextDate - 1*Years;
                    }
                    t1 = dayCounter.yearFraction(settlement, nextDate,
                                                 lastDate, nextDate);
                    t2 = dayCounter.yearFraction(lastDate, nextDate);
                } else  {
                    t1 = dayCounter.yearFraction(lastDate, nextDate);
                    t2 = t1;
                }

                discount /= QL_POW(1 + yield*t2, t1/t2);
                price += amount * discount;
                lastDate = nextDate;
            }

            return price;
        }

        class YieldFinder {
          public:
            YieldFinder(
                   const std::vector<boost::shared_ptr<CashFlow> >& cashflows,
                   Real redemption,
                   Real dirtyPrice,
                   const DayCounter& dayCounter,
                   const Date& settlement)
            : cashflows_(cashflows), redemption_(redemption),
              dirtyPrice_(dirtyPrice), dayCounter_(dayCounter),
              settlement_(settlement) {}
            Real operator()(Real yield) const {
                return dirtyPrice_ - dirtyPriceFromYield(cashflows_,
                                                         redemption_,
                                                         yield,
                                                         dayCounter_,
                                                         settlement_);
            }
          private:
            std::vector<boost::shared_ptr<CashFlow> > cashflows_;
            Real redemption_, dirtyPrice_;
            DayCounter dayCounter_;
            Date settlement_;
        };

    }


    Bond::Bond(const DayCounter& dayCount, const Calendar& calendar,
               Integer settlementDays)
    : settlementDays_(settlementDays), calendar_(calendar),
      dayCount_(dayCount) {
        registerWith(Settings::instance().evaluationDateGuard());
    }

    Date Bond::settlementDate() const {
        // usually, the settlement is at T+n...
        Date d = calendar_.advance(Settings::instance().evaluationDate(),
                                   settlementDays_, Days);
        // ...but the bond won't be traded until the issue date.
        return QL_MAX(d, issueDate_);
    }

    Real Bond::cleanPrice(Rate yield, Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return dirtyPrice(yield,settlement) - accruedAmount(settlement);
    }

    Real Bond::dirtyPrice(Rate yield, Date settlement) const {
        if (settlement == Date())
            settlement = settlementDate();
        return dirtyPriceFromYield(cashFlows_, redemption_, yield,
                                   dayCount_, settlement);
    }

    Real Bond::yield(Real cleanPrice, Date settlement,
                     Real accuracy, Size maxEvaluations) const {
        if (settlement == Date())
            settlement = settlementDate();
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        Real dirtyPrice = cleanPrice + accruedAmount(settlement);
        YieldFinder objective(cashFlows_, redemption_, dirtyPrice,
                              dayCount_, settlement);
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
        QL_FAIL("calculation of the theoretical price is not yet implemented");
    }

}
