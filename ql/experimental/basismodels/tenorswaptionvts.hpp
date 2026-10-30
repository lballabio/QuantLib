/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file tenorswaptionvts.hpp
    \brief swaption volatility term structure based on volatility transformation
*/

#ifndef quantlib_tenorswaptionvts_hpp
#define quantlib_tenorswaptionvts_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/option.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>
#include <utility>

namespace QuantLib {

    class TenorSwaptionVTS : public SwaptionVolatilityStructure {
      protected:
        class TenorSwaptionSmileSection : public SmileSection {
          protected:
            ext::shared_ptr<SmileSection> baseSmileSection_;
            Real swapRateBase_;
            Real swapRateTarg_;
            Real swapRateFinl_;
            Real lambda_;
            Real annuityScaling_;
            // implement transformation formula
            Volatility volatilityImpl(Rate strike) const override;

          public:
            // constructor includes actual transformation details
            TenorSwaptionSmileSection(const TenorSwaptionVTS& volTS,
                                      Time optionTime,
                                      Time swapLength);

            // further SmileSection interface methods
            Real minStrike() const override {
                return baseSmileSection_->minStrike() + swapRateTarg_ - swapRateBase_;
            }
            Real maxStrike() const override {
                return baseSmileSection_->maxStrike() + swapRateTarg_ - swapRateBase_;
            }
            Real atmLevel() const override { return swapRateFinl_; }
        };

        Handle<SwaptionVolatilityStructure> baseVTS_;
        Handle<YieldTermStructure> discountCurve_;

        ext::shared_ptr<IborIndex> baseIndex_;
        ext::shared_ptr<IborIndex> targIndex_;
        Period baseFixedFreq_;
        Period targFixedFreq_;
        DayCounter baseFixedDC_;
        DayCounter targFixedDC_;

      public:
        // constructor
        TenorSwaptionVTS(const Handle<SwaptionVolatilityStructure>& baseVTS,
                         Handle<YieldTermStructure> discountCurve,
                         ext::shared_ptr<IborIndex> baseIndex,
                         ext::shared_ptr<IborIndex> targIndex,
                         const Period& baseFixedFreq,
                         const Period& targFixedFreq,
                         DayCounter baseFixedDC,
                         DayCounter targFixedDC)
        : SwaptionVolatilityStructure(baseVTS->referenceDate(),
                                      baseVTS->calendar(),
                                      baseVTS->businessDayConvention(),
                                      baseVTS->dayCounter()),
          baseVTS_(baseVTS), discountCurve_(std::move(discountCurve)),
          baseIndex_(std::move(baseIndex)), targIndex_(std::move(targIndex)),
          baseFixedFreq_(baseFixedFreq), targFixedFreq_(targFixedFreq),
          baseFixedDC_(std::move(baseFixedDC)), targFixedDC_(std::move(targFixedDC)) {}

        // Termstructure interface

        //! the latest date for which the curve can return values
        Date maxDate() const override { return baseVTS_->maxDate(); }

        // SwaptionVolatility interface

        //! the minimum strike for which the term structure can return vols
        Rate minStrike() const override { return baseVTS_->minStrike(); }
        //! the maximum strike for which the term structure can return vols
        Rate maxStrike() const override { return baseVTS_->maxStrike(); }


        // SwaptionVolatilityStructure interface

        //! the largest length for which the term structure can return vols
        const Period& maxSwapTenor() const override { return baseVTS_->maxSwapTenor(); }

        ext::shared_ptr<SmileSection> smileSectionImpl(Time optionTime,
                                                       Time swapLength) const override {
            return ext::shared_ptr<SmileSection>(
                new TenorSwaptionSmileSection(*this, optionTime, swapLength));
        }

        Volatility volatilityImpl(Time optionTime, Time swapLength, Rate strike) const override {
            return smileSectionImpl(optionTime, swapLength)->volatility(strike, Normal, 0.0);
        }

        // the methodology is designed for normal volatilities
        VolatilityType volatilityType() const override { return Normal; }
    };

}

#endif
