
/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Instruments/multiassetoption.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    MultiAssetOption::MultiAssetOption(
        const std::vector<boost::shared_ptr<StochasticProcess> >& processes,
        const boost::shared_ptr<Payoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const Matrix& correlation,
        const boost::shared_ptr<PricingEngine>& engine)
    : Option(payoff, exercise, engine),
      stochasticProcesses_(processes), correlation_(correlation) {

        for (Size i=0; i<stochasticProcesses_.size(); i++)
            registerWith(stochasticProcesses_[i]);
    }

    bool MultiAssetOption::isExpired() const {
        return exercise_->lastDate() < Settings::instance().evaluationDate();
    }

    Real MultiAssetOption::delta() const {
        calculate();
        QL_REQUIRE(delta_ != Null<Real>(), "delta not provided");
        return delta_;
    }

    Real MultiAssetOption::gamma() const {
        calculate();
        QL_REQUIRE(gamma_ != Null<Real>(), "gamma not provided");
        return gamma_;
    }

    Real MultiAssetOption::theta() const {
        calculate();
        QL_REQUIRE(theta_ != Null<Real>(), "theta not provided");
        return theta_;
    }

    Real MultiAssetOption::vega() const {
        calculate();
        QL_REQUIRE(vega_ != Null<Real>(), "vega not provided");
        return vega_;
    }

    Real MultiAssetOption::rho() const {
        calculate();
        QL_REQUIRE(rho_ != Null<Real>(), "rho not provided");
        return rho_;
    }

    Real MultiAssetOption::dividendRho() const {
        calculate();
        QL_REQUIRE(dividendRho_ != Null<Real>(), "dividend rho not provided");
        return dividendRho_;
    }

    void MultiAssetOption::setupExpired() const {
        NPV_ = delta_ = gamma_ = theta_ =
            vega_ = rho_ = dividendRho_ =  0.0;
    }

    void MultiAssetOption::setupArguments(Arguments* args) const {
        MultiAssetOption::arguments* arguments =
            dynamic_cast<MultiAssetOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->payoff = payoff_;
        arguments->stochasticProcesses = stochasticProcesses_;
        arguments->correlation = correlation_;
        arguments->exercise = exercise_;

        // take the times from the first process.
        // it might be made more robust by checking that all processes
        // return the same times, but what the hey...
        arguments->stoppingTimes.clear();
        const boost::shared_ptr<StochasticProcess>& process =
            stochasticProcesses_[0];
        for (Size i=0; i<exercise_->dates().size(); i++) {
            arguments->stoppingTimes.push_back(
                                           process->time(exercise_->date(i)));
        }
    }

    void MultiAssetOption::performCalculations() const {
        Option::performCalculations();
        const Greeks* results =
            dynamic_cast<const Greeks*>(engine_->results());
        QL_ENSURE(results != 0,
                  "no greeks returned from pricing engine");
        /* no check on null values - just copy.
           this allows:
           a) to decide in derived options what to do when null
           results are returned (throw? numerical calculation?)
           b) to implement slim engines which only calculate the
           value---of course care must be taken not to call
           the greeks methods when using these.
        */
        delta_          = results->delta;
        gamma_          = results->gamma;
        theta_          = results->theta;
        vega_           = results->vega;
        rho_            = results->rho;
        dividendRho_    = results->dividendRho;
    }


    void MultiAssetOption::arguments::validate() const {
        #if defined(QL_PATCH_MSVC6)
        Option::arguments copy = *this;
        copy.validate();
        #else
        Option::arguments::validate();
        #endif

        QL_REQUIRE(correlation.rows() == correlation.columns(),
                   "correlation matrix is not square");
        QL_REQUIRE(correlation.rows() == stochasticProcesses.size(),
                   "the size of the correlation matrix does not match "
                   "the number of underlyings");

        for (Size i=0; i<stochasticProcesses.size(); i++) {
            QL_REQUIRE(stochasticProcesses[i]->x0() > 0.0,
                       "negative or zero underlying given");
        }
    }

}

