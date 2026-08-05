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
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <cmath>

namespace QuantLib {

    class JamshidianSwaptionEngine::rStarFinder {
      public:
        rStarFinder(const ext::shared_ptr<OneFactorAffineModel>& model,
                    Real nominal,
                    Time maturity,
                    Time valueTime,
                    const std::vector<Time>& fixedPayTimes,
                    const std::vector<Real>& amounts)
        : strike_(nominal), discountRatios_(fixedPayTimes.size()),
          rateCoefficients_(fixedPayTimes.size()), amounts_(amounts) {

            /* Optimization: Since for exercise time t_e

               P(t_e,T;r) = A(t_e,T) exp(-B(t_e,T) r)

               the bond-price ratios used here in operator() are

               R_i(r) = P(t_e,T_i;r) / P(t_e,T_0;r)
                      = (A(t_e,T_i) / A(t_e,T_0)) exp(-[B(t_e,T_i)-B(t_e,T_0)] r)

               that is, they have the form

               R_i(r) = c_i exp(d_i r)

               where the c_i (discount ratios) and the d_i (rate coefficients) don't depend on r.

               The loop below determines c_i and d_i for each i by plugging r=0 and r=1
               in the formula, yielding

               R_i(0) = c_i
               R_i(1) = c_i exp(d_i)

               from which we get c_i = R_i(0) and d_i = log(R_i(1) / R_i(0)) that can be stored
               and used in discountBondRatio() to save computation time.
            */

            const Real valueDiscount0 = model->discountBond(maturity, valueTime, 0.0);
            const Real valueDiscount1 = model->discountBond(maturity, valueTime, 1.0);

            for (Size i = 0; i < fixedPayTimes.size(); ++i) {
                const Real discount0 = model->discountBond(maturity, fixedPayTimes[i], 0.0);
                const Real discount1 = model->discountBond(maturity, fixedPayTimes[i], 1.0);
                discountRatios_[i] = discount0 / valueDiscount0;
                rateCoefficients_[i] =
                    std::log((discount1 / valueDiscount1) / discountRatios_[i]);
            }
        }

        Real discountBondRatio(Size i, Rate x) const {
            return discountRatios_[i] * std::exp(rateCoefficients_[i] * x);
        }

        Real operator()(Rate x) const {
            Real value = strike_;
            for (Size i = 0; i < discountRatios_.size(); ++i)
                value -= amounts_[i] * discountBondRatio(i, x);
            return value;
        }
      private:
        Real strike_;
        std::vector<Real> discountRatios_, rateCoefficients_;
        const std::vector<Real>& amounts_;
    };

    void JamshidianSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementMethod != Settlement::ParYieldCurve,
                   "cash settled (ParYieldCurve) swaptions not priced with "
                   "JamshidianSwaptionEngine");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "cannot use the Jamshidian decomposition "
                   "on exotic swaptions");

        QL_REQUIRE(arguments_.swap->spread() == 0.0, "non zero spread (" << arguments_.swap->spread() << ") not allowed");

        QL_REQUIRE(arguments_.nominal != Null<Real>(),
                   "non-constant nominals are not supported yet");

        QL_REQUIRE(!model_.empty(), "no model specified");

        Date referenceDate;
        DayCounter dayCounter;

        ext::shared_ptr<TermStructureConsistentModel> tsmodel =
            ext::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        if (tsmodel != nullptr) {
            referenceDate = tsmodel->termStructure()->referenceDate();
            dayCounter = tsmodel->termStructure()->dayCounter();
        } else {
            referenceDate = termStructure_->referenceDate();
            dayCounter = termStructure_->dayCounter();
        }

        std::vector<Real> amounts(arguments_.fixedCoupons);
        amounts.back() += arguments_.nominal;

        Real maturity = dayCounter.yearFraction(referenceDate,
                                                arguments_.exercise->date(0));

        std::vector<Time> fixedPayTimes(arguments_.fixedPayDates.size());
        Time valueTime = dayCounter.yearFraction(referenceDate,arguments_.fixedResetDates[0]);
        for (Size i=0; i<fixedPayTimes.size(); i++)
            fixedPayTimes[i] = dayCounter.yearFraction(referenceDate,
                                                       arguments_.fixedPayDates[i]);

        rStarFinder finder(*model_, arguments_.nominal, maturity, valueTime,
                           fixedPayTimes, amounts);
        Brent s1d;
        Rate minStrike = -10.0;
        Rate maxStrike = 10.0;
        s1d.setMaxEvaluations(10000);
        s1d.setLowerBound(minStrike);
        s1d.setUpperBound(maxStrike);
        Rate rStar = s1d.solve(finder, 1e-8, 0.05, minStrike, maxStrike);

        Option::Type w = arguments_.type==Swap::Payer ? Option::Put : Option::Call;
        Size size = arguments_.fixedCoupons.size();

        Real value = 0.0;
        for (Size i=0; i<size; i++) {
            Real fixedPayTime =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.fixedPayDates[i]);
            Real strike = finder.discountBondRatio(i, rStar);
            // Looks like the swaption decomposed into individual options adjusted for maturity. Each individual option is valued by Hull-White (or other one-factor model).
            Real dboValue = model_->discountBondOption(
                                               w, strike, maturity, valueTime,
                                               fixedPayTime);
            value += amounts[i]*dboValue;
        }
        results_.value = value;
    }

}
