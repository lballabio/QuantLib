
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Volatilities/localvolsurface.hpp>

namespace QuantLib {

    LocalVolSurface::LocalVolSurface(
                                 const Handle<BlackVolTermStructure>& blackTS,
                                 const Handle<YieldTermStructure>& riskFreeTS,
                                 const Handle<YieldTermStructure>& dividendTS,
                                 const Handle<Quote>& underlying)
    : blackTS_(blackTS), riskFreeTS_(riskFreeTS), dividendTS_(dividendTS),
      underlying_(underlying) {
        registerWith(blackTS_);
        registerWith(riskFreeTS_);
        registerWith(dividendTS_);
        registerWith(underlying_);
    }


    LocalVolSurface::LocalVolSurface(
                                 const Handle<BlackVolTermStructure>& blackTS,
                                 const Handle<YieldTermStructure>& riskFreeTS,
                                 const Handle<YieldTermStructure>& dividendTS,
                                 Real underlying)
    : blackTS_(blackTS), riskFreeTS_(riskFreeTS), dividendTS_(dividendTS) {
        registerWith(blackTS_);
        registerWith(riskFreeTS_);
        registerWith(dividendTS_);
        underlying_.linkTo(
                       boost::shared_ptr<Quote>(new SimpleQuote(underlying)));
    }

    void LocalVolSurface::accept(AcyclicVisitor& v) {
        Visitor<LocalVolSurface>* v1 =
            dynamic_cast<Visitor<LocalVolSurface>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            LocalVolTermStructure::accept(v);
    }

    Volatility LocalVolSurface::localVolImpl(Time t, Real underlyingLevel)
                                                                     const {

        Real forwardValue = underlying_->value() *
            (dividendTS_->discount(t, true)/
             riskFreeTS_->discount(t, true));

        // strike derivatives
        Real strike, y, dy, strikep, strikem;
        Real w, wp, wm, dwdy, d2wdy2;
        strike = underlyingLevel;
        y = std::log(strike/forwardValue);
        dy = ((y!=0.0) ? y*0.000001 : 0.000001);
        strikep=strike*std::exp(dy);
        strikem=strike/std::exp(dy);
        w  = blackTS_->blackVariance(t, strike,  true);
        wp = blackTS_->blackVariance(t, strikep, true);
        wm = blackTS_->blackVariance(t, strikem, true);
        dwdy = (wp-wm)/(2.0*dy);
        d2wdy2 = (wp-2.0*w+wm)/(dy*dy);

        // time derivative
        Real dt, wpt, wmt, dwdt;
        if (t==0.0) {
            dt = 0.0001;
            wpt = blackTS_->blackVariance(t+dt, strike, true);
            QL_ENSURE(wpt>=w,
                      "decreasing variance at strike " << strike
                      << " between time " << t << " and time " << t+dt);
            dwdt = (wpt-w)/dt;
        } else {
            dt = std::min<Time>(0.0001, t/2.0);
            wpt = blackTS_->blackVariance(t+dt, strike, true);
            wmt = blackTS_->blackVariance(t-dt, strike, true);
            QL_ENSURE(wpt>=w,
                      "decreasing variance at strike " << strike
                      << " between time " << t << " and time " << t+dt);
            QL_ENSURE(w>=wmt,
                      "decreasing variance at strike " << strike
                      << " between time " << t-dt << " and time " << t);
            dwdt = (wpt-wmt)/(2.0*dt);
        }

        if (dwdy==0.0 && d2wdy2==0.0) { // avoid /w where w might be 0.0
            return std::sqrt(dwdt);
        } else {
            Real den1 = 1.0 - y/w*dwdy;
            Real den2 = 0.25*(-0.25 - 1.0/w + y*y/w/w)*dwdy*dwdy;
            Real den3 = 0.5*d2wdy2;
            Real den = den1+den2+den3;
            Real result = dwdt / den;
            QL_ENSURE(result>=0.0,
                      "negative local vol^2 at strike " << strike
                      << " and time " << t
                      << "; the black vol surface is not smooth enough");
            return std::sqrt(result);
            // return std::sqrt(dwdt / (1.0 - y/w*dwdy +
            //    0.25*(-0.25 - 1.0/w + y*y/w/w)*dwdy*dwdy + 0.5*d2wdy2));
        }
    }

}

