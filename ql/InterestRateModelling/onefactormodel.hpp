
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
    ql/InterestRateModelling/%onefactormodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_one_factor_model_h
#define quantlib_interest_rate_modelling_one_factor_model_h

#include "ql/stochasticprocess.hpp"
#include "ql/InterestRateModelling/model.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper;

        class OneFactorModel : public Model {
          public:
            OneFactorModel(unsigned nParams, 
                const RelinkableHandle<TermStructure>& termStructure) 
            : Model(nParams, termStructure) {}
            virtual ~OneFactorModel() {}
            virtual double stateVariable(Rate r) const = 0;
            virtual Rate getRateFrom(double y) const = 0;
            virtual double minStateVariable() const { return -1000000;}
            virtual double maxStateVariable() const { return +1000000;}

            void fitToTermStructure(std::vector<double>& theta);
            const Handle<StochasticProcess>& process() const {
                return process_;
            }
          protected:
            Handle<StochasticProcess> process_;
          private:
            class FitFunction;
            friend class FitFunction;
        };

    }

}
#endif
