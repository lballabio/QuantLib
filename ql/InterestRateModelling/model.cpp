
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

/*! \file model.cpp
    \brief Abstract interest rate model class

    \fullpath
    ql/InterestRateModelling/%model.cpp
*/

// $Id$

#include "ql/InterestRateModelling/model.hpp"

#include "ql/Math/matrix.hpp"
#include "ql/InterestRateModelling/calibrationhelper.hpp"
#include "ql/Optimization/leastsquare.hpp"

#include <iostream>

namespace QuantLib {

    namespace InterestRateModelling {

        using Math::Matrix;
        using Optimization::LeastSquareProblem;
        using Optimization::NonLinearLeastSquare;

        class Model::CalibrationProblem : public LeastSquareProblem {
          public:
            CalibrationProblem(Model* model, 
                std::vector<Handle<CalibrationHelper> >& instruments)
            : model_(model, false), instruments_(instruments), 
              prices_(instruments.size()) {
                for (unsigned i=0; i<instruments_.size(); i++)
                    prices_[i] = instruments_[i]->marketValue();
            }
            virtual ~CalibrationProblem() {}

            //! Size of the least square problem
            virtual int size() { return instruments_.size(); }

            //! return function and target values
            virtual void targetAndValue(const Array& params, Array& target, 
                Array& fct2fit) {
                target = prices_;
                model_->setParams(params);
                for (signed i=0; i<size(); i++) {
                    fct2fit[i] = instruments_[i]->modelValue(model_);
                }
            }

            //! return function, target and first derivatives values
            virtual void targetValueAndGradient(const Array& params, 
                Matrix& grad_fct2fit, Array& target, Array& fct2fit) { 
                target = prices_;
                model_->setParams(params);
                Array newParams(params);
                int i;
                for (i=0; i<size(); i++)
                    fct2fit[i] = instruments_[i]->modelValue(model_);

                for (size_t j=0; j<params.size(); j++) {
                    double off = 1e-6;
                    newParams[j] -= off;
                    model_->setParams(newParams);
                    Array newValues(size());
                    for (i=0; i<size(); i++) 
                        newValues[i] = instruments_[i]->modelValue(model_);
                    newParams[j] += 2.0*off;
                    model_->setParams(newParams);
                    for (i=0; i<size(); i++) {
                        double diffValue = instruments_[i]->modelValue(model_);
                        diffValue -= newValues[i];
                        grad_fct2fit[i][j] = diffValue/(2.0*off);
                    }
                    newParams[j] = params[j];
                }
            }

          private:
            Handle<Model> model_;
            std::vector<Handle<CalibrationHelper> >& instruments_;
            Array prices_;
        };

        void Model::calibrate( 
            const Handle<Minimizer>&,
            std::vector<Handle<CalibrationHelper> >& instruments) {

            // Accuracy of the optimization method
            double accuracy = 1e-5;
            // Maximum number of iterations
            int maxiter = 10000;

            // Least square optimizer
            NonLinearLeastSquare lsqnonlin(accuracy,maxiter);

            // Define the least square problem
            CalibrationProblem problem(this, instruments);
         
            // Set initial values
            lsqnonlin.setInitialValue(Array(params_.size(), 0.1));

            // perform fitting
            Array solution = lsqnonlin.perform(problem);

            setParams(solution);

            std::cout << "Model calibrated to the following values:" << std::endl;
            for (unsigned i=0; i<params_.size(); i++)
                std::cout << i << "    " << solution[i]*100.0 << "%" << std::endl;
        }

    }

}
