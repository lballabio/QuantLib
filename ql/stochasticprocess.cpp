
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

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

#include <ql/stochasticprocess.hpp>
#include <ql/Volatilities/localvolsurface.hpp>
#include <ql/Volatilities/localvolcurve.hpp>
#include <ql/Volatilities/localconstantvol.hpp>

namespace QuantLib {

    // base class

    StochasticProcess::StochasticProcess() {}

    StochasticProcess::StochasticProcess(
             const boost::shared_ptr<StochasticProcess::discretization>& disc)
    : discretization_(disc) {}

    Real StochasticProcess::expectation(Time t0, Real x0, Time dt) const {
        return discretization_->expectation(*this, t0, x0, dt);
    }

    Real StochasticProcess::variance(Time t0, Real x0, Time dt) const {
        return discretization_->variance(*this, t0, x0, dt);
    }

    Real StochasticProcess::evolve(Real change, Real currentValue) const {
        return currentValue + change;
    }

    void StochasticProcess::update() {
        notifyObservers();
    }


    // Euler discretization

    Real EulerDiscretization::expectation(const StochasticProcess& process,
                                          Time t0, Real x0, Time dt) const {
        return x0 + process.drift(t0, x0)*dt;
    }

    Real EulerDiscretization::variance(const StochasticProcess& process,
                                       Time t0, Real x0, Time dt) const {
        Real sigma = process.diffusion(t0, x0);
        return sigma*sigma*dt;
    }


    // Geometric Brownian motion

    GeometricBrownianMotionProcess::GeometricBrownianMotionProcess(
                                                          double initialValue,
                                                          double mue,
                                                          double sigma)
    : StochasticProcess(boost::shared_ptr<StochasticProcess::discretization>(
                                                    new EulerDiscretization)),
      initialValue_(initialValue), mue_(mue), sigma_(sigma) {}

    Real GeometricBrownianMotionProcess::x0() const {
        return initialValue_;
    }

    Real GeometricBrownianMotionProcess::drift(Time, Real x) const {
        return mue_ * x;
    }

    Real GeometricBrownianMotionProcess::diffusion(Time, Real x) const {
        return sigma_ * x;
    }


    // Black-Scholes process

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
        return riskFreeRate_->forwardRate(t, t1, Continuous, NoFrequency, true)
             - dividendYield_->forwardRate(t, t1, Continuous, NoFrequency, true)
             - 0.5 * sigma * sigma;
    }

    Real BlackScholesProcess::diffusion(Time t, Real x) const {
        return localVolatility()->localVol(t, x, true);
    }

    Real BlackScholesProcess::evolve(Real change, Real currentValue) const {
        return currentValue * QL_EXP(change);
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
                                         constVol->blackVol(0.0, x0_->value()))));
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


    // Merton 76 process

    Merton76Process::Merton76Process(
             const Handle<Quote>& stateVariable,
             const Handle<YieldTermStructure>& dividendTS,
             const Handle<YieldTermStructure>& riskFreeTS,
             const Handle<BlackVolTermStructure>& blackVolTS,
             const Handle<Quote>& jumpInt,
             const Handle<Quote>& logJMean,
             const Handle<Quote>& logJVol,
             const boost::shared_ptr<StochasticProcess::discretization>& disc)
    : BlackScholesProcess(stateVariable, dividendTS, riskFreeTS,
                          blackVolTS, disc),
      jumpIntensity_(jumpInt), logMeanJump_(logJMean),
      logJumpVolatility_(logJVol) {
        registerWith(jumpIntensity_);
        registerWith(logMeanJump_);
        registerWith(logJumpVolatility_);
    }

    const boost::shared_ptr<Quote>& Merton76Process::jumpIntensity() const {
        return jumpIntensity_.currentLink();
    }

    const boost::shared_ptr<Quote>& Merton76Process::logMeanJump() const {
        return logMeanJump_.currentLink();
    }

    const boost::shared_ptr<Quote>&
    Merton76Process::logJumpVolatility() const {
        return logJumpVolatility_.currentLink();
    }


    // Ornstein-Uhlenbeck process

    OrnsteinUhlenbeckProcess::OrnsteinUhlenbeckProcess(Real speed,
                                                       Volatility vol,
                                                       Real x0)
    : x0_(x0), speed_(speed), volatility_(vol) {}

    Real OrnsteinUhlenbeckProcess::x0() const {
        return x0_;
    }

    Real OrnsteinUhlenbeckProcess::drift(Time, Real x) const {
        return - speed_*x;
    }

    Real OrnsteinUhlenbeckProcess::diffusion(Time, Real) const {
        return volatility_;
    }

    Real OrnsteinUhlenbeckProcess::expectation(Time, Real x0,
                                               Time dt) const {
        return x0*QL_EXP(-speed_*dt);
    }

    Real OrnsteinUhlenbeckProcess::variance(Time, Real, Time dt) const {
        return 0.5*volatility_*volatility_/speed_*
            (1.0 - QL_EXP(-2.0*speed_*dt));
    }


    // square-root process

    SquareRootProcess::SquareRootProcess(
             Real b, Real a, Volatility sigma, Real x0,
             const boost::shared_ptr<StochasticProcess::discretization>& disc)
    : StochasticProcess(disc), x0_(x0), mean_(b), speed_(a),
      volatility_(sigma) {}

    Real SquareRootProcess::x0() const {
        return x0_;
    }

    Real SquareRootProcess::drift(Time, Real x) const {
        return speed_*(mean_ - x);
    }

    Real SquareRootProcess::diffusion(Time, Real x) const {
        return volatility_*QL_SQRT(x);
    }

}
