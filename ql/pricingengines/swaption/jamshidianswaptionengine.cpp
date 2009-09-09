/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    class JamshidianSwaptionEngine::rStarFinder {
      public:
        rStarFinder(const boost::shared_ptr<OneFactorAffineModel>& model,
                    Real nominal,
                    Time maturity,
                    const std::vector<Time>& fixedPayTimes,
                    const std::vector<Real>& amounts)
        : strike_(nominal), maturity_(maturity), times_(fixedPayTimes),
          amounts_(amounts), model_(model) {}

        Real operator()(Rate x) const {
            Real value = strike_;
            Size size = times_.size();
            for (Size i=0; i<size; i++) {
                Real dbValue =
                    model_->discountBond(maturity_, times_[i], x);
                value -= amounts_[i]*dbValue;
            }
            return value;
        }
      private:
        Real strike_;
        Time maturity_;
        std::vector<Time> times_;
        const std::vector<Real>& amounts_;
        const boost::shared_ptr<OneFactorAffineModel>& model_;
    };

    void JamshidianSwaptionEngine::calculate() const {

        QL_REQUIRE(arguments_.settlementType==Settlement::Physical,
                   "cash-settled swaptions not priced by Jamshidian engine");

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "cannot use the Jamshidian decomposition "
                   "on exotic swaptions");

        Date referenceDate;
        DayCounter dayCounter;

        boost::shared_ptr<TermStructureConsistentModel> tsmodel =
            boost::dynamic_pointer_cast<TermStructureConsistentModel>(*model_);
        if (tsmodel) {
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
        for (Size i=0; i<fixedPayTimes.size(); i++)
            fixedPayTimes[i] =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.fixedPayDates[i]);

        rStarFinder finder(*model_, arguments_.nominal, maturity,
                           fixedPayTimes, amounts);
        Brent s1d;
        Rate minStrike = -10.0;
        Rate maxStrike = 10.0;
        s1d.setMaxEvaluations(10000);
        s1d.setLowerBound(minStrike);
        s1d.setUpperBound(maxStrike);
        Rate rStar = s1d.solve(finder, 1e-8, 0.05, minStrike, maxStrike);

        Option::Type w = arguments_.type==VanillaSwap::Payer ?
                                                Option::Put : Option::Call;
        Size size = arguments_.fixedCoupons.size();

        Real value = 0.0;
        for (Size i=0; i<size; i++) {
            Real fixedPayTime =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.fixedPayDates[i]);
            Real strike = model_->discountBond(maturity,
                                               fixedPayTime,
                                               rStar);
            Real dboValue = model_->discountBondOption(
                                               w, strike, maturity,
                                               fixedPayTime);
            value += amounts[i]*dboValue;
        }
        results_.value = value;
    }

}

