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
/*! \file antitheticcv.h
	\brief Generates an antithetic control variate from a path generator
	
	$Source$
	$Name$
	$Log$
	Revision 1.2  2001/01/17 11:54:02  marmar
	Some documentation added and 80 columns format enforced.

	Revision 1.1  2001/01/04 17:31:22  marmar
	Alpha version of the Monte Carlo tools.
	
*/


#ifndef quantlib_montecarlo_antithetic_control_variate_h
#define quantlib_montecarlo_antithetic_control_variate_h

#include "qldefines.h"

namespace QuantLib {

	namespace MonteCarlo {
		/*! Returns a path generator that gives an antithetic
		    control variate on the given path generator PG.
			PG must be have a method average().
		*/
		template <class PG>
		class AntitheticCV {
		public:				  
			AntitheticCV(): weight_(0){}		  		
			AntitheticCV(int dimension, long seed=0);
			const Path& next() const;
			const double weight() const{ return weight_;}
		private:		
			mutable bool returnPositive;		
			mutable Path path_;
			mutable double weight_;
			PG rndPath;
		};

		template <class PG>
		inline AntitheticCV<PG>::AntitheticCV(int dimension, long seed):
		            returnPositive(true), path_(dimension), 
		            rndPath(dimension, seed){}  

		template <class PG>
		inline const Path& AntitheticCV<PG>::next() const{		
			if(returnPositive) {
				path_ = rndPath.next();
				weight_ = rndPath.weight();
			}
			else 
				path_ = 2.0*rndPath.average() - path_;
			returnPositive = !returnPositive;
			return path_;	
		}

	}

}

#endif
