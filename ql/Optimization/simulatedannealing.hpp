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
/*! \file simulatedannealing.hpp
    \brief SimulatedAnnealing optimization method

    \fullpath
    ql/Optimization/%simulatedannealing.hpp
*/

#ifndef quantlib_optimization_simulated_annealing_h
#define quantlib_optimization_simulated_annealing_h

#include <ql/Optimization/optimizer.hpp>
#include <ql/RandomNumbers/knuthuniformrng.hpp>

namespace QuantLib {

    namespace Optimization {

        //! Multi-dimensionnal SimulatedAnnealing class
        class SimulatedAnnealing : public OptimizationMethod {
          public:
            SimulatedAnnealing(double temperature, double lambda, double tol) 
            : temperature_(temperature), lambda_(lambda), tol_(tol) {}
            virtual ~SimulatedAnnealing() {}

            //! minimize the optimization problem P
            virtual void minimize(OptimizationProblem& P);

          private:
            inline double fluctuation() {
                return temperature_*rng_.next().value;
            }
            double SimulatedAnnealing::extrapolate(
                OptimizationProblem& P,
                Size iHighest, 
                double& highestValue,
                double factor);

            double temperature_;
            double lambda_;
            double tol_;

            std::vector<Array> vertices_;
            Array values_;
            Array sum_;

            Array best_;
            double bestValue_;

            RandomNumbers::KnuthUniformRng rng_;
        };

    }

}


#endif
