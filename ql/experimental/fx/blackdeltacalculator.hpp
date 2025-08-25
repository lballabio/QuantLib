/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Dimitri Reiswich

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

/*! \file blackdeltacalculator.hpp
    \brief Black-Scholes formula delta calculator class
*/

#ifndef quantlib_black_delta_calculator_hpp
#define quantlib_black_delta_calculator_hpp

#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/experimental/fx/deltavolquote.hpp>

namespace QuantLib {

    //! Black delta calculator class
    /*! Class includes many operations needed for different applications
        in FX markets, which has special quoation mechanisms, since
        every price can be expressed in both numeraires.
    */
    class BlackDeltaCalculator {
      friend class BlackDeltaPremiumAdjustedSolverClass;
      friend class BlackDeltaPremiumAdjustedMaxStrikeClass;

      public:
        //! \name Constructors
        //@{
        /*!
            \brief Constructs a BlackDeltaCalculator object 
            This class provides methods to calculate option delta and strike values
            using the Black-Scholes formula, supporting various FX delta conventions
            (spot, forward, premium-adjusted, etc.). It is designed for efficient
            repeated calculations across different strikes, which is useful in
            volatility smile construction and calibration routines.

            \param ot Option type (call or put)
            \param dt Delta type (spot, forward, premium-adjusted, etc.)
            \param spot Spot FX rate
            \param dDiscount Domestic discount factor
            \param dDiscount Foreign discount factor
            \param stdDev Standard deviation volatility*sqrt(timeToMaturity)
            
            \warning Make sure you are passing the correct standard deviation
                 i.e. volatility*sqrt(timeToMaturity)
        */
        BlackDeltaCalculator(Option::Type ot,
                             DeltaVolQuote::DeltaType dt,
                             Real spot,
                             DiscountFactor dDiscount,   // domestic discount
                             DiscountFactor fDiscount,   // foreign discount
                             Real stdDev);
        /*!
            \brief Computes the option delta for a given strike.

            Calculates the delta of an option using the Black-Scholes formula,
            according to the delta convention specified at construction (spot, forward, premium-adjusted, etc.).

            \param strike The option strike price.
            \return       The option delta under the chosen convention.
        */
        Real deltaFromStrike(Real strike) const;
        /*!
            \brief Computes the strike corresponding to a given delta.

            Inverts the Black-Scholes formula to find the strike that yields the specified delta,
            according to the delta convention set at construction. Used for constructing volatility smiles
            and for quoting FX options by delta.

            \param delta  The target option delta (under the chosen convention).
            \return       The strike price corresponding to the given delta.
        */
        Real strikeFromDelta(Real delta) const;
        /*!
            \brief Calculates the at-the-money (ATM) strike for the given ATM convention.
            
            Computes the strike price that corresponds to "at-the-money" under different conventions
            commonly used in FX markets. This method does not require an explicit strike input
            as it determines the ATM level based on the specified convention.
            
            \param atmT The ATM convention to use:
                       - AtmNull: No ATM convention (returns null)
                       - AtmSpot: ATM strike equals the current spot rate
                       - AtmForward: ATM strike equals the forward rate
                       - AtmDeltaNeutral: ATM strike where call and put deltas sum to zero
                       - AtmVegaMax: ATM strike that maximizes vega (typically close to forward)
                       - AtmGammaMax: ATM strike that maximizes gamma
                       - AtmPutCall25: ATM strike where 25-delta call and put have equal volatility
            
            \return The ATM strike price according to the specified convention.
            
            \note This calculation is independent of the strike and uses the forward rate,
                  volatility, and time to expiration set at construction.
        */
        Real atmStrike(DeltaVolQuote::AtmType atmT) const;
        /*!
            \brief Sets the delta calculation convention.
            
            \param dt The new delta type convention:
        */
        void setDeltaType(DeltaVolQuote::DeltaType dt);
        /*!
            \brief Sets the option type (call or put).
                      
            \param ot The option type
        */
        void setOptionType(Option::Type ot);

      protected:
        Real cumD1(Real strike) const;    // N(d1) or N(-d1)
        Real cumD2(Real strike) const;    // N(d2) or N(-d2)

        Real nD1(Real strike) const;      // n(d1)
        Real nD2(Real strike) const;      // n(d2)

      private:
        // alternative delta type
        Real strikeFromDelta(Real delta, DeltaVolQuote::DeltaType dt) const;

        DeltaVolQuote::DeltaType dt_;
        Option::Type ot_;
        DiscountFactor dDiscount_, fDiscount_;
        Real stdDev_, spot_, forward_;
        Integer phi_;
        Real fExpPos_,fExpNeg_;
    };


    class BlackDeltaPremiumAdjustedSolverClass {
      public:
        BlackDeltaPremiumAdjustedSolverClass(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev,
                        Real delta);

        Real operator()(Real strike) const;

      private:
        BlackDeltaCalculator bdc_;
        Real delta_;
    };


    class BlackDeltaPremiumAdjustedMaxStrikeClass {
      public:
        BlackDeltaPremiumAdjustedMaxStrikeClass(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev);

        Real operator()(Real strike) const;

      private:
        BlackDeltaCalculator bdc_;
        Real stdDev_;
    };

}


#endif
