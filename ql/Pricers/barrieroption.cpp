
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file barrieroption.cpp
    \brief barrier option

    \fullpath
    ql/Pricers/%barrieroption.cpp
*/

// $Id$

#include <ql/Pricers/barrieroption.hpp>

namespace QuantLib {

    namespace Pricers {

        BarrierOption::BarrierOption(BarrierType barrType,
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
            riskFreeRate, residualTime, volatility), barrType_(barrType),
            barrier_(barrier), rebate_(rebate), f_() {

            QL_REQUIRE(barrier_ > 0,
                "BarrierOption: barrier must be positive");
            QL_REQUIRE(rebate_ >= 0,
                "BarrierOption: rebate cannot be neagative");

            switch (barrType_) {
              case DownIn:
                QL_REQUIRE(underlying_ >= barrier_, "underlying (" +
                    DoubleFormatter::toString(underlying_) +
                    ")< barrier(" +
                    DoubleFormatter::toString(barrier_) +
                    "): down-and-in barrier undefined");
                break;
              case UpIn:
                QL_REQUIRE(underlying_ <= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ")> barrier("+
                    DoubleFormatter::toString(barrier_) +
                    "): up-and-in barrier undefined");
                break;
              case DownOut:
                QL_REQUIRE(underlying_ >= barrier_, "underlying ("+
                    DoubleFormatter::toString(underlying_) +
                    ")< barrier("+
                    "): down-and-out barrier undefined");
                break;
              case UpOut:
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
            switch (type_) {
              case Option::Call:
                switch (barrType_) {
                  case DownIn:
                    if(strike_ >= barrier_)
                        value_ = C_(1,1) + E_(1,1);
                    else
                        value_ = A_(1,1) - B_(1,1) + D_(1,1) + E_(1,1);
                    break;
                  case UpIn:
                    if(strike_ >= barrier_)
                        value_ = A_(-1,1) + E_(-1,1);
                    else
                        value_ = B_(-1,1) - C_(-1,1) + D_(-1,1) + E_(-1,1);
                    break;
                  case DownOut:
                    if(strike_ >= barrier_)
                        value_ = A_(1,1) - C_(1,1) + F_(1,1);
                    else
                        value_ = B_(1,1) - D_(1,1) + F_(1,1);
                    break;
                  case UpOut:
                    if(strike_ >= barrier_)
                        value_ = F_(-1,1);
                    else
                        value_ = A_(-1,1) - B_(-1,1)+C_(-1,1)-D_(-1,1)+F_(-1,1);
                    break;
                }
                break;
              case Option::Put:
                switch (barrType_) {
                  case DownIn:
                    if(strike_ >= barrier_)
                        value_ = B_(1,-1) - C_(1,-1) + D_(1,-1) + E_(1,-1);
                    else
                        value_ = A_(1,-1) + E_(1,-1);
                    break;
                  case UpIn:
                    if(strike_ >= barrier_)
                        value_ = A_(-1,-1) - B_(-1,-1) + D_(-1,-1) + E_(-1,-1);
                    else
                        value_ = C_(-1,-1) + E_(-1,-1);
                    break;
                  case DownOut:
                    if(strike_ >= barrier_)
                        value_ = A_(1,-1) - B_(1,-1)+C_(1,-1)-D_(1,-1)+F_(1,-1);
                    else
                        value_ = F_(1,-1);
                    break;
                  case UpOut:
                    if(strike_ >= barrier_)
                        value_ = B_(-1,-1) - D_(-1,-1) + F_(-1,-1);
                    else
                        value_ = A_(-1,-1) - C_(-1,-1) + F_(-1,-1);
                    break;
                }
                break;
              case Option::Straddle:
                switch (barrType_) {
                  case DownIn:
                    if(strike_ >= barrier_)
                        value_ = C_(1,1) + E_(1,1) +
                            B_(1,-1) - C_(1,-1) + D_(1,-1) + E_(1,-1);
                    else
                        value_ = A_(1,1) - B_(1,1) + D_(1,1) + E_(1,1) +
                            A_(1,-1) + E_(1,-1);
                    break;
                  case UpIn:
                    if(strike_ >= barrier_)
                        value_ = A_(-1,1) + E_(-1,1) +
                            A_(-1,-1) - B_(-1,-1) + D_(-1,-1) + E_(-1,-1);
                    else
                        value_ = B_(-1,1) - C_(-1,1) + D_(-1,1) + E_(-1,1) +
                            C_(-1,-1) + E_(-1,-1);
                    break;
                  case DownOut:
                    if(strike_ >= barrier_)
                        value_ = A_(1,1) - C_(1,1) + F_(1,1) +
                            A_(1,-1) - B_(1,-1)+C_(1,-1)-D_(1,-1)+F_(1,-1);
                    else
                        value_ = B_(1,1) - D_(1,1) + F_(1,1) +
                            F_(1,-1);
                    break;
                  case UpOut:
                    if(strike_ >= barrier_)
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
            BarrierOption barrierTimePlus(barrType_, type_, underlying_,
                strike_, dividendYield_, riskFreeRate_, timePlus, volatility_,
                barrier_, rebate_);

            BarrierOption barrierPlus(barrType_, type_, underPlus, strike_,
                dividendYield_, riskFreeRate_, residualTime_, volatility_,
                barrier_, rebate_);

            BarrierOption barrierMinus(barrType_, type_, underMinu, strike_,
                dividendYield_, riskFreeRate_, residualTime_, volatility_,
                barrier_, rebate_);

            delta_ = (barrierPlus.value()-barrierMinus.value())/
                     (underPlus-underMinu);

            gamma_ = (barrierPlus.value()
                      + barrierMinus.value() - 2.0 * value())/
                       ((underPlus-underlying_)*(underlying_-underMinu));
            theta_ = (barrierTimePlus.value() - value())/
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
