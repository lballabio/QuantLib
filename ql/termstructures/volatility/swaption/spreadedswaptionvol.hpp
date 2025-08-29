/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers

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

/*! \file spreadedswaptionvol.hpp
    \brief Spreaded swaption volatility
*/

#ifndef quantlib_spreaded_swaption_volstructure_h
#define quantlib_spreaded_swaption_volstructure_h

#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>

namespace QuantLib {

    class Quote;

    class SpreadedSwaptionVolatility : public SwaptionVolatilityStructure {
      public:
        SpreadedSwaptionVolatility(const Handle<SwaptionVolatilityStructure>&,
                                   Handle<Quote> spread);
        // All virtual methods of base classes must be forwarded
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const override;
        Date maxDate() const override;
        Time maxTime() const override;
        const Date& referenceDate() const override;
        Calendar calendar() const override;
        Natural settlementDays() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Rate minStrike() const override;
        Rate maxStrike() const override;
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        const Period& maxSwapTenor() const override;
        //@}
        VolatilityType volatilityType() const override;

      protected:
        //! \name SwaptionVolatilityStructure interface
        //@{
        ext::shared_ptr<SmileSection> smileSectionImpl(const Date& optionDate,
                                                       const Period& swapTenor) const override;
        ext::shared_ptr<SmileSection> smileSectionImpl(Time optionTime,
                                                       Time swapLength) const override;
        Volatility
        volatilityImpl(const Date& optionDate, const Period& swapTenor, Rate strike) const override;
        Volatility volatilityImpl(Time optionTime, Time swapLength, Rate strike) const override;
        Real shiftImpl(Time optionTime, Time swapLength) const override;
        //@}
      private:
        const Handle<SwaptionVolatilityStructure> baseVol_;
        const Handle<Quote> spread_;
    };

    inline DayCounter SpreadedSwaptionVolatility::dayCounter() const {
        return baseVol_->dayCounter();
    }

    inline Date SpreadedSwaptionVolatility::maxDate() const {
        return baseVol_->maxDate();
    }

    inline Time SpreadedSwaptionVolatility::maxTime() const {
        return baseVol_->maxTime();
    }

    inline const Date& SpreadedSwaptionVolatility::referenceDate() const {
        return baseVol_->referenceDate();
    }

    inline Calendar SpreadedSwaptionVolatility::calendar() const {
        return baseVol_->calendar();
    }

    inline Natural SpreadedSwaptionVolatility::settlementDays() const {
        return baseVol_->settlementDays();
    }

    inline Rate SpreadedSwaptionVolatility::minStrike() const {
        return baseVol_->minStrike();
    }

    inline Rate SpreadedSwaptionVolatility::maxStrike() const {
        return baseVol_->maxStrike();
    }

    inline const Period& SpreadedSwaptionVolatility::maxSwapTenor() const {
        return baseVol_->maxSwapTenor();
    }

    inline Real SpreadedSwaptionVolatility::shiftImpl(Time optionTime,
                                                  Time swapLength) const {
        return baseVol_->shift(optionTime, swapLength, true);
    }

    inline VolatilityType SpreadedSwaptionVolatility::volatilityType() const {
        return baseVol_->volatilityType();
    }



}

#endif
