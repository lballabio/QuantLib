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
    ql/ShortRateModels/%model.cpp
*/

// $Id$

#include "ql/ShortRateModels/calibrationhelper.hpp"

namespace QuantLib {

    namespace ShortRateModels {

        Model::Model(Size nParameters) 
        : parameters_(nParameters),
          constraint_(new PrivateConstraint(parameters_)) {}

        class Model::CalibrationFunction : public Optimization::CostFunction {
          public:
            CalibrationFunction( 
              Model* model,
              CalibrationSet& instruments) 
            : model_(model, false),
              instruments_(instruments), prices_(instruments.size()) {
                for (Size i=0; i<prices_.size(); i++)
                    prices_[i] = instruments_[i]->marketValue();
            }
            virtual ~CalibrationFunction() {}
            
            virtual double value(
                const Array& params) {
                model_->setParams(params);
//                std::cout << "parameters set to " << params << std::endl;

                double value = 0.0;
                for (Size i=0; i<prices_.size(); i++) {
                    double diff = instruments_[i]->calibrationError();
                    value += diff*diff;
                }

//                std::cout << "Cost function: " << QL_SQRT(value) << std::endl;
                return QL_SQRT(value);
            }
            virtual double finiteDifferenceEpsilon() { return 1e-6; }
          private:
            Handle<Model> model_;
            CalibrationSet& instruments_;
            Array prices_;
        };

        void Model::calibrate(
            CalibrationSet& instruments,
            const Handle<Optimization::Method>& method) {

            CalibrationFunction f(this, instruments);

            method->setInitialValue(params());
            method->endCriteria().setPositiveOptimization();
            Optimization::Problem prob(f, *constraint_, *method);
            prob.minimize();

            Array result(prob.minimumValue());
            setParams(result);

//            std::cout << "Cost function: " << f.value(result) << std::endl;
//            std::cout << "Model calibrated to:" << result << std::endl;
        }

        Array Model::params() {
            Size size = 0, i;
            for (i=0; i<parameters_.size(); i++)
                size += parameters_[i].size();
            Array params(size);
            Size k = 0;
            for (i=0; i<parameters_.size(); i++) {
                for (Size j=0; j<parameters_[i].size(); j++, k++) {
                    params[k] = parameters_[i].params()[j];
                }
            }
            return params; 
        }

        void Model::setParams(const Array& params) {
            Array::const_iterator p = params.begin();
            for (Size i=0; i<parameters_.size(); i++) {
                for (Size j=0; j<parameters_[i].size(); j++, p++) {
                    QL_REQUIRE(p!=params.end(),"Parameter array too small");
                    parameters_[i].setParam(j, *p);
                }
            }
            QL_REQUIRE(p==params.end(),"Parameter array too big!");
            update();
        }

    }

}
