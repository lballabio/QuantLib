
/*
 Copyright (C) 2003 Neil Firth

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

/*! \file basketoption.hpp
    \brief Basket option on a number of assets
*/

#ifndef quantlib_basket_option_h
#define quantlib_basket_option_h


#include <ql/Instruments/payoffs.hpp>
#include <ql/Instruments/multiassetoption.hpp>

namespace QuantLib {

    //! Basket option on a number of assets
    class BasketOption : public MultiAssetOption {
      public:
        class arguments;
        enum BasketType { Min, Max };
        BasketOption(
               const BasketType basketType,
               const std::vector<Handle<BlackScholesStochasticProcess> >& 
                                                                  stochProcs,
               const Handle<PlainVanillaPayoff>& payoff,
               const Handle<Exercise>& exercise,
               const Matrix& correlation,
               const Handle<PricingEngine>& engine = Handle<PricingEngine>());

        void setupArguments(Arguments*) const;
      protected:
        // enforce in this class any check on engine/payoff
        void performCalculations() const;
      private:
        BasketType basketType_;
    };

    //! arguments for basket option calculation
    class BasketOption::arguments : public MultiAssetOption::arguments {
      public:
        arguments() {}
        void validate() const;
        BasketType basketType;
    };


    //! Basket option engine base class
    class BasketEngine : public GenericEngine<BasketOption::arguments,
                                               BasketOption::results> {};

}


#endif

