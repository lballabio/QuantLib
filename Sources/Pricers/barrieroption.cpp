
/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 *  http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file barrieroption.cpp
    \brief barrier option

    $Source$
    $Log$
    Revision 1.4  2001/03/28 12:40:42  lballabio
    Added constness to initialize() and mutableness to data members (how did this compile before?)

    Revision 1.3  2001/03/27 17:19:12  marmar
    Bug fixed in calculation of rho and vega

    Revision 1.2  2001/03/22 16:37:33  marmar
    Barrier option greeks included

    Revision 1.1  2001/02/20 11:09:59  marmar
    BarrierOption implements the analytical barrier option

*/

#include "barrieroption.h"

namespace QuantLib {

    namespace Pricers {

        BarrierOption::BarrierOption(BarrierType barrType,
                Type type, double underlying, double strike,
                Rate dividendYield, Rate riskFreeRate,
                Time residualTime, double volatility,
                double barrier, double rebate)
        : BSMOption(type, underlying, strike, dividendYield,
              riskFreeRate, residualTime, volatility), barrType_(barrType),
              barrier_(barrier), rebate_(rebate), f_(){

            QL_REQUIRE(type != Straddle,
                "BarrierOption: Straddle is meaningless for barrier options");
            QL_REQUIRE(barrier_ > 0,
                "BarrierOption: barrier must be positive");
            QL_REQUIRE(rebate_ >= 0,
                "BarrierOption: rebate cannot be neagative");

        }
        
        void BarrierOption::initialize() const {
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
            initialize();
            switch (type_) {
              case Call:
                switch (barrType_) {
                  case DownIn:
                    if(strike_ >= barrier_)
                        value_ = C(1,1) + E(1,1);
                    else
                        value_ = A(1,1) - B(1,1) + D(1,1) + E(1,1);
                    break;
                  case UpIn:
                    if(strike_ >= barrier_)
                        value_ = A(-1,1) + E(-1,1);
                    else
                        value_ = B(-1,1) - C(-1,1) + D(-1,1) + E(-1,1);
                    break;
                  case DownOut:
                    if(strike_ >= barrier_)
                        value_ = A(1,1) - C(1,1) + F(1,1);
                    else
                        value_ = B(1,1) - D(1,1) + F(1,1);
                    break;
                  case UpOut:
                    if(strike_ >= barrier_)
                        value_ = F(-1,1);
                    else
                        value_ = A(-1,1) - B(-1,1) + C(-1,1) - D(-1,1)+ F(-1,1);
                    break;
                }
                break;
              case Put:
                switch (barrType_) {
                  case DownIn:
                    if(strike_ >= barrier_)
                        value_ = B(1,-1) - C(1,-1) + D(1,-1) + E(1,-1);
                    else
                        value_ = A(1,-1) + E(1,-1);
                    break;
                  case UpIn:
                    if(strike_ >= barrier_)
                        value_ = A(-1,-1) - B(-1,-1) + D(-1,-1) + E(-1,-1);
                    else
                        value_ = C(-1,-1) + E(-1,-1);
                    break;
                  case DownOut:
                    if(strike_ >= barrier_)
                        value_ = A(1,-1) - B(1,-1) + C(1,-1)- D(1,-1) + F(1,-1);
                    else
                        value_ = F(1,-1);
                    break;
                  case UpOut:
                    if(strike_ >= barrier_)
                        value_ = B(-1,-1) - D(-1,-1) + F(-1,-1);
                    else
                        value_ = A(-1,-1) - C(-1,-1) + F(-1,-1);
                    break;
                }
                break;
            }
            hasBeenCalculated_ = true;
          }
          return value_;
        }

        void BarrierOption::calculate() const{
        
            double underPlus = underlying_ * (1 + 0.0001);
            double underMinu = underlying_ * (1 - 0.0001);
            double timePlus = residualTime_ * (1 + 0.0001);
            BarrierOption barrierTimePlus(barrType_,
                type_, underlying_, strike_, dividendYield_,
                riskFreeRate_, timePlus, volatility_,
                barrier_, rebate_);

            BarrierOption barrierPlus(barrType_,
                type_, underPlus, strike_, dividendYield_,
                riskFreeRate_, residualTime_, volatility_,
                barrier_, rebate_);

            BarrierOption barrierMinus(barrType_,
                type_, underMinu, strike_, dividendYield_,
                riskFreeRate_, residualTime_, volatility_,
                barrier_, rebate_);

            delta_ = (barrierPlus.value()-barrierMinus.value())/
                     (underPlus-underMinu);

            gamma_ = (barrierPlus.value()
                      + barrierMinus.value() - 2.0 * value())/
                                         (underPlus-underMinu);
            theta_ = (barrierTimePlus.value() - value())/
                            (timePlus - residualTime_);
            greeksCalculated_ = true;                            
        }

        double BarrierOption::delta() const {
            if (!greeksCalculated_)  
                calculate();
            return delta_;
        }
        
        double BarrierOption::gamma() const {
            if(!greeksCalculated_) 
                calculate();
            return gamma_;
        }
        
        double BarrierOption::theta() const {
            if(!greeksCalculated_) 
                calculate();
            return theta_;
        }

    }

}
