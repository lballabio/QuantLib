
/*
 Copyright (C) 2003 Ferdinando Ametrano

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
/*! \file localvolsurface.hpp
    \brief Local volatility surface derived from a Black vol surface

    \fullpath
    ql/Volatilities/%localvolsurface.hpp
*/

// $Id$

#ifndef quantlib_localvolsurface_hpp
#define quantlib_localvolsurface_hpp

#include <ql/voltermstructure.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Local volatility surface derived from a Black vol surface
        class LocalVolSurface : public LocalVolTermStructure,
                                public Patterns::Observer {
          public:
            // constructor
            LocalVolSurface(
                const RelinkableHandle<BlackVolTermStructure>& blackTS,
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const RelinkableHandle<TermStructure>& dividendTS,
                const RelinkableHandle<MarketElement>& underlying);
            LocalVolSurface(
                const RelinkableHandle<BlackVolTermStructure>& blackTS,
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const RelinkableHandle<TermStructure>& dividendTS,
                double underlying);
            // inspectors
            Date referenceDate() const {
                return blackTS_->referenceDate();
            }
            DayCounter dayCounter() const {
                return blackTS_->dayCounter();
            }
            Date maxDate() const { return blackTS_->maxDate(); }
            // Observer interface
            void update();
          protected:
            double localVolImpl(Time, double, bool extrapolate) const;
          private:
            RelinkableHandle<BlackVolTermStructure> blackTS_;
            RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
            RelinkableHandle<MarketElement> underlying_;
        };


        inline LocalVolSurface::LocalVolSurface(
            const RelinkableHandle<BlackVolTermStructure>& blackTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const RelinkableHandle<TermStructure>& dividendTS,
            const RelinkableHandle<MarketElement>& underlying)
        : blackTS_(blackTS), riskFreeTS_(riskFreeTS),
          dividendTS_(dividendTS), underlying_(underlying) {
            registerWith(blackTS_);
            registerWith(riskFreeTS_);
            registerWith(dividendTS_);
            registerWith(underlying_);
        }


        inline LocalVolSurface::LocalVolSurface(
            const RelinkableHandle<BlackVolTermStructure>& blackTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const RelinkableHandle<TermStructure>& dividendTS,
            double underlying)
        : blackTS_(blackTS), riskFreeTS_(riskFreeTS),
          dividendTS_(dividendTS) {
            registerWith(blackTS_);
            registerWith(riskFreeTS_);
            registerWith(dividendTS_);
            underlying_.linkTo(
                Handle<MarketElement>(new SimpleMarketElement(underlying)));
        }

        inline void LocalVolSurface::update() {
          notifyObservers();
        }

        inline double LocalVolSurface::localVolImpl(
            Time t, double underlyingLevel, bool extrapolate) const {

            double forwardValue = underlying_->value() *
                (dividendTS_->discount(t, extrapolate)/
                 riskFreeTS_->discount(t, extrapolate));

            double strike = underlyingLevel;
            double y = QL_LOG(strike/forwardValue);
            double dy = (y*0.0001 ? y*0.0001 : 0.0001);
            double yp = y+dy;
            double ym = y-dy;
            double strikep=QL_EXP(yp)*forwardValue;
            double strikem=QL_EXP(ym)*forwardValue;

            double w  = blackTS_->blackVariance(t, strike,  extrapolate);
            double wp = blackTS_->blackVariance(t, strikep, extrapolate);
            double wm = blackTS_->blackVariance(t, strikem, extrapolate);
            double dwdy = (wp-wm)/(2.0*dy);
            double d2wdy2 = (wp-2.0*w+wm)/(dy*dy);

            double dt, wpt, wmt, dwdt;
            if (t==0.0) {
                dt = 0.0001;
                wpt = blackTS_->blackVariance(t+dt, strike, extrapolate);
                QL_REQUIRE(wpt>=w,
                    "LocalVolSurface::localVolImpl : "
                    "decreasing variance");
                dwdt = (wpt-w)/dt;
            } else {
                dt = QL_MIN(0.0001, t);
                wpt = blackTS_->blackVariance(t+dt, strike, extrapolate);
                wmt = blackTS_->blackVariance(t-dt, strike, extrapolate);
                QL_REQUIRE(wpt>=w,
                    "LocalVolSurface::localVolImpl : "
                    "decreasing variance");
                QL_REQUIRE(w>=wmt,
                    "LocalVolSurface::localVolImpl : "
                    "decreasing variance");
                dwdt = (wpt-wmt)/(2.0*dt);
            }

            if (dwdy==0.0 && d2wdy2==0.0) { // avoid /w where w might be 0.0
                return QL_SQRT(dwdt);
            } else {
                double den1 = 1.0 - y/w*dwdy;
                double den2 = 0.25*(-0.25 - 1.0/w + y*y/w/w)*dwdy*dwdy;
                double den3 = 0.5*d2wdy2;
                double den = den1+den2+den3;
                double result = dwdt / den;
                QL_REQUIRE(result>=0.0,
                    "LocalVolSurface::localVolImpl : "
                    "negative vol^2, "
                    "the black vol surface is not smooth enough");
                return QL_SQRT(result);
    //            return QL_SQRT(dwdt / (1.0 - y/w*dwdy +
    //                0.25*(-0.25 - 1.0/w + y*y/w/w)*dwdy*dwdy + 0.5*d2wdy2));
            }
        }

    }

}


#endif
