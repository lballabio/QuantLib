
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

#ifndef quantlib_vanilla_option_h
#define quantlib_vanilla_option_h

#include <ql/Instruments/oneassetstrikedoption.hpp>

namespace QuantLib {

    //! Vanilla option (no discrete dividends, no barriers) on a single asset
    class VanillaOption : public OneAssetStrikedOption {
      public:
        VanillaOption(const Handle<BlackScholesStochasticProcess>& stochProc,
                      const Handle<StrikedTypePayoff>& payoff,
                      const Handle<Exercise>& exercise,
                      const Handle<PricingEngine>& engine =
                          Handle<PricingEngine>());
      protected:
        // enforce in this class any check on engine/payoff
        void performCalculations() const;
    };

    //! Vanilla option engine base class
    class VanillaEngine : public GenericEngine<VanillaOption::arguments,
                                               VanillaOption::results> {};

}


#endif

