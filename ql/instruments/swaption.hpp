/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2007 StatPro Italia srl

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
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! %settlement information
    struct Settlement {
        enum Type { Physical, Cash };
    };

    std::ostream& operator<<(std::ostream& out,
                             Settlement::Type type);

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
        // constructors
        Swaption(const boost::shared_ptr<VanillaSwap>& swap,
                 const boost::shared_ptr<Exercise>& exercise,
                 Settlement::Type delivery = Settlement::Physical);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
        //! \name Inspectors
        //@{
        Settlement::Type settlementType() const { return settlementType_; }
        VanillaSwap::Type type() const { return swap_->type(); }
        const boost::shared_ptr<VanillaSwap>& underlyingSwap() const {
            return swap_;
        }
        //@}
        void setupArguments(PricingEngine::arguments*) const;
        //! implied volatility
        Volatility impliedVolatility(
                              Real price,
                              const Handle<YieldTermStructure>& termStructure,
                              Real accuracy = 1.0e-4,
                              Size maxEvaluations = 100,
                              Volatility minVol = 1.0e-7,
                              Volatility maxVol = 4.0) const;
        Rate atmRate() const;
      private:
        // arguments
        boost::shared_ptr<VanillaSwap> swap_;
        //Handle<YieldTermStructure> termStructure_;
        Settlement::Type settlementType_;
        // helper class for implied volatility calculation
        class ImpliedVolHelper {
          public:
            ImpliedVolHelper(const Swaption&,
                             const Handle<YieldTermStructure>&,
                             Real targetValue);
            Real operator()(Volatility x) const;
          private:
            boost::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> termStructure_;
            Real targetValue_;
            boost::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };
    };

    //! %Arguments for swaption calculation
    class Swaption::arguments : public VanillaSwap::arguments,
                                public Option::arguments {
      public:
        arguments() : //fairRate(Null<Real>()),
                      //fixedRate(Null<Real>()),
                      //fixedBPS(Null<Real>()),
                      //fixedCashBPS(Null<Real>()),
                      //forecastingDiscount(Null<Real>()),
                      settlementType(Settlement::Physical) {}
        boost::shared_ptr<VanillaSwap> swap;
        // Rate fairRate;
        // Rate fixedRate;
        // Real fixedBPS;
        // Real fixedCashBPS;
        // Real forecastingDiscount;
        Settlement::Type settlementType;
        void validate() const;
    };

    //! base class for swaption engines
    class Swaption::engine
        : public GenericEngine<Swaption::arguments, Swaption::results> {};

}

#endif

