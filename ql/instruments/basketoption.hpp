/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2007 Joseph Wang

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

/*! \file basketoption.hpp
    \brief Basket option on a number of assets
*/

#ifndef quantlib_basket_option_hpp
#define quantlib_basket_option_hpp

#include <ql/instruments/payoffs.hpp>
#include <ql/instruments/multiassetoption.hpp>
#include <ql/math/array.hpp>

namespace QuantLib {

    class BasketPayoff : public Payoff {
      private:
        boost::shared_ptr<Payoff> basePayoff_;
      public:
        BasketPayoff(const boost::shared_ptr<Payoff> &p)
        : basePayoff_(p) {}
        virtual ~BasketPayoff() {}
        std::string name() const { return basePayoff_->name(); }
        std::string description() const { return basePayoff_->description(); }
        Real operator()(Real price) const { return (*basePayoff_)(price); }
        virtual Real operator()(const Array &a) const {
            return (*basePayoff_)(accumulate(a));
        }
        virtual Real accumulate(const Array &a) const = 0;
        const boost::shared_ptr<Payoff> basePayoff() {
            return basePayoff_;
        }
    };

    class MinBasketPayoff : public BasketPayoff {
      public:
        MinBasketPayoff(const boost::shared_ptr<Payoff> &p)
        : BasketPayoff(p) {}
        Real accumulate (const Array &a) const {
            return *std::min_element(a.begin(), a.end());
        }
    };

    class MaxBasketPayoff : public BasketPayoff {
      public:
        MaxBasketPayoff(const boost::shared_ptr<Payoff> &p)
        : BasketPayoff(p) {}
        Real accumulate (const Array &a) const {
            return *std::max_element(a.begin(), a.end());
        }
    };

    class AverageBasketPayoff : public BasketPayoff {
      public:
        AverageBasketPayoff(const boost::shared_ptr<Payoff> &p,
                            const Array &a)
        : BasketPayoff(p), weights_(a) {}
        AverageBasketPayoff(const boost::shared_ptr<Payoff> &p,
                            Size n)
        : BasketPayoff(p), weights_(n, 1.0/static_cast<Real>(n)) {}
        Real accumulate (const Array &a) const {
            return std::inner_product(weights_.begin(),
                                      weights_.end(),
                                      a.begin(), 0.0);
        }
      private:
        Array weights_;
    };


    class SpreadBasketPayoff : public BasketPayoff {
      public:
          SpreadBasketPayoff(const boost::shared_ptr<Payoff> &p)
        : BasketPayoff(p) {}
        Real accumulate (const Array &a) const {
            QL_REQUIRE(a.size() == 2, 
                    "payoff is only defined for two underlyings");
            return a[0]-a[1];
        }
    };
    
    //! Basket option on a number of assets
    /*! \ingroup instruments */
    class BasketOption : public MultiAssetOption {
      public:
        class engine;
        BasketOption(const boost::shared_ptr<BasketPayoff>&,
                     const boost::shared_ptr<Exercise>&);
    };

    //! %Basket-option %engine base class
    class BasketOption::engine
        : public GenericEngine<BasketOption::arguments,
                               BasketOption::results> {};

}


#endif

