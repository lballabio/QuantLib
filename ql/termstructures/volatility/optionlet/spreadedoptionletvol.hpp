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

/*! \file spreadedoptionletvol.hpp
    \brief Spreaded caplet/floorlet volatility
*/

#ifndef quantlib_spreaded_caplet_volstructure_h
#define quantlib_spreaded_caplet_volstructure_h

#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>

namespace QuantLib {

    class Quote;

    class SpreadedOptionletVolatility : public OptionletVolatilityStructure {
      public:
        SpreadedOptionletVolatility(const Handle<OptionletVolatilityStructure>&,
                                    const Handle<Quote>& spread);
        // All virtual methods of base classes must be forwarded
        //! \name VolatilityTermStructure interface
        //@{
        BusinessDayConvention businessDayConvention() const;
        Rate minStrike() const;
        Rate maxStrike() const;
        //@}
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const Date& referenceDate() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        //@}
      protected:
        // All virtual methods of base classes must be forwarded
        //! \name OptionletVolatilityStructure interface
        //@{
        boost::shared_ptr<SmileSection> smileSectionImpl(const Date& d) const;
        boost::shared_ptr<SmileSection> smileSectionImpl(Time optionT) const;
        Volatility volatilityImpl(Time optionTime,
                                  Rate strike) const;
        //@}
      private:
        const Handle<OptionletVolatilityStructure> baseVol_;
        const Handle<Quote> spread_;
    };

    inline DayCounter SpreadedOptionletVolatility::dayCounter() const {
        return baseVol_->dayCounter();
    }
    
    inline Date SpreadedOptionletVolatility::maxDate() const {
        return baseVol_->maxDate();
    }

    inline Time SpreadedOptionletVolatility::maxTime() const {
        return baseVol_->maxTime();
    }
        
    inline const Date& SpreadedOptionletVolatility::referenceDate() const {
        return baseVol_->referenceDate();
    }
        
    inline Calendar SpreadedOptionletVolatility::calendar() const {
        return baseVol_->calendar();
    }
        
    inline Natural SpreadedOptionletVolatility::settlementDays() const {
        return baseVol_->settlementDays();
    }

    inline BusinessDayConvention
    SpreadedOptionletVolatility::businessDayConvention() const {
        return baseVol_->businessDayConvention();
    }
    
    inline Rate SpreadedOptionletVolatility::minStrike() const {
        return baseVol_->minStrike();
    }
    
    inline Rate SpreadedOptionletVolatility::maxStrike() const {
        return baseVol_->maxStrike();
    }

}

#endif
