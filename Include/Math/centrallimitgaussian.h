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

#ifndef quantlib_central_limit_gaussian_transformation_h
#define quantlib_central_limit_gaussian_transformation_h

#include "qldefines.h"

namespace QuantLib {

	namespace Math {
	
		// CLGaussian<UniformZeroOne> is a class, with the minimal interface of a random
		// number generator, which returns a normal distributed gaussian deviate with
		// average 0.0 and standard deviation 1.0, from a uniform deviate in (0,1)
		// given by UniformZeroOne, using the well-known approximation that the sum
		// of twelve random x_i unform on (0,1), is almost Gaussian.
		//
		// Class UniformZeroOne must have, at least, the following interface:
		//
		//		UniformZeroOne::UniformZeroOne(long seed=0);	// the constructor
		//		double UniformZeroOne::next() const;			// returns the next random numer

		template <class UniformZeroOne>
		class CLGaussian {
			public:
				CLGaussian(long seed=0) : basicGenerator(seed) {}
				double next() const;
			private:		
				UniformZeroOne basicGenerator;		
		};

		template <class UniformZeroOne>
		inline double CLGaussian<UniformZeroOne>::next() const {

			double gaussPoint = -6.0;
			for(int i=1;i<=12;i++)
				gaussPoint += basicGenerator.next();
			return gaussPoint;
		}

	}

}

#endif
