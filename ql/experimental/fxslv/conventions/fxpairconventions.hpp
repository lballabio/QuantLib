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
#include <ql/experimental/fxslv/conventions/fxtenorexpiry.hpp>
#include <ql/experimental/fxslv/smilesections/fxsmilequotes.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <ql/time/period.hpp>
#include <string>
#include <utility>
#include <vector>

namespace QuantLib {

    //! The conventions of an FX pair
    /*! Everything about how a pair settles and how its vols are quoted,
        held in one object per pair: the spot and delivery dates, the
        expiry cut, the quoted vol tenors and forward tenors, and the
        delta, ATM and butterfly conventions of the vol quotes.

        The pair is foreign/domestic: the spot is the price of a unit of
        foreign currency in domestic currency, e.g. EUR/USD at 1.08.

        Modelling choices, such as the smile model or the trading-time
        weight of holidays, are not conventions of the pair: see
        FxVolSurfaceConfig.
    */
    class FxPairConventions {
      public:
        /*! \param foreignCurrency  code of the foreign (base) currency, e.g. "EUR".
            \param domesticCurrency code of the domestic (quote) currency, e.g. "USD".
            \param settlement       spot lag and joint calendar of the pair.
            \param cut              expiry cut of the quoted options.
            \param volTenors        the quoted vol tenors, increasing.
            \param deltas           the deltas of the risk reversals and
                                    butterflies, e.g. {0.25, 0.10}.
            \param deltaType        delta convention of the vol quotes.
            \param atmType          ATM convention of the vol quotes.
            \param flyType          whether butterflies are smile or broker strangles.
            \param forwardTenors    the tenors of the quoted forward points,
                                    increasing; none if no curve is
                                    bootstrapped from them.
        */
        FxPairConventions(std::string foreignCurrency,
                          std::string domesticCurrency,
                          FxSettleConvention settlement,
                          FxExpiryCut cut,
                          std::vector<Period> volTenors,
                          std::vector<Real> deltas,
                          DeltaVolQuote::DeltaType deltaType,
                          DeltaVolQuote::AtmType atmType,
                          FxRrBfQuotes::FlyType flyType,
                          std::vector<Period> forwardTenors = {})
        : foreignCurrency_(std::move(foreignCurrency)),
          domesticCurrency_(std::move(domesticCurrency)), settlement_(std::move(settlement)),
          cut_(cut), volTenors_(std::move(volTenors)), deltas_(std::move(deltas)),
          deltaType_(deltaType), atmType_(atmType), flyType_(flyType),
          forwardTenors_(std::move(forwardTenors)) {
            QL_REQUIRE(!foreignCurrency_.empty() && !domesticCurrency_.empty(),
                       "the currencies of an FX pair must be given");
            QL_REQUIRE(foreignCurrency_ != domesticCurrency_,
                       "an FX pair needs two currencies, not " << foreignCurrency_ << " twice");
            QL_REQUIRE(!volTenors_.empty(), name() << ": no vol tenors given");
            QL_REQUIRE(!deltas_.empty(), name() << ": no risk-reversal deltas given");
            for (Real d : deltas_)
                QL_REQUIRE(d > 0.0 && d < 0.5, name() << ": delta " << d << " not in (0, 0.5)");
        }

        //! \name Inspectors
        //@{
        const std::string& foreignCurrency() const { return foreignCurrency_; }
        const std::string& domesticCurrency() const { return domesticCurrency_; }
        //! The pair's name, e.g. "EURUSD".
        std::string name() const { return foreignCurrency_ + domesticCurrency_; }
        const FxSettleConvention& settlement() const { return settlement_; }
        const FxExpiryCut& cut() const { return cut_; }
        const std::vector<Period>& volTenors() const { return volTenors_; }
        const std::vector<Period>& forwardTenors() const { return forwardTenors_; }
        const std::vector<Real>& deltas() const { return deltas_; }
        DeltaVolQuote::DeltaType deltaType() const { return deltaType_; }
        DeltaVolQuote::AtmType atmType() const { return atmType_; }
        FxRrBfQuotes::FlyType flyType() const { return flyType_; }
        //@}

        //! The expiries of the vol tenors traded on the given date.
        std::vector<Date> volPillars(const Date& tradeDate) const {
            return fxExpiryDates(tradeDate, volTenors_, settlement_);
        }

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
        std::vector<Period> volTenors_;
        std::vector<Real> deltas_;
        DeltaVolQuote::DeltaType deltaType_;
        DeltaVolQuote::AtmType atmType_;
        FxRrBfQuotes::FlyType flyType_;
        std::vector<Period> forwardTenors_;
    };

}

#endif
