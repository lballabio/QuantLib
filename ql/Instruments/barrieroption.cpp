
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file barrieroption.cpp
    \brief Barrier option on a single asset
*/

#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Instruments/barrieroption.hpp>

using QuantLib::VolTermStructures::BlackConstantVol;

namespace QuantLib {

    namespace Instruments {

        BarrierOption::BarrierOption(
            Barrier::Type barrierType,
            double barrier,
            double rebate,
            Option::Type type,
            const RelinkableHandle<MarketElement>& underlying,
            double strike,
            const RelinkableHandle<TermStructure>& dividendTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const Exercise& exercise,
            const RelinkableHandle<BlackVolTermStructure>& volTS,
            const Handle<PricingEngine>& engine,
            const std::string& isinCode, const std::string& description)
        : Option(engine, isinCode, description),
          barrierType_(barrierType), barrier_(barrier), rebate_(rebate),
          type_(type), underlying_(underlying),
          strike_(strike), exercise_(exercise), 
          riskFreeTS_(riskFreeTS), dividendTS_(dividendTS),
          volTS_(volTS) {
            registerWith(underlying_);
            registerWith(dividendTS_);
            registerWith(riskFreeTS_);
            registerWith(volTS_);
        }

        double BarrierOption::delta() const {
            calculate();
            QL_REQUIRE(delta_ != Null<double>(),
                       "BarrierOption: delta not provided");
            return delta_;
        }

        double BarrierOption::gamma() const {
            calculate();
            QL_REQUIRE(gamma_ != Null<double>(),
                       "BarrierOption: gamma not provided");
            return gamma_;
        }

        double BarrierOption::theta() const {
            calculate();
            QL_REQUIRE(theta_ != Null<double>(),
                       "BarrierOption: theta not provided");
            return theta_;
        }

        double BarrierOption::vega() const {
            calculate();
            QL_REQUIRE(vega_ != Null<double>(),
                       "BarrierOption: vega not provided");
            return vega_;
        }

        double BarrierOption::rho() const {
            calculate();
            QL_REQUIRE(rho_ != Null<double>(),
                       "BarrierOption: rho not provided");
            return rho_;
        }

        double BarrierOption::dividendRho() const {
            calculate();
            QL_REQUIRE(dividendRho_ != Null<double>(),
                       "BarrierOption: dividend rho not provided");
            return dividendRho_;
        }

        double BarrierOption::strikeSensitivity() const {
            calculate();
            QL_REQUIRE(strikeSensitivity_ != Null<double>(),
                       "BarrierOption: strike sensitivity not provided");
            return strikeSensitivity_;
        }

        bool BarrierOption::isExpired() const {
            return exercise_.lastDate() < riskFreeTS_->referenceDate();
        }

        void BarrierOption::setupExpired() const {
            NPV_ = delta_ = gamma_ = theta_ =
                   vega_ = rho_ = dividendRho_ = strikeSensitivity_ = 0.0;
        }

        void BarrierOption::setupEngine() const {
            BarrierOptionArguments* arguments =
                dynamic_cast<BarrierOptionArguments*>(
                    engine_->arguments());
            QL_REQUIRE(arguments != 0,
                       "BarrierOption::setupEngine : "
                       "pricing engine does not supply needed arguments");

            arguments->payoff = Handle<Payoff>(
                new PlainVanillaPayoff(type_,strike_));

            arguments->barrierType = barrierType_;
            arguments->barrier = barrier_;
            arguments->rebate = rebate_;

            QL_REQUIRE(!underlying_.isNull(),
                       "BarrierOption::setupEngine : "
                       "null underlying price given");
            arguments->underlying = underlying_->value();

            // should I require !IsNull(TS) ???
            arguments->dividendTS = dividendTS_;
            arguments->riskFreeTS = riskFreeTS_;

            arguments->maturity = riskFreeTS_->dayCounter().yearFraction(
                riskFreeTS_->referenceDate(), exercise_.lastDate());
            arguments->exerciseType = exercise_.type();
            arguments->stoppingTimes = 
                std::vector<Time>(exercise_.dates().size());
            for (Size i=0; i<exercise_.dates().size(); i++) {
                arguments->stoppingTimes[i] = 
                    riskFreeTS_->dayCounter().yearFraction(
                        riskFreeTS_->referenceDate(), exercise_.date(i));
            }

            arguments->volTS = volTS_;
        }

        void BarrierOption::performCalculations() const {
            Option::performCalculations();
            const OptionGreeks* results =
                dynamic_cast<const OptionGreeks*>(engine_->results());
            QL_ENSURE(results != 0,
                      "BarrierOption::performCalculations : "
                      "no greeks returned from pricing engine");
            /* no check on null values - just copy.
               this allows:
               a) to decide in derived options what to do when null 
                  results are returned (throw? numerical calculation?)
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
            strikeSensitivity_ = results->strikeSensitivity;

            QL_ENSURE(NPV_ != Null<double>(),
                      "BarrierOption::performCalculations : "
                      "null value returned from option pricer");
        }
        
    }

}

