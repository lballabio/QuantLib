
/*
 Copyright (C) 2003 Neil Firth
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

/*! \file binaryoption.cpp
    \brief Binary option on a single asset
*/

#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/Instruments/binaryoption.hpp>

using QuantLib::VolTermStructures::BlackConstantVol;

namespace QuantLib {

    namespace Instruments {

        void BinaryOption::arguments::validate() const {
            #if defined(QL_PATCH_MICROSOFT)
            VanillaOption::arguments copy = *this;
            copy.validate();
            #else
            VanillaOption::arguments::validate();
            #endif
            QL_REQUIRE(barrier != Null<double>(),
                       "BinaryOption: no barrier given");
            QL_REQUIRE(cashPayoff != Null<double>(),
                       "BinaryOption: no cash payoff given");
        }

        BinaryOption::BinaryOption(Binary::Type binaryType,
            double barrier,
            double cashPayoff,
            Option::Type type,
            const RelinkableHandle<MarketElement>& underlying,
            const RelinkableHandle<TermStructure>& dividendTS,
            const RelinkableHandle<TermStructure>& riskFreeTS,
            const Exercise& exercise,
            const RelinkableHandle<BlackVolTermStructure>& volTS,
            const Handle<PricingEngine>& engine,
            const std::string& isinCode, const std::string& description)
        : Option(engine, isinCode, description),
          binaryType_(binaryType), barrier_(barrier), cashPayoff_(cashPayoff),
          type_(type), underlying_(underlying), exercise_(exercise), 
          riskFreeTS_(riskFreeTS), dividendTS_(dividendTS),
          volTS_(volTS) {
            registerWith(underlying_);
            registerWith(dividendTS_);
            registerWith(riskFreeTS_);
            registerWith(volTS_);
        }

        double BinaryOption::delta() const {
            calculate();
            QL_REQUIRE(delta_ != Null<double>(),
                       "BinaryOption: delta not provided");
            return delta_;
        }

        double BinaryOption::gamma() const {
            calculate();
            QL_REQUIRE(gamma_ != Null<double>(),
                       "BinaryOption: gamma not provided");
            return gamma_;
        }

        double BinaryOption::theta() const {
            calculate();
            QL_REQUIRE(theta_ != Null<double>(),
                       "BinaryOption: theta not provided");
            return theta_;
        }

        double BinaryOption::vega() const {
            calculate();
            QL_REQUIRE(vega_ != Null<double>(),
                       "BinaryOption: vega not provided");
            return vega_;
        }

        double BinaryOption::rho() const {
            calculate();
            QL_REQUIRE(rho_ != Null<double>(),
                       "BinaryOption: rho not provided");
            return rho_;
        }

        double BinaryOption::dividendRho() const {
            calculate();
            QL_REQUIRE(dividendRho_ != Null<double>(),
                       "BinaryOption: dividend rho not provided");
            return dividendRho_;
        }

        double BinaryOption::strikeSensitivity() const {
            calculate();
            QL_REQUIRE(strikeSensitivity_ != Null<double>(),
                       "BinaryOption: strike sensitivity not provided");
            return strikeSensitivity_;
        }

        bool BinaryOption::isExpired() const {
            return exercise_.lastDate() < riskFreeTS_->referenceDate();
        }

        void BinaryOption::setupExpired() const {
            NPV_ = delta_ = gamma_ = theta_ =
                   vega_ = rho_ = dividendRho_ = strikeSensitivity_ = 0.0;
        }

        void BinaryOption::setupArguments(Arguments* args) const {
            BinaryOption::arguments* arguments =
                dynamic_cast<BinaryOption::arguments*>(args);
            QL_REQUIRE(arguments != 0,
                       "BinaryOption::setupArguments : "
                       "Arguments wrong type");

            arguments->payoff = Handle<Payoff>(
                new PlainVanillaPayoff(type_,barrier_));

            arguments->binaryType = binaryType_;
            arguments->barrier = barrier_;
            arguments->cashPayoff = cashPayoff_;

            QL_REQUIRE(!underlying_.isNull(),
                       "BinaryOption::setupEngine : "
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

        void BinaryOption::performCalculations() const {
            Option::performCalculations();
            const Greeks* results =
                dynamic_cast<const Greeks*>(engine_->results());
            QL_ENSURE(results != 0,
                        "BinaryOption::performCalculations : "
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
                "BinaryOption::performCalculations : "
                "null value returned from option pricer");
        }

    }

}

