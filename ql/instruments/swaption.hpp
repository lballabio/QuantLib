/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2014 Ferdinando Ametrano
 Copyright (C) 2016, 2018 Peter Caspers

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

/*! \file swaption.hpp
    \brief Swaption class
*/

#ifndef quantlib_instruments_swaption_hpp
#define quantlib_instruments_swaption_hpp

#include <ql/option.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    //! %settlement information
    struct Settlement {
        enum Type { Physical, Cash };
        enum Method {
            PhysicalOTC,
            PhysicalCleared,
            CollateralizedCashPrice,
            ParYieldCurve
        };
        //! check consistency of settlement type and method
        static void checkTypeAndMethodConsistency(Settlement::Type,
                                                  Settlement::Method);
    };

    std::ostream& operator<<(std::ostream& out,
                             Settlement::Type type);

    std::ostream& operator<<(std::ostream& out,
                             Settlement::Method method);

    //! %Swaption class
    /*! \ingroup instruments

        \test
        - the correctness of the returned value is tested by checking
          that the price of a payer (resp. receiver) swaption
          decreases (resp. increases) with the strike.
        - the correctness of the returned value is tested by checking
          that the price of a payer (resp. receiver) swaption
          increases (resp. decreases) with the spread.
        - the correctness of the returned value is tested by checking
          it against that of a swaption on a swap with no spread and a
          correspondingly adjusted fixed rate.
        - the correctness of the returned value is tested by checking
          it against a known good value.
        - the correctness of the returned value of cash settled swaptions
          is tested by checking the modified annuity against a value
          calculated without using the Swaption class.


        \todo add greeks and explicit exercise lag
    */
    class Swaption : public Option {
      public:
        class arguments;
        class engine;
        Swaption(std::shared_ptr<VanillaSwap> swap,
                 const std::shared_ptr<Exercise>& exercise,
                 Settlement::Type delivery = Settlement::Physical,
                 Settlement::Method settlementMethod = Settlement::PhysicalOTC);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        //@}
        //! \name Inspectors
        //@{
        Settlement::Type settlementType() const { return settlementType_; }
        Settlement::Method settlementMethod() const {
            return settlementMethod_;
        }
        Swap::Type type() const { return swap_->type(); }
        const std::shared_ptr<VanillaSwap>& underlyingSwap() const {
            return swap_;
        }
        //@}
        //! implied volatility
        Volatility impliedVolatility(
                              Real price,
                              const Handle<YieldTermStructure>& discountCurve,
                              Volatility guess,
                              Real accuracy = 1.0e-4,
                              Natural maxEvaluations = 100,
                              Volatility minVol = 1.0e-7,
                              Volatility maxVol = 4.0,
                              VolatilityType type = ShiftedLognormal,
                              Real displacement = 0.0) const;
      private:
        // arguments
        std::shared_ptr<VanillaSwap> swap_;
        //Handle<YieldTermStructure> termStructure_;
        Settlement::Type settlementType_;
        Settlement::Method settlementMethod_;
    };

    //! %Arguments for swaption calculation
    class Swaption::arguments : public VanillaSwap::arguments,
                                public Option::arguments {
      public:
        arguments() = default;
        std::shared_ptr<VanillaSwap> swap;
        Settlement::Type settlementType = Settlement::Physical;
        Settlement::Method settlementMethod;
        void validate() const override;
    };

    //! base class for swaption engines
    class Swaption::engine
        : public GenericEngine<Swaption::arguments, Swaption::results> {};

}

#endif
