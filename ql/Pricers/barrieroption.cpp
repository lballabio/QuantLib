
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

namespace QuantLib {

    namespace Pricers {

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
                    ")< barrier(" +
                    DoubleFormatter::toString(barrier_) +
                    "): down-and-in barrier undefined");
                break;
              case Barrier::UpIn:
                QL_REQUIRE(underlying_ <= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ")> barrier("+
                    DoubleFormatter::toString(barrier_) +
                    "): up-and-in barrier undefined");
                break;
              case Barrier::DownOut:
                QL_REQUIRE(underlying_ >= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ")< barrier("+
                    "): down-and-out barrier undefined");
                break;
              case Barrier::UpOut:
                QL_REQUIRE(underlying_ <= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ")> barrier("+
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
            initialize_();
            switch (payoff_.optionType()) {
              case Option::Call:
                switch (barrType_) {
                  case Barrier::DownIn:
                    if(payoff_.strike() >= barrier_)
                        value_ = C_(1,1) + E_(1,1);
                    else
                        value_ = A_(1,1) - B_(1,1) + D_(1,1) + E_(1,1);
                    break;
                  case Barrier::UpIn:
                    if(payoff_.strike() >= barrier_)
                        value_ = A_(-1,1) + E_(-1,1);
                    else
                        value_ = B_(-1,1) - C_(-1,1) + D_(-1,1) + E_(-1,1);
                    break;
                  case Barrier::DownOut:
                    if(payoff_.strike() >= barrier_)
                        value_ = A_(1,1) - C_(1,1) + F_(1,1);
                    else
                        value_ = B_(1,1) - D_(1,1) + F_(1,1);
                    break;
                  case Barrier::UpOut:
                    if(payoff_.strike() >= barrier_)
                        value_ = F_(-1,1);
                    else
                        value_ = A_(-1,1) - B_(-1,1)+C_(-1,1)-D_(-1,1)+F_(-1,1);
                    break;
                }
                break;
              case Option::Put:
                switch (barrType_) {
                  case Barrier::DownIn:
                    if(payoff_.strike() >= barrier_)
                        value_ = B_(1,-1) - C_(1,-1) + D_(1,-1) + E_(1,-1);
                    else
                        value_ = A_(1,-1) + E_(1,-1);
                    break;
                  case Barrier::UpIn:
                    if(payoff_.strike() >= barrier_)
                        value_ = A_(-1,-1) - B_(-1,-1) + D_(-1,-1) + E_(-1,-1);
                    else
                        value_ = C_(-1,-1) + E_(-1,-1);
                    break;
                  case Barrier::DownOut:
                    if(payoff_.strike() >= barrier_)
                        value_ = A_(1,-1) - B_(1,-1)+C_(1,-1)-D_(1,-1)+F_(1,-1);
                    else
                        value_ = F_(1,-1);
                    break;
                  case Barrier::UpOut:
                    if(payoff_.strike() >= barrier_)
                        value_ = B_(-1,-1) - D_(-1,-1) + F_(-1,-1);
                    else
                        value_ = A_(-1,-1) - C_(-1,-1) + F_(-1,-1);
                    break;
                }
                break;
              case Option::Straddle:
                switch (barrType_) {
                  case Barrier::DownIn:
                    if(payoff_.strike() >= barrier_)
                        value_ = C_(1,1) + E_(1,1) +
                            B_(1,-1) - C_(1,-1) + D_(1,-1) + E_(1,-1);
                    else
                        value_ = A_(1,1) - B_(1,1) + D_(1,1) + E_(1,1) +
                            A_(1,-1) + E_(1,-1);
                    break;
                  case Barrier::UpIn:
                    if(payoff_.strike() >= barrier_)
                        value_ = A_(-1,1) + E_(-1,1) +
                            A_(-1,-1) - B_(-1,-1) + D_(-1,-1) + E_(-1,-1);
                    else
                        value_ = B_(-1,1) - C_(-1,1) + D_(-1,1) + E_(-1,1) +
                            C_(-1,-1) + E_(-1,-1);
                    break;
                  case Barrier::DownOut:
                    if(payoff_.strike() >= barrier_)
                        value_ = A_(1,1) - C_(1,1) + F_(1,1) +
                            A_(1,-1) - B_(1,-1)+C_(1,-1)-D_(1,-1)+F_(1,-1);
                    else
                        value_ = B_(1,1) - D_(1,1) + F_(1,1) +
                            F_(1,-1);
                    break;
                  case Barrier::UpOut:
                    if(payoff_.strike() >= barrier_)
                        value_ = F_(-1,1) +
                            B_(-1,-1) - D_(-1,-1) + F_(-1,-1);
                    else
                        value_ = A_(-1,1) -B_(-1,1)+C_(-1,1)-D_(-1,1)+F_(-1,1) +
                            A_(-1,-1) - C_(-1,-1) + F_(-1,-1);
                    break;
                }
                break;
              default:
                throw Error("Option: unknown type");
            }
            hasBeenCalculated_ = true;
          }
          return value_;
        }

        void BarrierOption::calculate_() const{

            double underPlus = underlying_ * (1 + 0.0001);
            double underMinu = underlying_ * (1 - 0.0001);
            double timePlus = residualTime_ * (1 + 0.0001);
            BarrierOption barrierTimePlus(barrType_, payoff_.optionType(), underlying_,
                payoff_.strike(), dividendYield_, riskFreeRate_, timePlus, volatility_,
                barrier_, rebate_);

            BarrierOption barrierPlus(barrType_, payoff_.optionType(), underPlus, payoff_.strike(),
                dividendYield_, riskFreeRate_, residualTime_, volatility_,
                barrier_, rebate_);

            BarrierOption barrierMinus(barrType_, payoff_.optionType(), underMinu, payoff_.strike(),
                dividendYield_, riskFreeRate_, residualTime_, volatility_,
                barrier_, rebate_);

            delta_ = (barrierPlus.value()-barrierMinus.value())/
                     (underPlus-underMinu);

            gamma_ = (barrierPlus.value()
                      + barrierMinus.value() - 2.0 * value())/
                       ((underPlus-underlying_)*(underlying_-underMinu));
            theta_ = -(barrierTimePlus.value() - value())/
                            (timePlus - residualTime_);
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
