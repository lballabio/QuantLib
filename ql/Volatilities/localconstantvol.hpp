
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
/*! \file localconstantvol.hpp
    \brief Local constant volatility, no time dependence, no asset dependence

    \fullpath
    ql/Volatilities/%localconstantvol.hpp
*/

// $Id$

#ifndef quantlib_localconstantvol_hpp
#define quantlib_localconstantvol_hpp

#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Constant local volatility, no time-strike dependence
        /*! This class implements the LocalVolatilityTermStructure
            interface for a constant local volatility (no time/asset
            dependence).
            Local volatility and Black volatility are the same when
            volatility is at most time dependent, so this class is
            basically a proxy for BlackVolatilityTermStructure.
        */
        class LocalConstantVol : public LocalVolTermStructure,
                                 public Patterns::Observer {
          public:
            // constructor
            LocalConstantVol(
                const Date& referenceDate,
                double volatility,
                const DayCounter& dayCounter = DayCounters::Actual365());
            LocalConstantVol(
                const Date& referenceDate,
                const RelinkableHandle<MarketElement>& volatility,
                const DayCounter& dayCounter = DayCounters::Actual365());
            // inspectors
            Date referenceDate() const { return referenceDate_; }
            DayCounter dayCounter() const { return dayCounter_; }
            Date maxDate() const { return Date::maxDate(); }
            // Observer interface
            void update();
          private:
            double localVolImpl(Time,double,bool extrapolate) const;
            Date referenceDate_;
            RelinkableHandle<MarketElement> volatility_;
            DayCounter dayCounter_;
        };

        // inline definitions

        inline LocalConstantVol::LocalConstantVol(const Date& referenceDate, 
            double volatility, const DayCounter& dayCounter)
        : referenceDate_(referenceDate), dayCounter_(dayCounter) {
            volatility_.linkTo(
                Handle<MarketElement>(new SimpleMarketElement(volatility)));
        }
            
        inline LocalConstantVol::LocalConstantVol(const Date& referenceDate, 
            const RelinkableHandle<MarketElement>& volatility, 
            const DayCounter& dayCounter)
        : referenceDate_(referenceDate), volatility_(volatility), 
          dayCounter_(dayCounter) {
            registerWith(volatility_);
        }

        inline void LocalConstantVol::update() {
            notifyObservers();
        }

        inline double LocalConstantVol::localVolImpl(Time t, 
                                        double, bool extrapolate) const {
            QL_REQUIRE(t >= 0.0, "LocalConstantVol: negative time given");
            return volatility_->value();
        }
    }

}


#endif
