
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

/*! \file multiassetoption.cpp
    \brief Option on multiple assets
*/

#include <ql/Instruments/multiassetoption.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    MultiAssetOption::MultiAssetOption(
        const std::vector<Handle<BlackScholesStochasticProcess> >& stochProcs,
        const Handle<Payoff>& payoff,
        const Handle<Exercise>& exercise,
        const Matrix& correlation,
        const Handle<PricingEngine>& engine)
    : Option(payoff, exercise, engine),
      blackScholesProcesses_(stochProcs), correlation_(correlation) {

        // register all the stochastic processes
        Handle<BlackScholesStochasticProcess> blackScholesProcess; 
        std::vector< Handle<BlackScholesStochasticProcess> >::const_iterator proc = stochProcs.begin();
        while (proc != stochProcs.end()) {
            blackScholesProcess = *proc;
            registerWith(blackScholesProcess->stateVariable);        
            registerWith(blackScholesProcess->dividendTS);        
            registerWith(blackScholesProcess->riskFreeTS);        
            registerWith(blackScholesProcess->volTS);
            ++proc;
        }
    }

    bool MultiAssetOption::isExpired() const {
        // what to do about term structures of differnet length?
        // we could take the max, or min
        // we could enfore the reference date to be the same for each process??
        return exercise_->lastDate() < blackScholesProcesses_[0]->riskFreeTS->referenceDate();
        //return true;
    }

    double MultiAssetOption::delta() const {
        calculate();
        QL_REQUIRE(delta_ != Null<double>(),
                   "MultiAssetOption: delta not provided");
        return delta_;
    }

    double MultiAssetOption::gamma() const {
        calculate();
        QL_REQUIRE(gamma_ != Null<double>(),
                   "MultiAssetOption: gamma not provided");
        return gamma_;
    }

    double MultiAssetOption::theta() const {
        calculate();
        QL_REQUIRE(theta_ != Null<double>(),
                   "MultiAssetOption: theta not provided");
        return theta_;
    }
    
    double MultiAssetOption::vega() const {
        calculate();
        QL_REQUIRE(vega_ != Null<double>(),
                   "MultiAssetOption: vega not provided");
        return vega_;
    }

    double MultiAssetOption::rho() const {
        calculate();
        QL_REQUIRE(rho_ != Null<double>(),
                   "MultiAssetOption: rho not provided");
        return rho_;
    }

    double MultiAssetOption::dividendRho() const {
        calculate();
        QL_REQUIRE(dividendRho_ != Null<double>(),
                   "MultiAssetOption: dividend rho not provided");
        return dividendRho_;
    }
    
    void MultiAssetOption::setupExpired() const {
        NPV_ = delta_ = gamma_ = theta_ =
            vega_ = rho_ = dividendRho_ =  0.0;
    }

    void MultiAssetOption::setupArguments(Arguments* args) const {
        MultiAssetOption::arguments* arguments =
            dynamic_cast<MultiAssetOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "MultiAssetOption::setupArguments : "
                   "wrong argument type");

        arguments->payoff = payoff_;        


/*
        QL_REQUIRE(!IsNull(blackScholesProcess_->stateVariable->value()),
                   "MultiAssetOption::setupArguments : "
                   "null underlying price given");
*/
        
        arguments->blackScholesProcesses = blackScholesProcesses_;
        
        arguments->correlation = correlation_;

        arguments->exercise = exercise_;


        // shouldn't be here
        // it should be moved elsewhere
        //
        // just take the times from the first blackScholesProcess....
        // Hmmmmm, not a very nice solution
        Handle<BlackScholesStochasticProcess> blackScholesProcess; 
        std::vector< Handle<BlackScholesStochasticProcess> >::const_iterator proc = 
            blackScholesProcesses_.begin();
        blackScholesProcess = *proc;                    
        arguments->stoppingTimes.clear();
        for (Size i=0; i<exercise_->dates().size(); i++) {
            Time time = blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
                blackScholesProcess->riskFreeTS->referenceDate(), exercise_->date(i));
            arguments->stoppingTimes.push_back(time);
        }
    
    }

    void MultiAssetOption::performCalculations() const {
        Option::performCalculations();
        const Greeks* results =
            dynamic_cast<const Greeks*>(engine_->results());
        QL_ENSURE(results != 0,
                  "MultiAssetOption::performCalculations : "
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

        QL_ENSURE(NPV_ != Null<double>(),
                  "MultiAssetOption::performCalculations : "
                  "null value returned from option pricer");
    }


    void MultiAssetOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        Option::arguments copy = *this;
        copy.validate();
        #else
        Option::arguments::validate();
        #endif
        /*
        QL_REQUIRE(blackScholesProcess->stateVariable != Null<double>(),
                   "MultiAssetOption::arguments::validate() : "
                   "no underlying given");
        QL_REQUIRE(blackScholesProcess->stateVariable > 0.0,
                   "MultiAssetOption::arguments::validate() : "
                   "negative or zero underlying given");
        */

        /*
        QL_REQUIRE(!IsNull(correlation),
                    "MultiAssetOption::arguments::validate() : "
                    "no correlation given");
*/

        Handle<BlackScholesStochasticProcess> blackScholesProcess; 
        std::vector< Handle<BlackScholesStochasticProcess> >::const_iterator proc = 
            blackScholesProcesses.begin();
        while (proc != blackScholesProcesses.end()) {
            blackScholesProcess = *proc;                    
            QL_REQUIRE(!IsNull(blackScholesProcess->dividendTS),
                    "MultiAssetOption::arguments::validate() : "
                    "no dividend term structure given");
            QL_REQUIRE(!IsNull(blackScholesProcess->riskFreeTS),
                    "MultiAssetOption::arguments::validate() : "
                    "no risk free term structure given");
            QL_REQUIRE(!IsNull(blackScholesProcess->volTS),
                    "MultiAssetOption::arguments::validate() : "
                    "no vol term structure given");
            ++proc;
        }
    }


    
}

