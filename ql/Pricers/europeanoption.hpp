
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

/*! \file europeanoption.hpp

    \fullpath
    Include/ql/Pricers/%europeanoption.hpp
    \brief european option

*/

// $Log$
// Revision 1.1  2001/09/03 14:04:01  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.7  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.6  2001/08/21 16:42:12  nando
// european option optimization
//
// Revision 1.5  2001/08/13 15:06:17  nando
// added dividendRho method
//
// Revision 1.4  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.3  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.2  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.1  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/07/06 18:24:17  nando
// slight modifications to avoid VisualStudio warnings
//
// Revision 1.6  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef BSM_european_option_pricer_h
#define BSM_european_option_pricer_h

#include "ql/Pricers/singleassetoption.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace Pricers {

        //! Black-Scholes-Merton European option
        class EuropeanOption : public SingleAssetOption {
          public:
            // constructor
            EuropeanOption(Type type, double underlying, double strike,
                      Rate dividendYield, Rate riskFreeRate,
                      Time residualTime, double volatility);
            // accessors
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            double vega() const;
            double rho() const;
            double dividendRho() const;
            Handle<SingleAssetOption> clone() const {
                return Handle<SingleAssetOption>(new EuropeanOption(*this)); }
            // modifiers
            void setVolatility(double newVolatility);
            void setRiskFreeRate(Rate newRate);
            void setDividendYield(Rate newDividendYield);
          private:
            static const Math::CumulativeNormalDistribution f_;
            double alpha() const;
            double beta() const;
            double standardDeviation() const;
            double D1() const;
            double D2() const;
            double NID1() const;
            DiscountFactor dividendDiscount() const;
            DiscountFactor riskFreeDiscount() const;
            // declared as mutable to preserve
            // the logical constness (does this word exist?) of value()
            mutable double alpha_, beta_, standardDeviation_, D1_, D2_, NID1_;
            mutable DiscountFactor dividendDiscount_, riskFreeDiscount_;
        };

        inline double EuropeanOption::alpha() const {
            if (alpha_==Null<double>()) {
                switch (type_) {
                    case Call:
                        alpha_ = f_(D1());
                        break;
                    case Put:
                        alpha_ = f_(D1())-1.0;
                        break;
                    case Straddle:
                        alpha_ = 2.0*f_(D1())-1.0;
                        break;
                    default:
                        throw IllegalArgumentError(
                            "EuropeanOption: invalid option type");
                }
            }
            return alpha_;
        }

        inline double EuropeanOption::beta() const {
            if (beta_==Null<double>()) {
                switch (type_) {
                    case Call:
                        beta_ = f_(D2());
                        break;
                    case Put:
                        beta_ = f_(D2())-1.0;
                        break;
                    case Straddle:
                        beta_ = 2.0*f_(D2())-1.0;
                        break;
                    default:
                        throw IllegalArgumentError(
                            "EuropeanOption: invalid option type");
                }
            }
            return beta_;
        }

        inline double EuropeanOption::standardDeviation() const {
            if (standardDeviation_==Null<double>())
                standardDeviation_ = volatility_*QL_SQRT(residualTime_);
            return standardDeviation_;
        }

        inline double EuropeanOption::D1() const {
            if (D1_==Null<double>())
                D1_ = QL_LOG(underlying_/strike_)/standardDeviation() +
                    standardDeviation()/2.0 + (riskFreeRate_ - dividendYield_)*
                    residualTime_/standardDeviation();
            return D1_;
        }

        inline double EuropeanOption::D2() const {
            if (D2_==Null<double>())
                D2_ = D1() - standardDeviation();
            return D2_;
        }

        inline double EuropeanOption::NID1() const {
            if (NID1_==Null<double>()) {
                switch (type_) {
                    case Call:
                    case Put:
                        NID1_ = f_.derivative(D1());
                        break;
                    case Straddle:
                        NID1_ = 2.0*f_.derivative(D1());
                        break;
                    default:
                        throw IllegalArgumentError(
                            "EuropeanOption: invalid option type");
                }
            }
            return NID1_;
        }

        inline DiscountFactor EuropeanOption::dividendDiscount() const {
            if (dividendDiscount_==Null<DiscountFactor>())
                dividendDiscount_ = (QL_EXP(-dividendYield_*residualTime_));
            return dividendDiscount_;
        }

        inline DiscountFactor EuropeanOption::riskFreeDiscount() const {
            if (riskFreeDiscount_==Null<DiscountFactor>())
                riskFreeDiscount_ = (QL_EXP(-riskFreeRate_*residualTime_));
            return riskFreeDiscount_;
        }


    }

}


#endif
