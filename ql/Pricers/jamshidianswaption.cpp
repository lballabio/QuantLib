
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file jamshidianswaption.cpp
    \brief Swaption pricer using Jamshidian's decomposition

    \fullpath
    ql/Pricers/%jamshidianswaption.cpp
*/

// $Id$

#include "ql/Pricers/jamshidianswaption.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace InterestRateModelling;
        using Instruments::SwaptionParameters;

        class JamshidianSwaption::rStarFinder : public ObjectiveFunction {
          public:
            rStarFinder(const SwaptionParameters &params, 
                        const Handle<OneFactorModel>& model,
                        const std::vector<double>& amounts) 
            : strike_(params.nominals[0]), maturity_(params.exerciseTimes[0]), 
              times_(params.fixedPayTimes), amounts_(amounts), model_(model) {}

            double operator()(double x) const {
                double value = strike_;
                unsigned size = times_.size();
                for (unsigned i=0; i<size; i++) {
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
            const Handle<OneFactorModel>& model_;
        };      
            
        void JamshidianSwaption::calculate() const {
            QL_REQUIRE(
                parameters_.exerciseType == Exercise::European,
                "Cannot use the Jamshidian decomposition on exotic swaptions");
            Time maturity = parameters_.exerciseTimes[0];

            QL_REQUIRE( model_->type()==Model::OneFactor,
                "Jamshidian decomposition is only valid for one-factor models");
            Handle<OneFactorModel> model(model_);

            Rate r0 = model_->termStructure()->forward(0.0);

            QL_REQUIRE(model->hasDiscountBondFormula(),
                "No analytical formula for discount bonds");
            QL_REQUIRE(model->hasDiscountBondOptionFormula(),
                "No analytical formula for discount bond options");

            std::vector<double> amounts(parameters_.fixedCoupons);
            amounts.back() += parameters_.nominals[0];

            rStarFinder finder(parameters_, model, amounts);
            Solvers1D::Brent s1d = Solvers1D::Brent();
            double minStrike = 0.00001;
            double maxStrike = 0.20000;
            s1d.setMaxEvaluations(10000);
            s1d.setLowBound(minStrike);
            s1d.setHiBound(maxStrike);
            double rStar = s1d.solve(finder, 1e-8, r0, minStrike, maxStrike);

            Option::Type type = parameters_.payFixed?Option::Put:Option::Call;
            unsigned size = parameters_.fixedCoupons.size();
            double value = 0.0;
            for (unsigned i=0; i<size; i++) {
                double strike = model->discountBond(maturity, 
                    parameters_.fixedPayTimes[i], rStar);
                double dboValue = model->discountBondOption(
                    type, strike, maturity, parameters_.fixedPayTimes[i]);
                value += amounts[i]*dboValue;
            }
            results_.value = value;
        }
    }

}
