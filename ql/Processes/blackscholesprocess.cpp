/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Processes/blackscholesprocess.hpp>
#include <ql/Volatilities/localvolsurface.hpp>
#include <ql/Volatilities/localvolcurve.hpp>
#include <ql/Volatilities/localconstantvol.hpp>

namespace QuantLib {

    BlackScholesProcess::BlackScholesProcess(
             const Handle<Quote>& x0,
             const Handle<YieldTermStructure>& dividendTS,
             const Handle<YieldTermStructure>& riskFreeTS,
             const Handle<BlackVolTermStructure>& blackVolTS,
             const boost::shared_ptr<StochasticProcess::discretization>& disc)
    : StochasticProcess(disc), x0_(x0), riskFreeRate_(riskFreeTS),
      dividendYield_(dividendTS), blackVolatility_(blackVolTS),
      updated_(false) {
        registerWith(x0_);
        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(blackVolatility_);
    }

    Real BlackScholesProcess::x0() const {
        return x0_->value();
    }

    Real BlackScholesProcess::drift(Time t, Real x) const {
        Real sigma = diffusion(t,x);
        // we could be more anticipatory if we know the right dt
        // for which the drift will be used
        Time t1 = t + 0.0001;
        return riskFreeRate_->forwardRate(t,t1,Continuous,NoFrequency,true)
             - dividendYield_->forwardRate(t,t1,Continuous,NoFrequency,true)
             - 0.5 * sigma * sigma;
    }

    Real BlackScholesProcess::diffusion(Time t, Real x) const {
        return localVolatility()->localVol(t, x, true);
    }

    Real BlackScholesProcess::evolve(Real change, Real currentValue) const {
        return currentValue * std::exp(change);
    }

    Time BlackScholesProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

    void BlackScholesProcess::update() {
        updated_ = false;
        StochasticProcess::update();
    }

    const boost::shared_ptr<Quote>&
    BlackScholesProcess::stateVariable() const {
        return x0_.currentLink();
    }

    const boost::shared_ptr<YieldTermStructure>&
    BlackScholesProcess::dividendYield() const {
        return dividendYield_.currentLink();
    }

    const boost::shared_ptr<YieldTermStructure>&
    BlackScholesProcess::riskFreeRate() const {
        return riskFreeRate_.currentLink();
    }

    const boost::shared_ptr<BlackVolTermStructure>&
    BlackScholesProcess::blackVolatility() const {
        return blackVolatility_.currentLink();
    }

    const boost::shared_ptr<LocalVolTermStructure>&
    BlackScholesProcess::localVolatility() const {
        if (!updated_) {

            // constant Black vol?
            boost::shared_ptr<BlackConstantVol> constVol =
                boost::dynamic_pointer_cast<BlackConstantVol>(
                                                           blackVolatility());
            if (constVol) {
                // ok, the local vol is constant too.
                localVolatility_.linkTo(
                    boost::shared_ptr<LocalVolTermStructure>(new
                        LocalConstantVol(constVol->referenceDate(),
                                         constVol->blackVol(0.0, x0_->value()),
                                         constVol->dayCounter())));
                updated_ = true;
                return localVolatility_.currentLink();
            }

            // ok, so it's not constant. Maybe it's strike-independent?
            boost::shared_ptr<BlackVarianceCurve> volCurve =
                boost::dynamic_pointer_cast<BlackVarianceCurve>(
                                                           blackVolatility());
            if (volCurve) {
                // ok, we can use the optimized algorithm
                localVolatility_.linkTo(
                    boost::shared_ptr<LocalVolTermStructure>(
                        new LocalVolCurve(
                                      Handle<BlackVarianceCurve>(volCurve))));
                updated_ = true;
                return localVolatility_.currentLink();
            }

            // ok, so it's strike-dependent. Never mind.
            localVolatility_.linkTo(
                      boost::shared_ptr<LocalVolTermStructure>(
                          new LocalVolSurface(blackVolatility_, riskFreeRate_,
                                              dividendYield_, x0_->value())));
            updated_ = true;
            return localVolatility_.currentLink();

        } else {
            return localVolatility_.currentLink();
        }
    }

}
