
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

/*! \file blackkarasinski.hpp
    \brief Black-Karasinski model

    \fullpath
    ql/%blackkarasinski.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_hull_and_white_h
#define quantlib_one_factor_models_hull_and_white_h

#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/InterestRateModelling/OneFactorModels/hoandlee.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class HullAndWhite : public OneFactorModel {
          public:
            HullAndWhite(const RelinkableHandle<TermStructure>& termStructure);
            virtual ~HullAndWhite() {}
            
            virtual void setParameters(const Array& params) {
                QL_REQUIRE(params.size()==2, 
                    "Incorrect number of parameters for HW calibration");
                alpha_ = params[0];
                sigma_ = params[1];
            }
            virtual double theta(Time t) const;
            
            virtual double stateVariable(Rate r) const {
                return r;
            }
            virtual Rate getRateFrom(double y) const {
                return y;
            }
            virtual double discountBond(Time T, Time s, Rate r) const ;

            virtual double discountBondOption(Option::Type type, double strike, Time maturity, Time bondMaturity) const;

          private:
            inline double B(Time t) const {
                if (alpha_ == 0.0)
                    return t;
                else
                    return (1.0 - QL_EXP(-alpha_*t))/alpha_;
            }
            double lnA(Time T, Time s) const;
            class Process;
            friend class Process;
                
            double alpha_, sigma_;
            std::vector<double> theta_;
            double dt_;
        };

    }

}

#endif
