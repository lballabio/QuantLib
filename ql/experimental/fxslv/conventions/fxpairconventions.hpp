/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file fxpairconventions.hpp
    \brief The conventions of an FX pair: settlement, expiry cut and vol quoting
*/

#ifndef quantlib_fx_pair_conventions_hpp
#define quantlib_fx_pair_conventions_hpp

#include <ql/experimental/fxslv/conventions/fxexpirycut.hpp>
#include <ql/experimental/fxslv/conventions/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilequotes.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <string>
#include <utility>

namespace QuantLib {

    //! The conventions of an FX pair
    /*! How a pair settles and how its vols are quoted, held in one object
        per pair: the spot and delivery dates, the expiry cut, and the
        delta, ATM and butterfly conventions of the vol quotes.

        Which tenors and deltas are quoted is market data, decided at run
        time, not a convention: see FxVolQuotes and bootstrapFxCurve().
        Modelling choices, such as the smile model or the trading-time
        weight of holidays, are not conventions either: see
        FxVolSurfaceConfig.

        The pair is foreign/domestic: the spot is the price of a unit of
        foreign currency in domestic currency, e.g. EUR/USD at 1.08.
    */
    class FxPairConventions {
      public:
        /*! \param foreignCurrency  code of the foreign (base) currency, e.g. "EUR".
            \param domesticCurrency code of the domestic (quote) currency, e.g. "USD".
            \param settlement       spot lag and joint calendar of the pair.
            \param cut              expiry cut of the quoted options.
            \param deltaType        delta convention of the vol quotes.
            \param atmType          ATM convention of the vol quotes.
            \param flyType          whether butterflies are smile or broker strangles.
        */
        FxPairConventions(std::string foreignCurrency,
                          std::string domesticCurrency,
                          FxSettleConvention settlement,
                          FxExpiryCut cut,
                          DeltaVolQuote::DeltaType deltaType,
                          DeltaVolQuote::AtmType atmType,
                          FxRrBfQuotes::FlyType flyType)
        : foreignCurrency_(std::move(foreignCurrency)),
          domesticCurrency_(std::move(domesticCurrency)), settlement_(std::move(settlement)),
          cut_(cut), deltaType_(deltaType), atmType_(atmType), flyType_(flyType) {
            QL_REQUIRE(!foreignCurrency_.empty() && !domesticCurrency_.empty(),
                       "the currencies of an FX pair must be given");
            QL_REQUIRE(foreignCurrency_ != domesticCurrency_,
                       "an FX pair needs two currencies, not " << foreignCurrency_ << " twice");
        }

        //! \name Inspectors
        //@{
        const std::string& foreignCurrency() const { return foreignCurrency_; }
        const std::string& domesticCurrency() const { return domesticCurrency_; }
        //! The pair's name, e.g. "EURUSD".
        std::string name() const { return foreignCurrency_ + domesticCurrency_; }
        const FxSettleConvention& settlement() const { return settlement_; }
        const FxExpiryCut& cut() const { return cut_; }
        DeltaVolQuote::DeltaType deltaType() const { return deltaType_; }
        DeltaVolQuote::AtmType atmType() const { return atmType_; }
        FxRrBfQuotes::FlyType flyType() const { return flyType_; }
        //@}

        //! The same conventions with butterflies of the given type.
        FxPairConventions withFlyType(FxRrBfQuotes::FlyType flyType) const {
            FxPairConventions result = *this;
            result.flyType_ = flyType;
            return result;
        }

      private:
        std::string foreignCurrency_, domesticCurrency_;
        FxSettleConvention settlement_;
        FxExpiryCut cut_;
        DeltaVolQuote::DeltaType deltaType_;
        DeltaVolQuote::AtmType atmType_;
        FxRrBfQuotes::FlyType flyType_;
    };

}

#endif
