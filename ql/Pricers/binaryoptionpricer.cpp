
/*
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

/*! \file binaryoptionpricer.cpp
    \brief European style cash-or-nothing option pricer
*/

#include <ql/Pricers/binaryoptionpricer.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    using namespace Instruments;
    using namespace DayCounters;
    using namespace TermStructures;
    using namespace VolTermStructures;

    namespace Pricers {

        BinaryOption::BinaryOption(Option::Type type, double underlying,
                                   double strike, Spread dividendYield,
                                   Rate riskFreeRate, Time residualTime,
                                   double volatility, double cashPayoff)
        : SingleAssetOption(type, underlying, strike, dividendYield,
                            riskFreeRate, residualTime, volatility) {

            Instruments::BinaryOption::arguments* args =
                dynamic_cast<Instruments::BinaryOption::arguments*>(
                    engine_.arguments());

            Date today = Date::todaysDate();

            args->payoff = Handle<Payoff>(
                new PlainVanillaPayoff(type,strike));

            args->binaryType = Binary::CashAtExpiry;
            args->barrier = strike;
            args->cashPayoff = cashPayoff;

            args->underlying = underlying;

            args->riskFreeTS.linkTo(Handle<TermStructure>(
                new FlatForward(today,today,riskFreeRate_,
                                SimpleDayCounter())));
            args->dividendTS.linkTo(Handle<TermStructure>(
                new FlatForward(today,today,dividendYield_,
                                SimpleDayCounter())));

            args->maturity = residualTime;

            args->exerciseType = Exercise::European;
            args->stoppingTimes = std::vector<Time>(1,residualTime);

            args->volTS.linkTo(Handle<BlackVolTermStructure>(
                new BlackConstantVol(today,volatility_,
                                     SimpleDayCounter())));

            args->validate();
            engine_.calculate();
        }

        Handle<SingleAssetOption> BinaryOption::clone() const {
            return Handle<SingleAssetOption>(new BinaryOption(*this));
        }

        double BinaryOption::value() const {
            const Value* result = 
                dynamic_cast<const Value*>(engine_.results());
            return result->value;
        }

        double BinaryOption::delta() const {
            const Greeks* results = 
                dynamic_cast<const Greeks*>(engine_.results());
            return results->delta;
        }

        double BinaryOption::gamma() const {
            const Greeks* results = 
                dynamic_cast<const Greeks*>(engine_.results());
            return results->gamma;
        }

        double BinaryOption::theta() const {
            const Greeks* results = 
                dynamic_cast<const Greeks*>(engine_.results());
            return results->theta;
        }

        double BinaryOption::rho() const {
            const Greeks* results = 
                dynamic_cast<const Greeks*>(engine_.results());
            return results->rho;
        }

        double BinaryOption::dividendRho() const {
            const Greeks* results = 
                dynamic_cast<const Greeks*>(engine_.results());
            return results->dividendRho;
        }

        double BinaryOption::vega() const {
            const Greeks* results = 
                dynamic_cast<const Greeks*>(engine_.results());
            return results->vega;
        }

    }

}

