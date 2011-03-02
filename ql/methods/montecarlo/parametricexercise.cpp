/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/methods/montecarlo/parametricexercise.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace QuantLib {

    namespace {

        class ValueEstimate : public CostFunction {
          public:
            ValueEstimate(const std::vector<NodeData>& simulationData,
                          const ParametricExercise& exercise,
                          Size exerciseIndex);
            Real value(const Array& parameters) const;
            Disposable<Array> values(const Array&) const {
                QL_FAIL("values method not implemented");
            }
        private:
            const std::vector<NodeData>& simulationData_;
            const ParametricExercise& exercise_;
            Size exerciseIndex_;
            mutable std::vector<Real> parameters_;
        };

        ValueEstimate::ValueEstimate(
                                 const std::vector<NodeData>& simulationData,
                                 const ParametricExercise& exercise,
                                 Size exerciseIndex)
        : simulationData_(simulationData), exercise_(exercise),
          exerciseIndex_(exerciseIndex),
          parameters_(exercise.numberOfParameters()[exerciseIndex]) {
            for (Size i=0; i<simulationData_.size(); ++i) {
                if (simulationData_[i].isValid)
                    return;
            }
            QL_FAIL("no valid paths");
        }

        Real ValueEstimate::value(const Array& parameters) const {
            std::copy(parameters.begin(), parameters.end(),
                      parameters_.begin());
            Real sum = 0.0;
            Size n = 0;
            for (Size i=0; i<simulationData_.size(); ++i) {
                if (simulationData_[i].isValid) {
                    ++n;
                    if (exercise_.exercise(exerciseIndex_,
                                           parameters_,
                                           simulationData_[i].values))
                        sum += simulationData_[i].exerciseValue;
                    else
                        sum += simulationData_[i].cumulatedCashFlows;
                }
            }
            return -sum/n;
        }

    }



    Real genericEarlyExerciseOptimization(
                          std::vector<std::vector<NodeData> >& simulationData,
                          const ParametricExercise& exercise,
                          std::vector<std::vector<Real> >& parameters,
                          const EndCriteria& endCriteria,
                          OptimizationMethod& method) {

        Size steps = simulationData.size();
        parameters.resize(steps-1);

        for (Size i=steps-1; i!=0; --i) {
            const std::vector<NodeData>& exerciseData = simulationData[i];

            parameters[i-1].resize(exercise.numberOfParameters()[i-1]);


            // optimize
            ValueEstimate f(exerciseData, exercise, i-1);

            Array guess(parameters[i-1].size());
            exercise.guess(i-1, parameters[i-1]);
            std::copy(parameters[i-1].begin(), parameters[i-1].end(),
                      guess.begin());

            NoConstraint c;

            Problem p(f, c, guess);
            method.minimize(p, endCriteria);

            Array result = p.currentValue();
            std::copy(result.begin(), result.end(),
                      parameters[i-1].begin());

            std::vector<NodeData>& previousData = simulationData[i-1];
            for (Size j=0; j<previousData.size(); ++j) {
                if (exerciseData[j].isValid) {
                    if (exercise.exercise(i-1,
                                          parameters[i-1],
                                          exerciseData[j].values))
                        previousData[j].cumulatedCashFlows +=
                            exerciseData[j].exerciseValue;
                    else
                        previousData[j].cumulatedCashFlows +=
                            exerciseData[j].cumulatedCashFlows;
                }
            }
        }

        Real sum = 0.0;
        const std::vector<NodeData>& initialData = simulationData.front();
        for (Size i=0; i<initialData.size(); ++i)
            sum += initialData[i].cumulatedCashFlows;
        return sum/initialData.size();
    }

}

