
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

#include <ql/Instruments/oneassetoption.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    OneAssetOption::OneAssetOption(
        const boost::shared_ptr<BlackScholesStochasticProcess>& stochProc,
        const boost::shared_ptr<Payoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : Option(payoff, exercise, engine),
      blackScholesProcess_(stochProc) {
        registerWith(blackScholesProcess_->stateVariable);
        registerWith(blackScholesProcess_->dividendTS);
        registerWith(blackScholesProcess_->riskFreeTS);
        registerWith(blackScholesProcess_->volTS);
    }

    bool OneAssetOption::isExpired() const {
        return exercise_->lastDate() < blackScholesProcess_->riskFreeTS->referenceDate();
    }

    double OneAssetOption::delta() const {
        calculate();
        QL_REQUIRE(delta_ != Null<double>(),
                   "OneAssetOption: delta not provided");
        return delta_;
    }

    double OneAssetOption::deltaForward() const {
        calculate();
        QL_REQUIRE(deltaForward_ != Null<double>(),
                   "OneAssetOption: deltaForward not provided");
        return deltaForward_;
    }

    double OneAssetOption::elasticity() const {
        calculate();
        QL_REQUIRE(elasticity_ != Null<double>(),
                   "OneAssetOption: elasticity not provided");
        return elasticity_;
    }

    double OneAssetOption::gamma() const {
        calculate();
        QL_REQUIRE(gamma_ != Null<double>(),
                   "OneAssetOption: gamma not provided");
        return gamma_;
    }

    double OneAssetOption::theta() const {
        calculate();
        QL_REQUIRE(theta_ != Null<double>(),
                   "OneAssetOption: theta not provided");
        return theta_;
    }

    double OneAssetOption::thetaPerDay() const {
        calculate();
        QL_REQUIRE(thetaPerDay_ != Null<double>(),
                   "OneAssetOption: thetaPerDay not provided");
        return thetaPerDay_;
    }

    double OneAssetOption::vega() const {
        calculate();
        QL_REQUIRE(vega_ != Null<double>(),
                   "OneAssetOption: vega not provided");
        return vega_;
    }

    double OneAssetOption::rho() const {
        calculate();
        QL_REQUIRE(rho_ != Null<double>(),
                   "OneAssetOption: rho not provided");
        return rho_;
    }

    double OneAssetOption::dividendRho() const {
        calculate();
        QL_REQUIRE(dividendRho_ != Null<double>(),
                   "OneAssetOption: dividend rho not provided");
        return dividendRho_;
    }

    double OneAssetOption::itmCashProbability() const {
        calculate();
        QL_REQUIRE(itmCashProbability_ != Null<double>(),
                   "OneAssetOption::itmCashProbability() "
                   "in-the-money cash probability not provided");
        return itmCashProbability_;
    }

    double OneAssetOption::impliedVolatility(double targetValue,
                                            double accuracy,
                                            Size maxEvaluations,
                                            double minVol,
                                            double maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(),
                   "OneAssetOption::impliedVolatility : "
                   "option expired");

        double guess = blackScholesProcess_->volTS->blackVol(
            exercise_->lastDate(),
            blackScholesProcess_->stateVariable->value());

        ImpliedVolHelper f(engine_,targetValue);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        // Borland compiler fails here: cannot enter into Solver1D::solve(...)
        double result = solver.solve(f, accuracy, guess, minVol, maxVol);
        return result;
    }

    void OneAssetOption::setupExpired() const {
        NPV_ = delta_ = deltaForward_ = elasticity_ = gamma_ = theta_ =
            thetaPerDay_ = vega_ = rho_ = dividendRho_ =
            itmCashProbability_ = 0.0;
    }

    void OneAssetOption::setupArguments(Arguments* args) const {
        OneAssetOption::arguments* arguments =
            dynamic_cast<OneAssetOption::arguments*>(args);
        QL_REQUIRE(arguments != 0,
                   "OneAssetOption::setupArguments : "
                   "wrong argument type");

/*
        QL_REQUIRE(blackScholesProcess_->stateVariable->value(),
                   "OneAssetOption::setupArguments : "
                   "null underlying price given");
*/

        arguments->blackScholesProcess = blackScholesProcess_;

        arguments->exercise = exercise_;


        // shouldn't be here
        // it should be moved elsewhere
        arguments->stoppingTimes.clear();
        for (Size i=0; i<exercise_->dates().size(); i++) {
            Time time = blackScholesProcess_->riskFreeTS->dayCounter().yearFraction(
                blackScholesProcess_->riskFreeTS->referenceDate(), exercise_->date(i));
            arguments->stoppingTimes.push_back(time);
        }


    }

    void OneAssetOption::performCalculations() const {
        Option::performCalculations();
        const Greeks* results =
            dynamic_cast<const Greeks*>(engine_->results());
        QL_ENSURE(results != 0,
                  "OneAssetOption::performCalculations : "
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

        const MoreGreeks* moreResults =
            dynamic_cast<const MoreGreeks*>(engine_->results());
        QL_ENSURE(moreResults != 0,
                  "OneAssetOption::performCalculations : "
                  "no more greeks returned from pricing engine");
        /* no check on null values - just copy.
           this allows:
           a) to decide in derived options what to do when null
           results are returned (throw? numerical calculation?)
           b) to implement slim engines which only calculate the
           value---of course care must be taken not to call
           the greeks methods when using these.
        */
        deltaForward_       = moreResults->deltaForward;
        elasticity_         = moreResults->elasticity;
        thetaPerDay_        = moreResults->thetaPerDay;
        itmCashProbability_ = moreResults->itmCashProbability;

        QL_ENSURE(NPV_ != Null<double>(),
                  "OneAssetOption::performCalculations : "
                  "null value returned from option pricer");
    }


    void OneAssetOption::arguments::validate() const {
        #if defined(QL_PATCH_MICROSOFT)
        Option::arguments copy = *this;
        copy.validate();
        #else
        Option::arguments::validate();
        #endif
        /*
        QL_REQUIRE(blackScholesProcess->stateVariable != Null<double>(),
                   "OneAssetOption::arguments::validate() : "
                   "no underlying given");
        QL_REQUIRE(blackScholesProcess->stateVariable > 0.0,
                   "OneAssetOption::arguments::validate() : "
                   "negative or zero underlying given");
        */
        QL_REQUIRE(blackScholesProcess->dividendTS,
                   "OneAssetOption::arguments::validate() : "
                   "no dividend term structure given");
        QL_REQUIRE(blackScholesProcess->riskFreeTS,
                   "OneAssetOption::arguments::validate() : "
                   "no risk free term structure given");
        QL_REQUIRE(blackScholesProcess->volTS,
                   "OneAssetOption::arguments::validate() : "
                   "no vol term structure given");
    }


    OneAssetOption::ImpliedVolHelper::ImpliedVolHelper(
                               const boost::shared_ptr<PricingEngine>& engine,
                               double targetValue)
    : engine_(engine), targetValue_(targetValue) {
        OneAssetOption::arguments* arguments_ =
            dynamic_cast<OneAssetOption::arguments*>(engine_->arguments());
        QL_REQUIRE(arguments_ != 0,
                   "OneAssetOption::ImpliedVolHelper::ImpliedVolHelper : "
                   "pricing engine does not supply needed arguments");
        vol_ = boost::shared_ptr<SimpleQuote>(new SimpleQuote(0.0));
        arguments_->blackScholesProcess->volTS = 
            RelinkableHandle<BlackVolTermStructure>(
                boost::shared_ptr<BlackVolTermStructure>(
                    new BlackConstantVol(arguments_->blackScholesProcess
                                         ->volTS->referenceDate(),
                                         RelinkableHandle<Quote>(vol_))));
        results_ = dynamic_cast<const Value*>(engine_->results());
        QL_REQUIRE(results_ != 0,
                   "OneAssetOption::ImpliedVolHelper::ImpliedVolHelper : "
                   "pricing engine does not supply needed results");
    }

    double OneAssetOption::ImpliedVolHelper::operator()(double x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

}

