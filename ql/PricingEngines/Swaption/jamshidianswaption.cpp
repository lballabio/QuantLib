
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/PricingEngines/Swaption/jamshidianswaption.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    class JamshidianSwaption::rStarFinder {
      public:
        rStarFinder(const Swaption::arguments &params,
                    const boost::shared_ptr<OneFactorAffineModel>& model,
                    const std::vector<double>& amounts)
        : strike_(params.nominal), maturity_(params.stoppingTimes[0]),
          times_(params.fixedPayTimes), amounts_(amounts), model_(model) {
        }

        double operator()(double x) const {
            double value = strike_;
            Size size = times_.size();
            for (Size i=0; i<size; i++) {
                double dbValue =
                    model_->discountBond(maturity_, times_[i], x);
                value -= amounts_[i]*dbValue;
            }
            return value;
        }
      private:
        double strike_;
        Time maturity_;
        const std::vector<Time>& times_;
        const std::vector<double>& amounts_;
        const boost::shared_ptr<OneFactorAffineModel>& model_;
    };

    void JamshidianSwaption::calculate() const {
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "Cannot use the Jamshidian decomposition "
                   "on exotic swaptions");
        Time maturity = arguments_.stoppingTimes[0];
        QL_REQUIRE(maturity==arguments_.floatingResetTimes[0],
                   "Maturity must be equal to first reset date");

        std::vector<double> amounts(arguments_.fixedCoupons);
        amounts.back() += arguments_.nominal;

        rStarFinder finder(arguments_, model_, amounts);
        Brent s1d;
        double minStrike = -10.0;
        double maxStrike = 10.0;
        s1d.setMaxEvaluations(10000);
        s1d.setLowerBound(minStrike);
        s1d.setUpperBound(maxStrike);
        double rStar = s1d.solve(finder, 1e-8, 0.05, minStrike, maxStrike);

        Option::Type type = arguments_.payFixed?Option::Put:Option::Call;
        Size size = arguments_.fixedCoupons.size();
        double value = 0.0;
        for (Size i=0; i<size; i++) {
            double strike = model_->discountBond(maturity,
                                                 arguments_.fixedPayTimes[i], 
                                                 rStar);
            double dboValue = model_->discountBondOption(
                                                 type, strike, maturity, 
                                                 arguments_.fixedPayTimes[i]);
            value += amounts[i]*dboValue;
        }
        results_.value = value;
    }

}
