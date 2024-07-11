/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file singlefactorbsmnasketengine.cpp
*/

#include <ql/exercise.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newton.hpp>
#include <ql/math/solvers1d/ridder.hpp>
#include <ql/pricingengines/basket/vectorbsmprocessextractor.hpp>
#include <ql/pricingengines/basket/singlefactorbsmbasketengine.hpp>

#include <iostream>

namespace QuantLib {

    SumExponentialsRootSolver::SumExponentialsRootSolver(
    	Array a, Array sig, Real K)
      : a_(std::move(a)), sig_(std::move(sig)), K_(K),
		fCtr_(0), fPrimeCtr_(0), fDoublePrimeCtr_(0) {
    	QL_REQUIRE(a_.size() == sig_.size(),
    			"Arrays must have the same size");
    }

    Real SumExponentialsRootSolver::operator()(Real x) const {
    	++fCtr_;

        Real s = 0.0;
        for (Size i=0; i < a_.size(); ++i)
            s += a_[i]*std::exp(sig_[i]*x);
        return s - K_;
    }

    Real SumExponentialsRootSolver::derivative(Real x) const {
    	++fPrimeCtr_;

        Real s = 0.0;
        for (Size i=0; i < a_.size(); ++i)
            s += a_[i]*sig_[i]*std::exp(sig_[i]*x);
        return s;
    }

    Real SumExponentialsRootSolver::secondDerivative(Real x) const {
    	++fDoublePrimeCtr_;

        Real s = 0.0;
        for (Size i=0; i < a_.size(); ++i)
            s += a_[i]*squared(sig_[i])*std::exp(sig_[i]*x);
        return s;
    }

    Size SumExponentialsRootSolver::getFCtr() const {
    	return fCtr_;
    }

    Size SumExponentialsRootSolver::getDerivativeCtr() const {
    	return fPrimeCtr_;
    }

    Size SumExponentialsRootSolver::getSecondDerivativeCtr() const {
    	return fDoublePrimeCtr_;
    }

    Real SumExponentialsRootSolver::rootSumExponentials(
    	Real xTol, Strategy strategy) const {

        const Array attr = a_*sig_;
        QL_REQUIRE(
            std::all_of(
                attr.begin(), attr.end(),
                [](Real x) -> bool { return x >= 0.0; }
            ),
            "a*sig should not be negative"
        );

        const bool logProb =
            std::all_of(
                a_.begin(), a_.end(),
                [](Real x) -> bool { return x > 0;}
        );

        QL_REQUIRE(K_ > 0 || !logProb,
            "non-positive strikes only allowed for spread options");

        // linear approximation
        const Real denom = std::accumulate(attr.begin(), attr.end(), 0.0);
        const Real xInit = (std::abs(denom) > 1000*QL_EPSILON)
            ? std::max(5.0, std::min(-5.0,
                    (K_ - std::accumulate(a_.begin(), a_.end(), 0.0))/denom)
              )
            : 0.0;

        switch(strategy) {
          case Brent:
            return QuantLib::Brent().solve(*this, xTol, xInit, 0.1);
          case Newton:
        	return QuantLib::Newton().solve(*this, xTol, xInit, 0.1);
          case Ridder:
        	return QuantLib::Ridder().solve(*this, xTol, xInit, 0.1);
          case Halley:
          case SuperHalley: {
        	const Size maxIter = 100;
        	Size nIter = 0;
            bool resultCloseEnough;
            Real x = xInit;
            Real xOld, fx;

            do {
                xOld = x;
                fx = (*this)(x);
                const Real fPrime = derivative(x);
                const Real lf = fx*secondDerivative(x)/(fPrime*fPrime);
                const Real step = (strategy == Halley)
                    ? Real(1/(1 - 0.5*lf)*fx/fPrime)
                    : Real((1 + 0.5*lf/(1-lf))*fx/fPrime);

                x -= step;
                resultCloseEnough = std::fabs(x-xOld) < 0.5*xTol;
            }
            while (!resultCloseEnough && ++nIter < maxIter);

            if (!resultCloseEnough && !close(std::fabs(fx), 0.0)) {
            	std::cout << "ouch" << std::endl;
                x = QuantLib::Brent().solve(*this, xTol, xInit, 0.1);

            }
            return x;
          }
          default:
        	QL_FAIL("unknown strategy type");
        }
    }


    SingleFactorBsmBasketEngine::SingleFactorBsmBasketEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > p)
      : n_(p.size()), processes_(std::move(p)) {
        for (const auto& process: processes_)
            registerWith(process);
    }

    void SingleFactorBsmBasketEngine::calculate() const {
        const ext::shared_ptr<AverageBasketPayoff> avgPayoff =
            ext::dynamic_pointer_cast<AverageBasketPayoff>(arguments_.payoff);
        QL_REQUIRE(avgPayoff, "average basket payoff expected");

        // sort assets by their weight
        const Array weights = avgPayoff->weights();
        QL_REQUIRE(n_ == weights.size(),
             "wrong number of weights arguments in payoff");

        const ext::shared_ptr<EuropeanExercise> exercise =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise);
        QL_REQUIRE(exercise, "not an European exercise");
        const Date maturityDate = exercise->lastDate();

        const detail::VectorBsmProcessExtractor pExtractor(processes_);
        const Array s = pExtractor.getSpot();
        const Array dq = pExtractor.getDividendYield(maturityDate);
        const Array v = pExtractor.getBlackVariance(maturityDate);
        const DiscountFactor dr0 = pExtractor.getInterestRate(maturityDate);



    }
}

