
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

/*! \file model.hpp
    \brief Abstract interest rate model class

    \fullpath
    ql/InterestRateModelling/%model.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_model_h
#define quantlib_interest_rate_modelling_model_h

#include <vector>
#include "ql/array.hpp"
#include "ql/minimizer.hpp"
#include "ql/option.hpp"
#include "ql/termstructure.hpp"
#include "ql/types.hpp"

#include <iostream>
using std::cout;
using std::endl;

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper;

        class Model {
          public:
            Model(unsigned nbParams, 
                const RelinkableHandle<TermStructure>& termStructure) 
            : nbParams_(nbParams), termStructure_(termStructure) {}
            virtual ~Model() {}
            virtual void setParameters(const Array& params) = 0;

            virtual double discountBond(Time now, Time maturity, Rate r) const { 
                return Null<double>();
            }

            virtual double discountBondOption(Option::Type type, double strike,
                Time maturity, Time bondMaturity) const {
                return Null<double>();
            }

            void calibrate(const Handle<Minimizer>& minimizer,
                std::vector<Handle<CalibrationHelper> > instruments, 
                std::vector<double> volatilities);

            const RelinkableHandle<TermStructure>& termStructure() const { 
                return termStructure_; 
            }

          protected:
            Handle<Constraint> constraint_;

          private:
            class CalibrationProblem;
            friend class CalibrationProblem;
            unsigned int nbParams_;
            const RelinkableHandle<TermStructure>& termStructure_;
        };

        class CalibrationHelper {
          public:
            virtual double value(const Handle<Model>& model) = 0;
            virtual double blackPrice(double volatility) const = 0;
        };

    }

}
#endif
