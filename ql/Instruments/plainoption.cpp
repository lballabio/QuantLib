

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file plainoption.cpp
    \brief Plain (no dividends, no barriers) option on a single asset

    \fullpath
    ql/Instruments/%plainoption.cpp
*/

// $Id$

#include <ql/Instruments/plainoption.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace Instruments {

        PlainOption::PlainOption(Option::Type type,
            const RelinkableHandle<MarketElement>& underlying,
            double strike,
            const RelinkableHandle<TermStructure>& dividendYield,
            const RelinkableHandle<TermStructure>& riskFreeRate,
            const Date& exerciseDate,
            const RelinkableHandle<MarketElement>& volatility,
            const Handle<OptionPricingEngine>& engine,
            const std::string& isinCode, const std::string& description)
        : Option(engine, isinCode, description), type_(type),
          underlying_(underlying), strike_(strike),
          dividendYield_(dividendYield), riskFreeRate_(riskFreeRate),
          exerciseDate_(exerciseDate), volatility_(volatility) {
            underlying_.registerObserver(this);
            dividendYield_.registerObserver(this);
            riskFreeRate_.registerObserver(this);
            volatility_.registerObserver(this);
        }

    PlainOption::~PlainOption() {
            underlying_.unregisterObserver(this);
            dividendYield_.unregisterObserver(this);
            riskFreeRate_.unregisterObserver(this);
            volatility_.unregisterObserver(this);
        }

        double PlainOption::delta() const {
            calculate();
            QL_REQUIRE(delta_ != Null<double>(),
                       "delta calculation failed");
            return delta_;
    }

    double PlainOption::gamma() const {
            calculate();
            QL_REQUIRE(gamma_ != Null<double>(),
                       "gamma calculation failed");
            return gamma_;
    }

    double PlainOption::theta() const {
            calculate();
            QL_REQUIRE(theta_ != Null<double>(),
                       "theta calculation failed");
            return theta_;
    }

    double PlainOption::vega() const {
            calculate();
            QL_REQUIRE(vega_ != Null<double>(),
                       "vega calculation failed");
            return vega_;
    }

        double PlainOption::rho() const {
            calculate();
            QL_REQUIRE(rho_ != Null<double>(),
                       "rho calculation failed");
            return rho_;
    }

        double PlainOption::dividendRho() const {
            calculate();
            QL_REQUIRE(dividendRho_ != Null<double>(),
                       "dividend rho calculation failed");
            return dividendRho_;
    }

        double PlainOption::impliedVolatility(double targetValue,
          double accuracy, Size maxEvaluations,
          double minVol, double maxVol) const {
        double value = NPV(), vol = volatility_->value();
        QL_REQUIRE(!isExpired_, "option expired");
        if (value == targetValue) {
        return vol;
        } else {
        ImpliedVolHelper f(engine_,targetValue);
        Solvers1D::Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f,accuracy,vol,minVol,maxVol);
        }
    }

    void PlainOption::setupEngine() const {
            PlainOptionParameters* parameters =
                dynamic_cast<PlainOptionParameters*>(
                    engine_->parameters());
            QL_REQUIRE(parameters != 0,
                       "pricing engine does not supply needed parameters");

            parameters->type = type_;

            QL_REQUIRE(!underlying_.isNull(), "null underlying price given");
            parameters->underlying = underlying_->value();

            parameters->strike = strike_;

            if (dividendYield_.isNull())
                parameters->dividendYield = 0.0;
            else
                parameters->dividendYield =
                dividendYield_->zeroYield(exerciseDate_);

            QL_REQUIRE(!riskFreeRate_.isNull(), "null risk free rate given");
            parameters->riskFreeRate =
                riskFreeRate_->zeroYield(exerciseDate_);

            parameters->residualTime =
                riskFreeRate_->dayCounter().yearFraction(
                    riskFreeRate_->settlementDate(), exerciseDate_);

            QL_REQUIRE(!volatility_.isNull(), "null volatility given");
            parameters->volatility = volatility_->value();
        }

    void PlainOption::performCalculations() const {
            if (exerciseDate_ <= riskFreeRate_->settlementDate()) {
                isExpired_ = true;
                NPV_ = delta_ = gamma_ = theta_ =
                    vega_ = rho_ = dividendRho_ = 0.0;
            } else {
                isExpired_ = false;
                Option::performCalculations();
                const OptionGreeks* results =
                    dynamic_cast<const OptionGreeks*>(engine_->results());
                QL_ENSURE(results != 0,
                          "no greeks returned from option pricer");
                /* no check on     null values - just copy.
                   this allows:
                   a) to decide in derived options what to do when null results
                      are returned (throw? numerical calculation?)
                   b) to implement slim engines which only calculate the
                      value---of course care must be taken not to call
                      the greeks methods when using these.
                */
                delta_       = results->delta;
                gamma_       = results->gamma;
                theta_       = results->theta;
                vega_        = results->vega;
                rho_         = results->rho;
                dividendRho_ = results->dividendRho;
            }
            QL_ENSURE(isExpired_ || NPV_ != Null<double>(),
                      "null value returned from option pricer");
        }


        PlainOption::ImpliedVolHelper::ImpliedVolHelper(
            const Handle<OptionPricingEngine>& engine, double targetValue)
        : engine_(engine), targetValue_(targetValue) {
            parameters_ = dynamic_cast<PlainOptionParameters*>(
                engine_->parameters());
            QL_REQUIRE(parameters_ != 0,
                       "pricing engine does not supply needed parameters");
            results_ = dynamic_cast<const OptionValue*>(
                engine_->results());
            QL_REQUIRE(results_ != 0,
                       "pricing engine does not supply needed results");
        }

        double PlainOption::ImpliedVolHelper::operator()(double x) const {
            parameters_->volatility = x;
            engine_->calculate();
            return results_->value-targetValue_;
        }

    }

    namespace Pricers {

        Arguments* PlainOptionEngine::parameters() {
            return &parameters_;
        }

    void PlainOptionEngine::validateParameters() const {
            QL_REQUIRE(parameters_.type != Option::Type(-1),
                       "no option type given");
            QL_REQUIRE(parameters_.underlying != Null<double>(),
                       "null underlying given");
            QL_REQUIRE(parameters_.strike != Null<double>(),
                       "null strike given");
            QL_REQUIRE(parameters_.dividendYield != Null<double>(),
                       "null dividend yield given");
            QL_REQUIRE(parameters_.riskFreeRate != Null<double>(),
                       "null risk free rate given");
            QL_REQUIRE(parameters_.residualTime != Null<double>(),
                       "null residual time given");
            QL_REQUIRE(parameters_.volatility != Null<double>(),
                       "null volatility given");
        }

    const Results* PlainOptionEngine::results() const {
        return &results_;
    }

    }

}

