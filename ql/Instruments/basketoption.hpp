/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file basketoption.hpp
    \brief Basket option on a number of assets
*/

#ifndef quantlib_basket_option_hpp
#define quantlib_basket_option_hpp

#include <ql/Instruments/payoffs.hpp>
#include <ql/Instruments/multiassetoption.hpp>

namespace QuantLib {

    //! Basket option on a number of assets
    /*! \ingroup instruments 
      @TODO: Replace with STL algorithms */

    class BasketOptionType {
    public:
        virtual ~BasketOptionType() {};
        virtual Real pricingFunction(const Array &a) = 0;
    };

    class MinBasketOptionType : public BasketOptionType {
    public:
        MinBasketOptionType() {};
        Real pricingFunction(const Array &a) {
            return *std::min_element(a.begin(), a.end());
        }
    };

    class MaxBasketOptionType : public BasketOptionType {
    public:
        MaxBasketOptionType() {};
        Real pricingFunction(const Array &a) {
            return *std::max_element(a.begin(), a.end());
        }
    };

    class AverageBasketOptionType : public BasketOptionType {
    public:
        AverageBasketOptionType(const Array &a) :
            weights_(a) {};
        Real pricingFunction(const Array &a) {
            Real basketPrice = 0.0;
            for (Size j = 0; j < weights_.size(); j++) {
                basketPrice += a[j] * weights_[j];
            }
            return basketPrice;
        }
    private:
        Array weights_;
    };

    class BasketOption : public MultiAssetOption {
      public:
        class arguments;
        class engine;
        typedef boost::shared_ptr<BasketOptionType> type;
        enum BasketType { Min, Max };
        // Backward compatibility
        BasketOption(BasketType, 
                     const boost::shared_ptr<StochasticProcess>&,
                     const boost::shared_ptr<PlainVanillaPayoff>&,
                     const boost::shared_ptr<Exercise>&,
                     const boost::shared_ptr<PricingEngine>& engine =
                                          boost::shared_ptr<PricingEngine>());
        BasketOption(const boost::shared_ptr<BasketOptionType>&,
                     const boost::shared_ptr<StochasticProcess>&,
                     const boost::shared_ptr<PlainVanillaPayoff>&,
                     const boost::shared_ptr<Exercise>&,
                     const boost::shared_ptr<PricingEngine>& engine =
                                          boost::shared_ptr<PricingEngine>());
        void setupArguments(PricingEngine::arguments*) const;
      private:
        type basketType_;
    };

    //! %Arguments for basket option calculation
    class BasketOption::arguments : public MultiAssetOption::arguments {
      public:
        arguments() {}
        void validate() const;
        BasketOption::type basketType;
    };


    //! %Basket option engine base class
    class BasketOption::engine
        : public GenericEngine<BasketOption::arguments,
                               BasketOption::results> {};

}


#endif

