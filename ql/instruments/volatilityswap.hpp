/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yang Ye

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file volatilityswap.hpp
    \brief Volatility swap
*/

#ifndef quantlib_volatility_swap_hpp
#define quantlib_volatility_swap_hpp

#include <ql/instrument.hpp>
#include <ql/position.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    //! Volatility swap
    /*! A volatility swap pays, at maturity,
        \f[
        N \left( \sigma_R - K \right)
        \f]
        to the long side, where \f$ N \f$ is the notional (the payoff
        per unit of volatility), \f$ K \f$ the volatility strike and
        \f$ \sigma_R = \sqrt{V} \f$ the square root of the annualized
        realized variance \f$ V \f$ over the life of the swap.
        Strike and realized volatility are expressed as absolute
        annualized volatilities (e.g., 0.20 for 20%).

        \warning This class does not manage seasoned volatility swaps.

        \ingroup instruments
    */
    class VolatilitySwap : public Instrument {
      public:
        class arguments;
        class results;
        class engine;
        VolatilitySwap(Position::Type position,
                       Real strike,
                       Real notional,
                       const Date& startDate,
                       const Date& maturityDate);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        //@}
        //! \name Inspectors
        //@{
        Real strike() const;
        Position::Type position() const;
        Date startDate() const;
        Date maturityDate() const;
        Real notional() const;
        //@}
        //! \name Results
        //@{
        //! fair volatility strike, i.e., the strike for which the swap is worth zero
        Real fairVolatility() const;
        //@}
        void setupArguments(PricingEngine::arguments* args) const override;
        void fetchResults(const PricingEngine::results*) const override;

      protected:
        void setupExpired() const override;
        // data members
        Position::Type position_;
        Real strike_;
        Real notional_;
        Date startDate_, maturityDate_;
        // results
        mutable Real fairVolatility_;
    };


    //! %Arguments for volatility-swap calculation
    class VolatilitySwap::arguments : public virtual PricingEngine::arguments {
      public:
        arguments() : strike(Null<Real>()), notional(Null<Real>()) {}
        void validate() const override;
        Position::Type position;
        Real strike;
        Real notional;
        Date startDate;
        Date maturityDate;
    };


    //! %Results from volatility-swap calculation
    class VolatilitySwap::results : public Instrument::results {
      public:
        Real fairVolatility;
        void reset() override {
            Instrument::results::reset();
            fairVolatility = Null<Real>();
        }
    };

    //! base class for volatility-swap engines
    class VolatilitySwap::engine :
        public GenericEngine<VolatilitySwap::arguments,
                             VolatilitySwap::results> {};


    // inline definitions

    inline Date VolatilitySwap::startDate() const {
        return startDate_;
    }

    inline Date VolatilitySwap::maturityDate() const {
        return maturityDate_;
    }

    inline Real VolatilitySwap::strike() const {
        return strike_;
    }

    inline Real VolatilitySwap::notional() const {
        return notional_;
    }

    inline Position::Type VolatilitySwap::position() const {
        return position_;
    }

}


#endif
