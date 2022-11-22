/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Mike Parker

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

/*! \file g2swaptionengine.hpp
    \brief Swaption pricing engine for two-factor additive Gaussian Model G2++
*/

#ifndef quantlib_pricers_G2_swaption_hpp
#define quantlib_pricers_G2_swaption_hpp

#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>

namespace QuantLib {

    //! %Swaption priced by means of the Black formula
    /*! \ingroup swaptionengines

        \warning The engine assumes that the exercise date equals the
                 start date of the passed swap.
    */
    class G2SwaptionEngine : public GenericModelEngine<G2, Swaption::arguments,
                                                           Swaption::results> {
      public:
        // range is the number of standard deviations to use in the
        // exponential term of the integral for the european swaption.
        // intervals is the number of intervals to use in the integration.
        G2SwaptionEngine(const ext::shared_ptr<G2>& model,
                         Real range,
                         Size intervals)
        : GenericModelEngine<G2, Swaption::arguments, Swaption::results>(model),
          range_(range), intervals_(intervals) {}
        void calculate() const override {
            QL_REQUIRE(arguments_.settlementType == Settlement::Physical,
                       "cash-settled swaptions not priced with G2 engine");
            QL_REQUIRE(!model_.empty(), "no model specified");

            // adjust the fixed rate of the swap for the spread on the
            // floating leg (which is not taken into account by the
            // model)
            VanillaSwap swap = *arguments_.swap;
            swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(model_->termStructure(), false));
            Spread correction = swap.spread() *
                std::fabs(swap.floatingLegBPS() / swap.fixedLegBPS());
            Rate fixedRate = swap.fixedRate() - correction;

            results_.value =  model_->swaption(arguments_, fixedRate,
                                               range_, intervals_);
        }

      private:
        Real range_;
        Size intervals_;
    };

}


#endif
