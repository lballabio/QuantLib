/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/localvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/localvolcurve.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    GeneralizedBlackScholesProcess::GeneralizedBlackScholesProcess(
             const Handle<Quote>& x0,
             const Handle<YieldTermStructure>& dividendTS,
             const Handle<YieldTermStructure>& riskFreeTS,
             const Handle<BlackVolTermStructure>& blackVolTS,
             const boost::shared_ptr<discretization>& disc)
    : StochasticProcess1D(disc), x0_(x0), riskFreeRate_(riskFreeTS),
      dividendYield_(dividendTS), blackVolatility_(blackVolTS),
      updated_(false) {
        registerWith(x0_);
        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(blackVolatility_);
    }

    Real GeneralizedBlackScholesProcess::x0() const {
        return x0_->value();
    }

    Real GeneralizedBlackScholesProcess::drift(Time t, Real x) const {
        Real sigma = diffusion(t,x);
        // we could be more anticipatory if we know the right dt
        // for which the drift will be used
        Time t1 = t + 0.0001;
        return riskFreeRate_->forwardRate(t,t1,Continuous,NoFrequency,true)
             - dividendYield_->forwardRate(t,t1,Continuous,NoFrequency,true)
             - 0.5 * sigma * sigma;
    }

    Real GeneralizedBlackScholesProcess::diffusion(Time t, Real x) const {
        return localVolatility()->localVol(t, x, true);
    }

    Real GeneralizedBlackScholesProcess::apply(Real x0, Real dx) const {
        return x0 * std::exp(dx);
    }

    Real GeneralizedBlackScholesProcess::expectation(Time,
                                                     Real,
                                                     Time) const {
        QL_FAIL("not implemented");
    }

    Real GeneralizedBlackScholesProcess::evolve(Time t0, Real x0,
                                                Time dt, Real dw) const {
        return apply(x0, discretization_->drift(*this,t0,x0,dt) +
                         stdDeviation(t0,x0,dt)*dw);
    }

    Time GeneralizedBlackScholesProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

    void GeneralizedBlackScholesProcess::update() {
        updated_ = false;
        StochasticProcess1D::update();
    }

    const Handle<Quote>&
    GeneralizedBlackScholesProcess::stateVariable() const {
        return x0_;
    }

    const Handle<YieldTermStructure>&
    GeneralizedBlackScholesProcess::dividendYield() const {
        return dividendYield_;
    }

    const Handle<YieldTermStructure>&
    GeneralizedBlackScholesProcess::riskFreeRate() const {
        return riskFreeRate_;
    }

    const Handle<BlackVolTermStructure>&
    GeneralizedBlackScholesProcess::blackVolatility() const {
        return blackVolatility_;
    }

    const Handle<LocalVolTermStructure>&
    GeneralizedBlackScholesProcess::localVolatility() const {
        if (!updated_) {

            // constant Black vol?
            boost::shared_ptr<BlackConstantVol> constVol =
                boost::dynamic_pointer_cast<BlackConstantVol>(
                                                          *blackVolatility());
            if (constVol) {
                // ok, the local vol is constant too.
                localVolatility_.linkTo(
                    boost::shared_ptr<LocalVolTermStructure>(new
                        LocalConstantVol(constVol->referenceDate(),
                                         constVol->blackVol(0.0, x0_->value()),
                                         constVol->dayCounter())));
                updated_ = true;
                return localVolatility_;
            }

            // ok, so it's not constant. Maybe it's strike-independent?
            boost::shared_ptr<BlackVarianceCurve> volCurve =
                boost::dynamic_pointer_cast<BlackVarianceCurve>(
                                                          *blackVolatility());
            if (volCurve) {
                // ok, we can use the optimized algorithm
                localVolatility_.linkTo(
                    boost::shared_ptr<LocalVolTermStructure>(
                        new LocalVolCurve(
                                      Handle<BlackVarianceCurve>(volCurve))));
                updated_ = true;
                return localVolatility_;
            }

            // ok, so it's strike-dependent. Never mind.
            localVolatility_.linkTo(
                      boost::shared_ptr<LocalVolTermStructure>(
                          new LocalVolSurface(blackVolatility_, riskFreeRate_,
                                              dividendYield_, x0_->value())));
            updated_ = true;
            return localVolatility_;

        } else {
            return localVolatility_;
        }
    }


    // specific models

    BlackScholesProcess::BlackScholesProcess(
                              const Handle<Quote>& x0,
                              const Handle<YieldTermStructure>& riskFreeTS,
                              const Handle<BlackVolTermStructure>& blackVolTS,
                              const boost::shared_ptr<discretization>& d)
    : GeneralizedBlackScholesProcess(
             x0,
             // no dividend yield
             Handle<YieldTermStructure>(boost::shared_ptr<YieldTermStructure>(
                  new FlatForward(0, NullCalendar(), 0.0, Actual365Fixed()))),
             riskFreeTS,
             blackVolTS,
             d) {}


    BlackScholesMertonProcess::BlackScholesMertonProcess(
                              const Handle<Quote>& x0,
                              const Handle<YieldTermStructure>& dividendTS,
                              const Handle<YieldTermStructure>& riskFreeTS,
                              const Handle<BlackVolTermStructure>& blackVolTS,
                              const boost::shared_ptr<discretization>& d)
    : GeneralizedBlackScholesProcess(x0,dividendTS,riskFreeTS,blackVolTS,d) {}


    BlackProcess::BlackProcess(const Handle<Quote>& x0,
                               const Handle<YieldTermStructure>& riskFreeTS,
                               const Handle<BlackVolTermStructure>& blackVolTS,
                               const boost::shared_ptr<discretization>& d)
    : GeneralizedBlackScholesProcess(x0,riskFreeTS,riskFreeTS,blackVolTS,d) {}


    GarmanKohlagenProcess::GarmanKohlagenProcess(
                          const Handle<Quote>& x0,
                          const Handle<YieldTermStructure>& foreignRiskFreeTS,
                          const Handle<YieldTermStructure>& domesticRiskFreeTS,
                          const Handle<BlackVolTermStructure>& blackVolTS,
                          const boost::shared_ptr<discretization>& d)
    : GeneralizedBlackScholesProcess(x0,foreignRiskFreeTS,domesticRiskFreeTS,
                                     blackVolTS,d) {}

}
