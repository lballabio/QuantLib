/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2013 Peter Caspers

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

#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/swaption/gaussian1djamshidianswaptionengine.hpp>
#include <utility>

namespace QuantLib {

    class Gaussian1dJamshidianSwaptionEngine::rStarFinder {
      public:
        rStarFinder(const ext::shared_ptr<Gaussian1dModel>& model,
                    Real nominal,
                    const Date& maturityDate,
                    const Date& valueDate,
                    std::vector<Date> fixedPayDates,
                    const std::vector<Real>& amounts,
                    const Size startIndex)
        : strike_(nominal), maturityDate_(maturityDate), valueDate_(valueDate),
          startIndex_(startIndex), times_(std::move(fixedPayDates)), amounts_(amounts),
          model_(model) {}

        Real operator()(Rate y) const {
            Real value = strike_;
            Size size = times_.size();
            for (Size i = startIndex_; i < size; i++) {
                Real dbValue = model_->zerobond(times_[i], maturityDate_, y) /
                               model_->zerobond(valueDate_, maturityDate_, y);
                value -= amounts_[i] * dbValue;
            }
            return value;
        }

      private:
        Real strike_;
        Date maturityDate_, valueDate_;
        Size startIndex_;
        std::vector<Date> times_;
        const std::vector<Real> &amounts_;
        const ext::shared_ptr<Gaussian1dModel> &model_;
    };

    void Gaussian1dJamshidianSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "Gaussian1dJamshidianSwaptionEngine");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "cannot use the Jamshidian decomposition "
                   "on exotic swaptions");

        QL_REQUIRE(arguments_.swap->spread() == 0.0,
                   "non zero spread (" << arguments_.swap->spread()
                                       << ") not allowed"); // PC

        QL_REQUIRE(arguments_.nominal != Null<Real>(),
                   "non-constant nominals are not supported yet");

        Date referenceDate;
        DayCounter dayCounter;

        referenceDate = model_->termStructure()->referenceDate();
        dayCounter = model_->termStructure()->dayCounter();

        Size startIndex = std::upper_bound(arguments_.fixedResetDates.begin(),
                                           arguments_.fixedResetDates.end(),
                                           arguments_.exercise->date(0) - 1) -
                          arguments_.fixedResetDates.begin();
        // only consider coupons with start date >= exercise dates

        // handle SOFR with a payment lag
        std::vector<Real> amounts(arguments_.fixedCoupons);
        std::vector<Date> payDates(arguments_.fixedPayDates);
        Date floatingAccrualEnd =
            arguments_.swap->floatingSchedule().dates().back();
        amounts.push_back(arguments_.nominal);
        payDates.push_back(floatingAccrualEnd);

        Real parNominal = arguments_.nominal;
        {
            Handle<YieldTermStructure> curve = model_->termStructure();
            const std::vector<Date>& floatDates =
                arguments_.swap->floatingSchedule().dates();
            DiscountFactor dfValue =
                curve->discount(arguments_.fixedResetDates[startIndex]);
            for (Size j = 0; j + 1 < floatDates.size() &&
                             j < arguments_.floatingPayDates.size(); ++j) {
                if (floatDates[j] < arguments_.fixedResetDates[startIndex])
                    continue;
                Date accrualEnd = floatDates[j + 1];
                if (arguments_.floatingPayDates[j] > accrualEnd) {
                    DiscountFactor dfStart = curve->discount(floatDates[j]);
                    DiscountFactor dfEnd = curve->discount(accrualEnd);
                    DiscountFactor dfPay =
                        curve->discount(arguments_.floatingPayDates[j]);
                    // Freeze the time-zero lag discount ratio and absorb the
                    // resulting floating-leg PV shortfall into the effective
                    // notional exchanged at the swap value date.
                    parNominal -= arguments_.nominal * (dfStart - dfEnd) *
                                  (1.0 - dfPay / dfEnd) / dfValue;
                }
            }
        }

        rStarFinder finder(*model_, parNominal,
                           arguments_.exercise->date(0),
                           arguments_.fixedResetDates[startIndex],
                           payDates, amounts, startIndex);
        Brent s1d;
        Rate minStrike = -8.0;
        Rate maxStrike = 8.0;
        s1d.setMaxEvaluations(10000);
        s1d.setLowerBound(minStrike);
        s1d.setUpperBound(maxStrike);
        Rate rStar = s1d.solve(finder, 1e-8, 0.00, minStrike,
                               maxStrike); // this is actually yStar

        Option::Type w =
            arguments_.type == Swap::Payer ? Option::Put : Option::Call;
        Size size = amounts.size();

        Real value = 0.0;
        for (Size i = startIndex; i < size; i++) {
            Real strike =
                model_->zerobond(payDates[i],
                                 arguments_.exercise->date(0), rStar) /
                model_->zerobond(arguments_.fixedResetDates[startIndex],
                                 arguments_.exercise->date(0), rStar);
            Real dboValue =
                model_->zerobondOption(w, arguments_.exercise->date(0),
                                       arguments_.fixedResetDates[startIndex],
                                       payDates[i], strike);
            value += amounts[i] * dboValue;
        }
        results_.value = value;
    }
}
