/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti

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
                                   const Handle<Quote>& spread);
        // All virtual methods of base classes must be forwarded
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const Date& referenceDate() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Rate minStrike() const;
        Rate maxStrike() const;
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        const Period& maxSwapTenor() const;
        //@}
      protected:
        //! \name SwaptionVolatilityStructure interface
        //@{
        boost::shared_ptr<SmileSection> smileSectionImpl(
                                        const Date& optionDate,
                                        const Period& swapTenor) const;
        boost::shared_ptr<SmileSection> smileSectionImpl(
                                        Time optionTime,
                                        Time swapLength) const;
        Volatility volatilityImpl(const Date& optionDate,
                                  const Period& swapTenor,
                                  Rate strike) const;
        Volatility volatilityImpl(Time optionTime,
                                  Time swapLength,
                                  Rate strike) const;
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

}

#endif
