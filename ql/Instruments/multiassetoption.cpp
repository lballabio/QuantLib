
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
        const std::vector<boost::shared_ptr<BlackScholesProcess> >& stochProcs,
        const boost::shared_ptr<Payoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const Matrix& correlation,
        const boost::shared_ptr<PricingEngine>& engine)
    : Option(payoff, exercise, engine),
      blackScholesProcesses_(stochProcs), correlation_(correlation) {

        for (Size i=0; i<blackScholesProcesses_.size(); i++)
            registerWith(blackScholesProcesses_[i]);
    }

    bool MultiAssetOption::isExpired() const {
        // what to do about term structures of differnet length?
        // we could take the max, or min
        // we could enfore the reference date to be the same for each process??
        return exercise_->lastDate() < 
            blackScholesProcesses_[0]->riskFreeRate()->referenceDate();
        //return true;
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
        arguments->blackScholesProcesses = blackScholesProcesses_;
        arguments->correlation = correlation_;
        arguments->exercise = exercise_;

        // shouldn't be here
        // it should be moved elsewhere
        //
        // just take the times from the first blackScholesProcess....
        // Hmmmmm, not a very nice solution
        boost::shared_ptr<BlackScholesProcess> blackScholesProcess =
            blackScholesProcesses_[0];
        arguments->stoppingTimes.clear();
        for (Size i=0; i<exercise_->dates().size(); i++) {
            Time time = 
                blackScholesProcess->riskFreeRate()->dayCounter().yearFraction(
                        blackScholesProcess->riskFreeRate()->referenceDate(), 
                        exercise_->date(i));
            arguments->stoppingTimes.push_back(time);
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
        QL_REQUIRE(correlation.rows() == blackScholesProcesses.size(),
                   "the size of the correlation matrix does not match "
                   "the number of underlyings");

        for (Size i=0; i<blackScholesProcesses.size(); i++) {
            QL_REQUIRE(blackScholesProcesses[i]->stateVariable(),
                       "no underlying given");
            QL_REQUIRE(blackScholesProcesses[i]->stateVariable()->value() 
                       > 0.0,
                       "negative or zero underlying given");
            QL_REQUIRE(blackScholesProcesses[i]->dividendYield(),
                       "no dividend term structure given");
            QL_REQUIRE(blackScholesProcesses[i]->riskFreeRate(),
                       "no risk free term structure given");
            QL_REQUIRE(blackScholesProcesses[i]->blackVolatility(),
                       "no vol term structure given");
        }
    }

}

