/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file model.cpp
    \brief Abstract interest rate model class

    \fullpath
    ql/InterestRateModelling/%model.cpp
*/

// $Id$


#include "ql/InterestRateModelling/calibrationhelper.hpp"
#include "ql/Optimization/optimizer.hpp"

#include <iostream>

namespace QuantLib {

    namespace InterestRateModelling {

        using namespace Optimization;

        class Model::CalibrationFunction : public CostFunction {
          public:
            CalibrationFunction( 
              Model* model,
              const std::vector<Handle<CalibrationHelper> >& instruments) 
            : model_(model, false),
              instruments_(instruments), prices_(instruments.size()) {
                for (Size i=0; i<prices_.size(); i++)
                    prices_[i] = instruments_[i]->marketValue();
            }
            virtual ~CalibrationFunction() {}
            
            virtual double Model::CalibrationFunction::value(
                const Array& params) {
                model_->setParams(params);

                double value = 0.0;
                for (Size i=0; i<prices_.size(); i++) {
                    double diff = (instruments_[i]->modelValue(model_) - 
                        prices_[i])/prices_[i];
                    value += diff*diff;
                }
                return QL_SQRT(value);
            }
            virtual double finiteDifferenceEpsilon() { return 1e-6; }
          private:
            Handle<Model> model_;
            const std::vector<Handle<CalibrationHelper> >& instruments_;
            Array prices_;
        };

        void Model::calibrate(
            const std::vector<Handle<CalibrationHelper> >& instruments,
            const Handle<OptimizationMethod>& method) {

            CalibrationFunction f(this, instruments);

            method->endCriteria().setPositiveOptimization();
            OptimizationProblem prob(f, *constraint_, *method);
            prob.minimize();

            Array result(prob.minimumValue());
            setParams(result);

            std::cout << "Cost function value: " << f.value(result) << std::endl;
            std::cout << "Model calibrated to these parameters:" << std::endl;
            for (Size i=0; i<params_.size(); i++)
                std::cout << i << "   " << params_[i]*100.0 << "%" << std::endl;
        }

    }

}
