
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

/*! \file onefactormodel.hpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/%onefactormodel.hpp
*/

// $Id$

#ifndef quantlib_one_factor_models_black_derman_and_toy_h
#define quantlib_one_factor_models_black_derman_and_toy_h

#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {
   
    namespace InterestRateModelling { 

        using std::vector;

        class BlackDermanAndToy : public OneFactorModel {
          public:
            BlackDermanAndToy(const RelinkableHandle<TermStructure>& termStructure, unsigned int timeSteps = 1000);
            virtual ~BlackDermanAndToy() {}

            virtual void setParameters(const Array& params) {
                QL_REQUIRE(params.size()==1,
                    "Incorrect number of parameters for BDT calibration");
                sigma_ = params[0];
                calculateTree();
            }

            virtual double discountBond(Time now, Time maturity, Rate r) const;

            virtual double discountBondOption(Option::Type type, double strike,
                Time maturity, Time bondMaturity) const;

            virtual double stateVariable(Rate r) const {
                return QL_LOG(r);
            }
            virtual Rate getRateFrom(double y) const {
                return QL_EXP(y);
            }

          private:
            void calculateTree();

            double theta(Time t) const;
            double sigma(Time t) const { return sigma_; }
            double sigmaPrime(Time t) const { return 0.0; }

            class Process;
            friend class Process;

            class PrivateFunction;
            friend class PrivateFunction;

            vector<double> theta_, u_;
            vector<vector<double> > statePrices_, discountFactors_;
            double sigma_;
            double dt_;
            unsigned int timeSteps_;
        };
    }
}

#endif
