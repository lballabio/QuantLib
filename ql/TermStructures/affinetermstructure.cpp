
/*
 Copyright (C) 2000, 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file affinetermstructure.cpp
    \brief Affine term-structure
*/

#include <ql/TermStructures/affinetermstructure.hpp>

namespace QuantLib {

    namespace TermStructures {

        class AffineTermStructure::CalibrationFunction
            : public Optimization::CostFunction {
          public:
            CalibrationFunction(
              const Handle<ShortRateModels::Model>& model,
              const std::vector<Handle<RateHelper> >& instruments)
            : model_(model), instruments_(instruments) {}
            virtual ~CalibrationFunction() {}

            virtual double value(const Array& params) const {
                model_->setParams(params);

                double value = 0.0;
                for (Size i=0; i<instruments_.size(); i++) {
                    double diff = instruments_[i]->quoteError();
                    value += diff*diff;
                }
                return value;
            }
            virtual double finiteDifferenceEpsilon() const { return 1e-7; }
          private:
            Handle<ShortRateModels::Model> model_;
            const std::vector<Handle<RateHelper> >& instruments_;
        };

        AffineTermStructure::AffineTermStructure(
            const Date& todaysDate,
            const Date& referenceDate,
            const Handle<ShortRateModels::AffineModel>& model,
            const DayCounter& dayCounter)
        : dayCounter_(dayCounter), todaysDate_(todaysDate), 
          referenceDate_(referenceDate), needsRecalibration_(false), 
          model_(model) { }

        AffineTermStructure::AffineTermStructure(
            const Date& todaysDate,
            const Date& referenceDate,
            const Handle<ShortRateModels::AffineModel>& model,
            const std::vector<Handle<RateHelper> >& instruments,
            const Handle<Optimization::Method>& method,
            const DayCounter& dayCounter)
        : dayCounter_(dayCounter), todaysDate_(todaysDate), 
          referenceDate_(referenceDate), needsRecalibration_(true), 
          model_(model), instruments_(instruments), method_(method) {
            for (Size i=0; i<instruments_.size(); i++)
                registerWith(instruments_[i]);
        }

        void AffineTermStructure::calibrate() const {
            Handle<ShortRateModels::Model> model = model_;
            CalibrationFunction f(model, instruments_);

            method_->setInitialValue(model->params());
            method_->endCriteria().setPositiveOptimization();
            Optimization::Problem prob(f, *model->constraint(), *method_);
            prob.minimize();

            Array result(prob.minimumValue());
            model->setParams(result);
            needsRecalibration_ = false;
        }

    }

}
