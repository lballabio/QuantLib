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

/*! \file boxmuller.h
	\brief Box-Muller Gaussian random-number generator
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2000/12/27 15:23:39  marmar
	Random number generators has been updated and documented.
	Now the Sample Generator idea is fully implemented

*/

#ifndef quantlib_box_muller_transformation_h
#define quantlib_box_muller_transformation_h

#include "qldefines.h"

namespace QuantLib {

	namespace MonteCarlo {

		//! Gaussian random number generator
		/*! It uses the well-known Box-Muller transformation to return a normal 
			distributed Gaussian deviate with average 0.0 and standard deviation 1.0, 
			from a uniform deviate in (0,1) supplied by U.
		
			Class U must have, at least, the following interface:
			\code
				U::U(long seed);	
				double U::next() const;	
			    double U::weight() const;
			\endcode
		*/
		template <class U>
		class BoxMuller {
		public:
			BoxMuller(long seed=0);
			typedef double SampleType;
			//! returns next sample from the Gaussian distribution
			double next() const;
			//! returns the weight of the last extracted sample
            double weight() const;
		private:		
			U basicGenerator;		
			mutable double weight_;
			mutable bool returnFirst;		  
			mutable double firstValue,secondValue;
			mutable double firstWeight,secondWeight;
		};

		template <class U>
		BoxMuller<U>::BoxMuller(long seed): 
		    basicGenerator(seed), returnFirst(true), weight_(0.0){}

		template <class U>
		inline double BoxMuller<U>::next() const {
			if(returnFirst) {
				double x1,x2,r,ratio;
				do {
					x1 = basicGenerator.next()*2-1;
					firstWeight = basicGenerator.weight();
					x2 = basicGenerator.next()*2-1;
					secondWeight = basicGenerator.weight();
					r = x1*x1+x2*x2;
				} while(r>=1 || r==0);

				ratio = QL_SQRT(-2.0*QL_LOG(r)/r);
				firstValue = x1*ratio;
				secondValue = x2*ratio;
				weight_ = firstWeight*secondWeight;

				returnFirst = false;
				return firstValue;
			} else {
				returnFirst = true;
				return secondValue;		
			}
		}

		template <class U>
        inline double BoxMuller<U>::weight() const {
            return weight_;
        }

	}

}

#endif
