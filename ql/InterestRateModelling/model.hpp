
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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

#include <vector>

namespace QuantLib {

    namespace InterestRateModelling {

        class CalibrationHelper;

        class Model {
          public:
            enum Type { OneFactor, TwoFactor, Market };

            Model(Size nParams, Type type,
                  const RelinkableHandle<TermStructure>& termStructure)
            : params_(nParams), termStructure_(termStructure) {}
            virtual ~Model() {}

            virtual bool hasDiscountBondFormula() {
                return false;
            }
            virtual bool hasDiscountBondOptionFormula() {
                return false;
            }
            virtual double discountBondOption(Option::Type type,
                                              double strike,
                                              Time maturity,
                                              Time bondMaturity) {
                return Null<double>();
            }

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

    }

}
#endif
