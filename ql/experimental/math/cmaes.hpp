/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Colin Alberts

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cmaes.hpp
    \brief Covariance Matrix Adaptation Evolution Strategy (CMA-ES) optimizer
*/

#ifndef quantlib_cmaes_hpp
#define quantlib_cmaes_hpp

#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    //! Covariance Matrix Adaptation Evolution Strategy (CMA-ES)
    /*! Derivative-free global optimizer. Standard (mu/mu_w, lambda)-CMA-ES with
        positive recombination weights.

        Reference: N. Hansen, "The CMA Evolution Strategy: A Tutorial", 2016,
        arXiv:1604.00772.

        \test convergence is checked on sphere, Rosenbrock (2D/10D), and a
              multimodal function, plus seed-reproducibility and box bounds.

        \ingroup optimizers
    */
    class Cmaes : public OptimizationMethod {
      public:
        class Configuration {
          public:
            Size populationSize = 0;   //!< lambda; 0 => default 4 + floor(3 ln n)
            Real sigma = 0.3;          //!< initial step size, in x-space units
            unsigned long seed = 0;
            Array lowerBound, upperBound;
            Array initialMean;

            Configuration() {}
            Configuration& withPopulationSize(Size n) { 
                populationSize = n; 

                return *this; 
            }
            Configuration& withSigma(Real s) {
                QL_REQUIRE(s > 0.0, "sigma must be positive");
                sigma = s; 

                return *this;
            }
            Configuration& withSeed(unsigned long s) { 
                seed = s; 

                return *this; 
            }
            Configuration& withBounds(const Array& lo, const Array& up) {
                lowerBound = lo; 
                upperBound = up; 

                return *this;
            }
            Configuration& withInitialMean(const Array& m) { 
                initialMean = m; 
                
                return *this; 
            }
        };

        explicit Cmaes(Configuration configuration = Configuration());

        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override;

        const Configuration& configuration() const { return configuration_; }

      private:
        Configuration configuration_;
        MersenneTwisterUniformRng rng_;
    };

}

#endif
