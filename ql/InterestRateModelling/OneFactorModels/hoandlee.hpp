
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

/*! \file hoandlee.hpp
    \brief Ho-Lee model

    \fullpath
    ql/%hoandlee.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_ho_and_lee_h
#define quantlib_one_factor_models_ho_and_lee_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class HoAndLee : public OneFactorModel {
          public:
            HoAndLee(const RelinkableHandle<TermStructure>& termStructure);
            virtual ~HoAndLee() {}

            virtual void setParameters(const Array& params) {
                QL_REQUIRE(params.size()==1, 
                    "Incorrect number of parameters for HL calibration");
                sigma_ = params[0];
            }

            double theta(Time t) const {
                //Implying piecewise constant forward term structure
                double forwardDerivative = 0.0;
                return forwardDerivative + sigma_*sigma_*t;
            }
            virtual double stateVariable(Rate r) const {
                return r;
            }
            virtual Rate getRateFrom(double y) const {
                return y;
            }
            virtual double discountBond(Time T, Time s, Rate r) const {
                double value = QL_EXP(lnA(T,s) - B(T,s)*r);
                return value;
            }
            virtual double discountBondOption(Option::Type type, double strike, Time maturity, Time bondMaturity) const;

            double B(Time T, Time s) const {
                return (s - T);
            }
            double lnA(Time T, Time s) const;

          private:

            class Process;
            friend class Process;

            double sigma_;
            double dt_;

        };
        
    }

}

#endif
