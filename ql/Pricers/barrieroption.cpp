
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

/*! \file barrieroption.cpp
    \brief barrier option
*/

#include <ql/Pricers/barrieroption.hpp>
#include <ql/Instruments/barrieroption.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>
#include <ql/DayCounters/simpledaycounter.hpp>

namespace QuantLib {

    namespace Pricers {

        using namespace DayCounters;
        using namespace Instruments;
        using namespace PricingEngines;
        using namespace TermStructures;
        using namespace VolTermStructures;

        BarrierOption::BarrierOption(Barrier::Type barrType,
                                     Option::Type type,
                                     double underlying,
                                     double strike,
                                     Spread dividendYield,
                                     Rate riskFreeRate,
                                     Time residualTime,
                                     double volatility,
                                     double barrier,
                                     double rebate)
        : SingleAssetOption(type, underlying, strike, dividendYield,
                            riskFreeRate, residualTime, volatility), 
            barrType_(barrType),
            barrier_(barrier), rebate_(rebate), f_() {

            QL_REQUIRE(barrier_ > 0,
                "BarrierOption: barrier must be positive");
            QL_REQUIRE(rebate_ >= 0,
                "BarrierOption: rebate cannot be neagative");

            switch (barrType_) {
              case Barrier::DownIn:
                QL_REQUIRE(underlying_ >= barrier_, "underlying (" +
                    DoubleFormatter::toString(underlying_) +
                    ") < barrier (" +
                    DoubleFormatter::toString(barrier_) +
                    "): down-and-in barrier undefined");
                break;
              case Barrier::UpIn:
                QL_REQUIRE(underlying_ <= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ") > barrier ("+
                    DoubleFormatter::toString(barrier_) +
                    "): up-and-in barrier undefined");
                break;
              case Barrier::DownOut:
                QL_REQUIRE(underlying_ >= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ") < barrier ("+
                    DoubleFormatter::toString(barrier_) +
                    "): down-and-out barrier undefined");
                break;
              case Barrier::UpOut:
                QL_REQUIRE(underlying_ <= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ") > barrier ("+
                    DoubleFormatter::toString(barrier_) +
                    "): up-and-out barrier undefined");
                break;
              default:
                throw Error("Barrier Option: unknown type");
            }

        }

        void BarrierOption::initialize_() const {
            sigmaSqrtT_ = volatility_ * QL_SQRT(residualTime_);

            mu_ = (riskFreeRate_ - dividendYield_)/
                                (volatility_ * volatility_) - 0.5;
            muSigma_ = (1 + mu_) * sigmaSqrtT_;
            dividendDiscount_ = QL_EXP(-dividendYield_*residualTime_);
            riskFreeDiscount_ = QL_EXP(-riskFreeRate_*residualTime_);
            greeksCalculated_ = false;
        }

        double BarrierOption::value() const {
            if(!hasBeenCalculated_) {
                Instruments::BarrierOption::arguments* args =
                    dynamic_cast<Instruments::BarrierOption::arguments*>(
                                                        engine_.arguments());

                Date today = Date::todaysDate();

                args->payoff = Handle<Payoff>(new PlainVanillaPayoff(payoff_));
                args->underlying = underlying_;
                args->riskFreeTS.linkTo(Handle<TermStructure>(
                    new FlatForward(today,today,riskFreeRate_,
                                    SimpleDayCounter())));
                args->dividendTS.linkTo(Handle<TermStructure>(
                    new FlatForward(today,today,dividendYield_,
                                    SimpleDayCounter())));
                args->volTS.linkTo(Handle<BlackVolTermStructure>(
                    new BlackConstantVol(today,volatility_,
                                         SimpleDayCounter())));
                args->maturity = residualTime_;

                args->barrierType = barrType_;
                args->barrier = barrier_;
                args->rebate = rebate_;

                engine_.calculate();

                const Value* result =
                    dynamic_cast<const Value*>(engine_.results());
                value_ = result->value;

                hasBeenCalculated_ = true;
            }
            return value_;
        }

        void BarrierOption::calculate_() const{

            double refValue = value();
            Instruments::BarrierOption::arguments* args =
                dynamic_cast<Instruments::BarrierOption::arguments*>(
                                                        engine_.arguments());
            const Value* result =
                dynamic_cast<const Value*>(engine_.results());
            
            double du = underlying_ * 1.0e-4;
            double dt = residualTime_ * 1.0e-4;

            args->underlying = underlying_ + du;
            engine_.calculate();
            double valuePlus = result->value;

            args->underlying = underlying_ - du;
            engine_.calculate();
            double valueMinus = result->value;
            
            args->maturity = residualTime_ + dt;
            engine_.calculate();
            double valueTimePlus = result->value;

            delta_ = (valuePlus-valueMinus)/(2*du);

            gamma_ = (valuePlus + valueMinus - 2.0 * refValue)/(du*du);
            theta_ = -(valueTimePlus - refValue)/dt;
            greeksCalculated_ = true;
        }

        double BarrierOption::delta() const {
            if (!greeksCalculated_)
                calculate_();
            return delta_;
        }

        double BarrierOption::gamma() const {
            if(!greeksCalculated_)
                calculate_();
            return gamma_;
        }

        double BarrierOption::theta() const {
            if(!greeksCalculated_)
                calculate_();
            return theta_;
        }

    }

}
