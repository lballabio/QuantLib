
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
#include <ql/PricingEngines/vanillaengines.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Local volatility surface derived from a Black vol surface
        class LocalVolSurface : public LocalVolTermStructure,
                                public Patterns::Observer {
          public:
            // constructor
            LocalVolSurface(
                const Handle<BlackVolTermStructure>& blackTS,
                const RelinkableHandle<TermStructure>& riskFreeTS,
                const RelinkableHandle<TermStructure>& dividendTS,
                const RelinkableHandle<MarketElement>& underlying);
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
            Handle<BlackVolTermStructure> blackTS_;
            RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
            RelinkableHandle<MarketElement> underlying_;
//            PricingEngines::AnalyticalVanillaEngine blackEngine_;
//            PricingEngines::VanillaOptionArguments * arguments_;
        };


        LocalVolSurface::LocalVolSurface(
            const Handle<BlackVolTermStructure>& blackTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const RelinkableHandle<TermStructure>& dividendTS,
            const RelinkableHandle<MarketElement>& underlying)
        : blackTS_(blackTS), riskFreeTS_(riskFreeTS),
          dividendTS_(dividendTS), underlying_(underlying) {
//                arguments_ =
//                    dynamic_cast<PricingEngines::VanillaOptionArguments*>(
//                        blackEngine_.arguments());
//                arguments_->type       = Option::Call;
//                arguments_->exercise   = EuropeanExercise(Date());
//                arguments_->volTS      = blackTS_;
//                arguments_->dividendTS = dividendTS_;
//                arguments_->riskFreeTS = riskFreeTS_;
        }


        void LocalVolSurface::update() {
            notifyObservers();
        }

        double LocalVolSurface::localVolImpl(
            Time t, double underlyingLevel, bool extrapolate) const {

/*
            // the following might be changed
            arguments_->underlying = underlying_->value();
            // time and strike
//            arguments_->exercise.date() = t;
            arguments_->strike = strike;

//            blackEngine_.arguments()->validate();
            blackEngine_.calculate();
            const PricingEngines::VanillaOptionResults* results =
                dynamic_cast<const PricingEngines::VanillaOptionResults*>(
                    blackEngine_.results());

            double value            = results->value;
            double theta            = results->theta;
//            double stikeSensitivity = results->strikeSensitivity;
            double stikeSensitivity = 0.0;
            double vega             = results->vega;
            double r = riskFreeTS_->zeroYield(t);
            double d = dividendTS_->zeroYield(t);
            double c1 = theta + vega *
                blackTS_->timeDerivative(t, strike, extrapolate);
            double c2 = (r - d) * strike * (stikeSensitivity + vega *
                blackTS_->strikeDerivative(t,strike,extrapolate));
            double c3 = d * value;
//            double c4 = (2nd der with respect to strike) * strike * strike;
            double c4 = strike * strike;
            return QL_SQRT(2.0*(c1+c2+c3)/c4);

*/

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

            double dt = 0.0001;
            double wpt = blackTS_->blackVariance(t+dt, strike, extrapolate);
            double wmt = blackTS_->blackVariance(t-dt, strike, extrapolate);
            QL_REQUIRE(wpt>=w,
                "LocalVolSurface::localVolImpl : "
                "decreasing variance");
            QL_REQUIRE(w>=wmt,
                "LocalVolSurface::localVolImpl : "
                "decreasing variance");
            double dwdt = (wpt-wmt)/(2.0*dt);

            return QL_SQRT(dwdt / (1.0 - y/w*dwdy +
                0.25*(-0.25 - 1.0/w + y*y/w/w)*dwdy*dwdy + 0.5*d2wdy2));
        }

    }

}


#endif
