
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

#include <ql/array.hpp>
#include <ql/minimizer.hpp>
#include <ql/option.hpp>
#include <ql/termstructure.hpp>

#include <iostream>
#include <list>
#include <vector>

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper;
/*
        class TermStructureConsistentModel {
          public:
            TermStructureConsistentModel(
                double dt,
                const RelinkableHandle<TermStructure>& termStructure)
            : dt_(dt), termStructure_(termStructure) {}
            virtual ~TermStructureConsistentModel() {}

            virtual void setCalibrationParameters() {}
            void calibrate(
                const Handle<Minimizer>& minimizer,
                std::vector<Handle<CalibrationHelper> > instruments,
                std::vector<double> volatilities) {}

            const RelinkableHandle<TermStructure>& termStructure() const {
                return termStructure_;

            virtual double theta(Time t) = 0;
        };
*/
        class Model {
          public:

            enum Type { OneFactor, TwoFactor, Market };

            Model(unsigned nParams, Type type,
                  const RelinkableHandle<TermStructure>& termStructure)
            : params_(nParams), termStructure_(termStructure) {}
            virtual ~Model() {}

            virtual double discountBondOption(Option::Type type, 
                                              double strike,
                                              Time maturity, 
                                              Time bondMaturity) {
                return Null<double>();
            }

            virtual void reconfigure(const std::list<Time>& times_) = 0;

            void calibrate(
                const Handle<Minimizer>& minimizer,
                std::vector<Handle<CalibrationHelper> >& instruments);

            const RelinkableHandle<TermStructure>& termStructure() const {
                return termStructure_;
            }

            Type type() { return type_; }

            const Array& params() { return params_; }
            void setParams(const Array& params) { params_ = params; }

            virtual std::string name() = 0;

          protected:
            Handle<Constraint> constraint_;
            Array params_;

          private:
            class CalibrationProblem;
            friend class CalibrationProblem;
            const RelinkableHandle<TermStructure>& termStructure_;
            Type type_;
        };

        class CalibrationHelper {
          public:
            virtual double marketValue() = 0;
            virtual double modelValue(const Handle<Model>& model) = 0;
        };

    }

}
#endif
