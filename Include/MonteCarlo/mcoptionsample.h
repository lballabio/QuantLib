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
/*! \file mcoptionsample.h
	\brief Create a sample generator from a path generator and a single-path pricer
	
	$Source$
	$Name$
	$Log$
	Revision 1.2  2001/01/17 11:54:02  marmar
	Some documentation added and 80 columns format enforced.

	Revision 1.1  2001/01/04 17:31:22  marmar
	Alpha version of the Monte Carlo tools.
			
*/

#ifndef quantlib_montecarlo_option_sample_h
#define quantlib_montecarlo_option_sample_h

#include "qldefines.h"
#include "handle.h"

namespace QuantLib {

	namespace MonteCarlo {
	/*!
	Given a path generator class PG, together with an instance "samplepath", 
	and a single-path pricer SPP, again with an instance singlepathpricer, 
	a sample generator OptionSample<PG, SPP> returns at each next a value
	for the option price.

	Minimal interfaces for PG and SPP:

	class PG{	
		PATH_TYPE next() const;
		double weight() const;		
	};

	class SPP{
		double value(PATH_TYPE &) const;	// this will eventually evolve into
											// SPP::ValueType value() const;
	};
	*/

		template<class PG, class SPP>
		class OptionSample {
		public:		
			OptionSample(){}
			OptionSample(const PG& samplepath, Handle<SPP> singlePathPricer);			
			double next() const; // this will eventually evolve into
								 // SPP::ValueType next() const;
			double weight() const;
		private:  
			mutable PG samplePath_;
			mutable double weight_;
			Handle<SPP> singlePathPricer_;
		};

		template<class PG, class SPP>
		inline OptionSample<PG, SPP>::OptionSample(const PG& samplePath, 
	           Handle<SPP> singlePathPricer): samplePath_(samplePath), 
	           singlePathPricer_(singlePathPricer), weight_(0){}

		template<class PG, class SPP>
		inline double OptionSample<PG, SPP>::next() const{
			double price = singlePathPricer_ -> value(samplePath_.next());
			weight_ = samplePath_.weight();
			return price;
		}

		template<class PG, class SPP>
		inline double OptionSample<PG, SPP>::weight() const{
			return weight_;
		}

	}

}
#endif
