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

/*! \file boxmullergaussian.h
	\brief Box-Muller Gaussian random number generator
	
	$Source$
	$Name$
	$Log$
	Revision 1.2  2000/12/18 18:31:17  lballabio
	Added CVS tags

*/

#ifndef quantlib_box_muller_transformation_h
#define quantlib_box_muller_transformation_h

#include "qldefines.h"

namespace QuantLib {

	namespace Math {	

		// BoxMuller<UniformZeroOne> is a class, with the minimal interface of a random
		// number generator, which returns a normal distributed gaussian deviate with
		// average 0.0 and standard deviation 1.0, from a uniform deviate in (0,1)
		// given by UniformZeroOne, using the well-known Box-Muller transformation.
		//
		// Class UniformZeroOne must have, at least, the following interface:
		//
		//		UniformZeroOne::UniformZeroOne(long seed=0);	// the constructor
		//		double UniformZeroOne::next() const;			// returns the next random numer

		template <class UniformZeroOne>
		class BoxMuller {
			public:
				BoxMuller(long seed=0) : basicGenerator(seed), returnFirst(true){}
				double next() const;
			private:		
				UniformZeroOne basicGenerator;		
				mutable bool returnFirst;		  
				mutable double firstValue,secondValue;
		};

		template <class UniformZeroOne>
		inline double BoxMuller<UniformZeroOne>::next() const {
			if(returnFirst) {
				double x1,x2,r,ratio;
				do {
					x1 = basicGenerator.next()*2-1;
					x2 = basicGenerator.next()*2-1;
					r = x1*x1+x2*x2;
				} while(r>=1 || r==0);

				ratio = QL_SQRT(-2.0*QL_LOG(r)/r);
				firstValue = x1*ratio;
				secondValue = x2*ratio;

				returnFirst = false;
				return firstValue;
			} else {
				returnFirst = true;
				return secondValue;		
			}
		}
	}
}

#endif
