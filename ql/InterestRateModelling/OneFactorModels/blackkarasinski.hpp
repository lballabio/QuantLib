
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

#ifndef quantlib_one_factor_models_black_karasinski_h
#define quantlib_one_factor_models_black_karasinski_h

#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/InterestRateModelling/OneFactorModels/tree.hpp"

namespace QuantLib {
   
    namespace InterestRateModelling { 

        class BlackKarasinski : public OneFactorModel {
          public:
            BlackKarasinski(const RelinkableHandle<TermStructure>& termStructure, unsigned int timeSteps = 1000);
            virtual ~BlackKarasinski() {}

            virtual void setParameters(const Array& params) {
                QL_REQUIRE(params.size()==2, 
                    "Incorrect number of parameters for BK calibration");
                alpha_ = params[0];
                sigma_ = params[1];
                calculateTree();
            }

            virtual double discountBond(Time now, Time maturity, Rate r) const;

            virtual double discountBondOption(Option::Type type, double strike, Time maturity, Time bondMaturity) const {
                throw("Not yet implemented");
                QL_DUMMY_RETURN(0.0);
            }

            virtual double stateVariable(Rate r) const {
                return QL_LOG(r);
            }
            virtual Rate getRateFrom(double y) const {
                return QL_EXP(y);
            }

          private:
            double theta(Time t) const {
                unsigned index = (unsigned)(t/dt_);
                return theta_[index];
            }

            void calculateTree();

            class Process;
            friend class Process;

            class PrivateFunction;
            friend class PrivateFunction;

            unsigned int timeSteps_;
            std::vector<double> theta_, u_;
            TrinomialTree tree_;
            double alpha_;
            double sigma_;
            double dt_;
        };
    }
}

#endif
