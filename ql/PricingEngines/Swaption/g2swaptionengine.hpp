
/*
 Copyright (C) 2004 Mike Parker

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

/*! \file g2swaptionengine.hpp
    \brief Swaption pricing engine for two-factor additive Gaussian Model G2++
*/

#ifndef quantlib_pricers_G2_swaption_hpp
#define quantlib_pricers_G2_swaption_hpp

#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/PricingEngines/genericmodelengine.hpp>
#include <ql/ShortRateModels/TwoFactorModels/g2.hpp>

namespace QuantLib {

    //! %Swaption priced by means of the Black formula
    /*! \ingroup swaptionengines */
    class G2SwaptionEngine : public GenericModelEngine<G2, Swaption::arguments,
                                                           Swaption::results> {
      public:
        // range is the number of standard deviations to use in the
        // exponential term of the integral for the european swaption.
        // intervals is the number of intervals to use in the integration.
        G2SwaptionEngine(const  boost::shared_ptr<G2>& mod,
                         Real range,
                         Size intervals)
        : GenericModelEngine<G2, Swaption::arguments, Swaption::results>(mod),
          range_(range), intervals_(intervals) {}
        void calculate() const {
            results_.value =  model_->swaption(arguments_, range_, intervals_);
        }
      private:
        Real range_;
        Size intervals_;
    };
}


#endif
