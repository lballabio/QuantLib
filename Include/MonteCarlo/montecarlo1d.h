/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file montecarlo1d.h
	\brief Create a sample generator from a path generator and a single-path pricer
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2001/01/04 17:31:22  marmar
	Alpha version of the Monte Carlo tools.

*/


#ifndef quantlib_montecarlo_one_dimensional_h
#define quantlib_montecarlo_one_dimensional_h

#include "qldefines.h"
#include "rate.h"
#include "statistics.h"
#include "standardpathgenerator.h"
#include "mcoptionsample.h"
#include "handle.h"

#include "singlepathpricer.h"

namespace QuantLib {

	namespace MonteCarlo {
		//! General purpose 1D Monte Carlo pricer
		using Math::Statistics;

		class MonteCarlo1D {
		public:
			MonteCarlo1D(){}
			MonteCarlo1D(Handle<SinglePathPricer> singlePathPricer, Rate underlyingGrowthRate,
						Rate riskFreeRate, double residualTime, double volatility,	
						int timesteps, long seed=0);
			double value(int confnumber) const;
			double errorEstimate() const;
		 private:  
			int confnumber_;
			mutable OptionSample<StandardPathGenerator, SinglePathPricer > optionSample_;
			mutable Statistics sampleAccumulator_;
		};


		inline MonteCarlo1D::MonteCarlo1D(Handle<SinglePathPricer> singlePathPricer, Rate underlyingGrowthRate,   
							Rate riskFreeRate, double residualTime, double volatility,	
							int timesteps, long seed): sampleAccumulator_(){

			double deltaT = residualTime/timesteps;
			double mu = deltaT*(riskFreeRate-underlyingGrowthRate-0.5*volatility*volatility);
			double sigma = volatility*QL_SQRT(deltaT);

			StandardPathGenerator pathGenerator(timesteps, mu, sigma, seed);

			optionSample_ = OptionSample<StandardPathGenerator, SinglePathPricer >
									(pathGenerator, singlePathPricer);

		}

		inline double MonteCarlo1D::value(int confnumber) const{
			for(int i=1;i<=confnumber;i++)
				sampleAccumulator_.add(optionSample_.next(), optionSample_.weight());
			return sampleAccumulator_.mean();
		}

		inline double MonteCarlo1D::errorEstimate() const{ 
			return sampleAccumulator_.errorEstimate(); 
		}

	}

}
#endif
