
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file barrieroption.cpp
    \brief barrier option

    $Source$
    $Log$
    Revision 1.11  2001/05/24 12:52:02  nando
    smoothing #include xx.hpp

    Revision 1.10  2001/04/17 09:15:37  marmar
    Another bug fixed

    Revision 1.9  2001/04/11 11:52:30  marmar
    Bug fixed in gamma(), (thanks to JH)

    Revision 1.8  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.7  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.6  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.5  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

*/

#include "ql/Pricers/barrieroption.hpp"

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
                       ((underPlus-underlying_)*(underlying_-underMinu));
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
