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

/*! \file deltavolquote.hpp
    \brief Class for the quotation of delta vs vol.
*/

#ifndef quantlib_delta_vol_quote_hpp
#define quantlib_delta_vol_quote_hpp

#include <ql/quote.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    //! Class for the quotation of delta vs vol.
    /*! It includes the various delta quotation types
        in FX markets as well as ATM types.
    */
    class DeltaVolQuote : public Quote,
                          public Observer {
      public:
        enum DeltaType {
            Spot,        // Spot Delta, e.g. usual Black Scholes delta
            Fwd,         // Forward Delta
            PaSpot,      // Premium Adjusted Spot Delta
            PaFwd        // Premium Adjusted Forward Delta
        };

        enum AtmType {
            AtmNull,         // Default, if not an atm quote
            AtmSpot,         // K=S_0
            AtmFwd,          // K=F
            AtmDeltaNeutral, // Call Delta = Put Delta
            AtmVegaMax,      // K such that Vega is Maximum
            AtmGammaMax,     // K such that Gamma is Maximum
            AtmPutCall50     // K such that Call Delta=0.50 (only for Fwd Delta)
        };

        // Standard constructor delta vs vol.
        DeltaVolQuote(Real delta, Handle<Quote> vol, Time maturity, DeltaType deltaType);

        // Additional constructor, if special atm quote is used
        DeltaVolQuote(Handle<Quote> vol, DeltaType deltaType, Time maturity, AtmType atmType);

        void update() override;

        Real value() const override;
        Real delta() const;
        Time maturity() const;

        AtmType atmType()   const;
        DeltaType deltaType() const;

        bool isValid() const override;

      private:
          Real delta_;
          Handle<Quote> vol_;
          DeltaType deltaType_;
          Time maturity_;
          AtmType atmType_;
    };

}

#endif
