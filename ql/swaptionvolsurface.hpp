
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file swaptionvolsurface.hpp
    \brief Swaption volatility surface

    \fullpath
    ql/%swaptionvolsurface.hpp
*/

// $Id$

#ifndef quantlib_swaption_volatility_surface_h
#define quantlib_swaption_volatility_surface_h

#include <ql/date.hpp>
#include <ql/relinkablehandle.hpp>
#include <ql/marketelement.hpp>
#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    //! Swaption volatility surface
    /*! This class is purely abstract and defines the interface of concrete
        swaption volatility structures which will be derived from this one.
    */
    class SwaptionVolatilitySurface : public Patterns::Observable {
      public:
        virtual ~SwaptionVolatilitySurface() {}
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the day counter used for internal date/time conversions
        virtual DayCounter dayCounter() const = 0;
        //! returns the volatility for a given starting date and length
        double volatility(const Date& start, const Period& length) {
            Time startTime = dayCounter().yearFraction(todaysDate(),start,
                                                       todaysDate(),start);
            Date end = start.plus(length);
            Time timeLength = dayCounter().yearFraction(start,end,start,end);
            return volatilityImpl(startTime,timeLength);
        }
        //! returns the volatility for a given starting time and length
        double volatility(Time start, Time length) {
            return volatilityImpl(start,length);
        }
      protected:
        //! implements the actual volatility calculation in derived classes
        virtual double volatilityImpl(Time start, Time length) const = 0;
    };

    //! Swaption volatility surface with an added spread
    /*! This surface will remain linked to the original surface, i.e.,
        any changes in the latter will be reflected in this surface as well.
    */
    class SpreadedSwaptionVolatilitySurface : public SwaptionVolatilitySurface,
                                              public Patterns::Observer {
      public:
        SpreadedSwaptionVolatilitySurface(
            const RelinkableHandle<SwaptionVolatilitySurface>&, 
            const RelinkableHandle<MarketElement>& spread);
        //! Observer interface
        void update() { notifyObservers(); }
        DayCounter dayCounter() const;
      private:
        RelinkableHandle<SwaptionVolatilitySurface> originalSurface_;
        RelinkableHandle<MarketElement> spread_;
        //! volatility of the original surface plus the given spread
        double volatilityImpl(Time start, Time length) const;
    };


    // inline definitions

    inline
    SpreadedSwaptionVolatilitySurface::SpreadedSwaptionVolatilitySurface(
        const RelinkableHandle<SwaptionVolatilitySurface>& h, 
        const RelinkableHandle<MarketElement>& spread)
    : originalSurface_(h), spread_(spread) {
        registerWith(originalSurface_);
        registerWith(spread_);
    }

    inline double SpreadedSwaptionVolatilitySurface::volatilityImpl(
        Time start, Time length) const {
            return originalSurface_->volatility(start,length) + 
                   spread_->value();
    }

    inline DayCounter SpreadedSwaptionVolatilitySurface::dayCounter() const {
        return originalSurface_->dayCounter();
    }

}


#endif
