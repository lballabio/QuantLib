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
/*! \file mcpricer.h
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2001/01/04 17:31:22  marmar
	Alpha version of the Monte Carlo tools.

*/

#ifndef quantlib_montecarlo_pricer_h
#define quantlib_montecarlo_pricer_h

#include "qldefines.h"
#include "montecarlo1d.h"

namespace QuantLib {

	namespace Pricers {

		using MonteCarlo::MonteCarlo1D;

		class McPricer {
		public:
			McPricer():isInitialized_(false){}
			McPricer(int confnumber, long seed=0);
			~McPricer(){}
			virtual double value() const;
			virtual double errorEstimate() const;
		protected:		
			bool isInitialized_;
			long seed_;
			mutable double confnumber_;
			mutable MonteCarlo1D montecarloPricer_;
		};

		inline McPricer::McPricer(int confnumber, long seed):
					confnumber_(confnumber), seed_(seed), isInitialized_(true){}

		inline double McPricer::value() const{
			QL_REQUIRE(isInitialized_,"McPricer::value has not been initialized");
			return montecarloPricer_.value(confnumber_);
		}
		
		inline double McPricer::errorEstimate() const{ 
			QL_REQUIRE(isInitialized_,"McPricer::errorEstimate has not been initialized");
			return montecarloPricer_.errorEstimate(); 
		}

	}

}
#endif
