
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file vanillaoption.hpp
    \brief Vanilla option on a single asset
*/

#ifndef quantlib_vanilla_option_hpp
#define quantlib_vanilla_option_hpp

#include <ql/Instruments/oneassetstrikedoption.hpp>

namespace QuantLib {

    //! Vanilla option (no discrete dividends, no barriers) on a single asset
    /*! \ingroup instruments */
    class VanillaOption : public OneAssetStrikedOption {
      public:
        class engine;
        VanillaOption(const boost::shared_ptr<BlackScholesStochasticProcess>&,
                      const boost::shared_ptr<StrikedTypePayoff>& payoff,
                      const boost::shared_ptr<Exercise>& exercise,
                      const boost::shared_ptr<PricingEngine>& engine =
                          boost::shared_ptr<PricingEngine>());
    };

    //! Vanilla option engine base class
    class VanillaOption::engine 
        : public GenericEngine<VanillaOption::arguments,
                               VanillaOption::results> {};

}


#endif

