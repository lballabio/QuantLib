/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Ralph Schreyer

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

/*! \file differentialevolution.hpp
    \brief Differential Evolution optimization method
*/

#ifndef quantlib_optimization_differential_evolution_hpp
#define quantlib_optimization_differential_evolution_hpp

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    //! Differential evolution optimizer
    /*! The algorithm and strategy names are taken from here:

        Price, K., Storn, R., 1997. Differential Evolution – 
        A Simple and Efficient Heuristic for Global Optimization 
        over Continuous Spaces.
        Journal of Global Optimization, Kluwer Academic Publishers, 
        1997, Vol. 11, pp. 341 - 359.

        The binomial crossover is not yet implemented, neither the
        strategies with a fourth random population. One should
        factor out the mutation functions before doing so ...


        The adaptive parameter algorithm is described here:

        Brest, J. et al., 2006. Self-Adapting Control Parameters 
        in Differential Evolution: A Comparative Study on Numerical 
        Benchmark Problems.
        IEEE Transactions on Evolutionary Computation, Vol. 10, 
        No. 6, December 2006.

        \test optimization of known test functions.
    */
    class DifferentialEvolution : public OptimizationMethod {
      public:
		enum Strategy {Rand1Exp, RandToBest1Exp};

        DifferentialEvolution(
			const Array& minParams, const Array& maxParams,
			DifferentialEvolution::Strategy strategy = 
						DifferentialEvolution::RandToBest1Exp,
            Real F = 0.85, Real CR = 1.0, bool adaptive = false, 
            Size nPop = Null<Size>());

        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria);
      private:
		void init();
		void setStrategy();
		void adaptParameters();

		const Array minParams_;
		const Array maxParams_;
		const Size nParam_;
		const Size nPop_;
		const DifferentialEvolution::Strategy strategy_;
        const bool adaptive_;
		Real F_, CR_, a0_, a1_, a2_, a3_, aBest_;
		MersenneTwisterUniformRng uniformRng_;

	  private:
		struct Population {
			Array pop_;
			Real  cost_;
		};

		typedef	std::vector<Population> Generation;

		Generation currGen_;
		Generation nextGen_;
	};
}


#endif
