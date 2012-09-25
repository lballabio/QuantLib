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

#include <ql/math/optimization/differentialevolution.hpp>

namespace QuantLib {

    DifferentialEvolution::DifferentialEvolution(
		const Array& minParams, const Array& maxParams,
		DifferentialEvolution::Strategy strategy,
		Real F, Real CR, bool adaptive, Size nPop)
	: minParams_(minParams), maxParams_(maxParams),
	  nParam_(minParams_.size()),
      nPop_(nPop == Null<Size>() ? minParams.size()*10 : nPop),
	  strategy_(strategy), adaptive_(adaptive),F_(F), CR_(CR) {

		QL_REQUIRE(minParams_.size() == maxParams_.size(),
			"Need same number of minimum and maximum start parameters");
		for (Size i=0; i<nParam_; ++i) {
			QL_REQUIRE(minParams_[i] < maxParams_[i],
				"Minimum start parameter number " << i 
				<< " must be smaller than maximum start parameter");
		}
		QL_REQUIRE(F_ >= 0.0 && F_ <= 2.0,
			"F must be between 0.0 and 2.0");
		QL_REQUIRE(CR_ >= 0.0 && CR_ <= 1.0,
			"CR must be between 0.0 and 1.0");

		setStrategy();
	}


    EndCriteria::Type DifferentialEvolution::minimize(Problem& P,
											const EndCriteria& endCriteria) {

		EndCriteria::Type ecType = EndCriteria::MaxIterations;
	    QL_REQUIRE(P.currentValue().size() == nParam_,
			"Number of parameters mismatch between problem and DE optimizer");
        P.reset();		
		init();

		Real bestCost = QL_MAX_REAL;
		Size bestPop  = 0;
		for (Size p = 0; p < nPop_; ++p) {
			Array tmp(currGen_[p].pop_);
			try {
				currGen_[p].cost_ = P.costFunction().value(tmp);
			} catch (Error&) {
				currGen_[p].cost_ = QL_MAX_REAL;
			}
			if (currGen_[p].cost_ < bestCost) {
				bestPop = p;
				bestCost = currGen_[p].cost_;
			}
		}

		Size lastChange = 0;
		Size lastParamChange = 0;
		for(Size i=0; i<endCriteria.maxIterations(); ++i) {

			Size newBestPop = bestPop;
			Real newBestCost = bestCost;

			for (Size p=0; p<nPop_; ++p) {
				// Find 3 different populations randomly
				Size r1;
				do {
					r1 = static_cast <Size> (uniformRng_.nextInt32() % nPop_);
				}		
				while(r1 == p || r1 == bestPop);

				Size r2;
				do {
					r2 = static_cast <Size> (uniformRng_.nextInt32() % nPop_);
				}
				while ( r2 == p || r2 == bestPop || r2 == r1);

				Size r3;
				do {
					r3 = static_cast <Size> (uniformRng_.nextInt32() % nPop_);
				} while ( r3 == p || r3 == bestPop || r3 == r1 || r3 == r2);

				for(Size j=0; j<nParam_; ++j) {
					nextGen_[p].pop_[j] = currGen_[p].pop_[j];
				}

				Size j = static_cast <Size> (uniformRng_.nextInt32() % nParam_);
				Size L = 0;
				do {
					const double tmp = 
						currGen_[      p].pop_[j] * a0_
					  + currGen_[     r1].pop_[j] * a1_
					  + currGen_[     r2].pop_[j] * a2_
					  + currGen_[     r3].pop_[j] * a3_
					  + currGen_[bestPop].pop_[j] * aBest_;

					nextGen_[p].pop_[j] =
						std::min(maxParams_[j], std::max(minParams_[j], tmp));

					j = (j+1)%nParam_;
					++L;
				} while ((uniformRng_.nextReal() < CR_) && (L < nParam_));

				// Evaluate the new population
				Array tmp(nextGen_[p].pop_);
				try {
					nextGen_[p].cost_ = P.costFunction().value(tmp);
                } catch (Error&) {
					nextGen_[p].cost_ = QL_MAX_REAL;
                }

				// Not better, discard it and keep the old one.
				if (nextGen_[p].cost_ >= currGen_[p].cost_) {
					nextGen_[p] = currGen_[p];
				}
				// Better, keep it.
				else {
					// New best?
					if (nextGen_[p].cost_ < newBestCost) {
						newBestPop = p;
						newBestCost = nextGen_[p].cost_;
					}
				}
			}

			if(std::abs(newBestCost-bestCost) > endCriteria.functionEpsilon()) {
				lastChange = i;
			}
			const Array absDiff = Abs(nextGen_[newBestPop].pop_-currGen_[bestPop].pop_);
			if(*std::max_element(absDiff.begin(), absDiff.end()) > endCriteria.rootEpsilon()) {
				lastParamChange = i;
			}

			bestPop = newBestPop;
			bestCost = newBestCost;
			currGen_ = nextGen_;

            if(i-lastChange > endCriteria.maxStationaryStateIterations()) {
				ecType = EndCriteria::StationaryFunctionValue;
				break;
			}
			if(i-lastParamChange > endCriteria.maxStationaryStateIterations()) {
				ecType = EndCriteria::StationaryPoint;
				break;
			}

            if (adaptive_) adaptParameters();
		}
		
		const Array res(currGen_[bestPop].pop_);
        P.setCurrentValue(res);
        P.setFunctionValue(bestCost);
        
        return ecType;
    }

	
	void DifferentialEvolution::init () {
		// Initialize RNG
		uniformRng_ = MersenneTwisterUniformRng(4711);
		// Size all the arrays
		currGen_.resize(nPop_);
		nextGen_.resize(nPop_);
		for (Size i = 0; i < nPop_; ++i) {
			currGen_[i].pop_ = Array(nParam_);
			nextGen_[i].pop_ = Array(nParam_);
		}
		// Initialize populations with random values
		for (Size i=0; i<nParam_; ++i) {
			const double offset = minParams_[i];
			const double mul = maxParams_[i] - minParams_[i];

			for (Size j=0; j<nPop_; ++j) {
				const double val_rnd = uniformRng_.nextReal();
				currGen_[j].pop_[i] = val_rnd*mul + offset;
			}
		}
	}


	void DifferentialEvolution::setStrategy() {
        switch (strategy_) {
		  case DifferentialEvolution::Rand1Exp:
			a0_    =  0.0;
			a1_    =  1.0;
			a2_    =  F_;
			a3_    = -F_;
			aBest_ =  0.0;
            break;
          case DifferentialEvolution::RandToBest1Exp:
			a0_    =  1.0-F_;
			a1_    =  F_;
			a2_    = -F_;
			a3_    =  0.0;
			aBest_ =  F_;
            break;
          default:
            QL_FAIL("Unknown Differential Evolution strategy " << strategy_);
        }
	}


	void DifferentialEvolution::adaptParameters() {
		const Real F_l = 0.1;
		const Real F_u = 0.9;
		const Real tau_1 = 0.1;
		const Real tau_2 = 0.1;

		const Real r_1 = uniformRng_.nextReal();
		const Real r_2 = uniformRng_.nextReal();
		const Real r_3 = uniformRng_.nextReal();
		const Real r_4 = uniformRng_.nextReal();

		F_  = r_2 < tau_1 ? F_l + r_1*F_u : F_;
		CR_ = r_4 < tau_2 ? r_3 : CR_;

		setStrategy();
	}
}

