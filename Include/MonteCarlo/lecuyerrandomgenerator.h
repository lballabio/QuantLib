
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

/*! \file lecuyerrandomgenerator.h
    \brief L'Ecuyer uniform random-number generator

    $Source$
    $Name$
    $Log$
    Revision 1.1  2000/12/27 15:23:39  marmar
    Random number generators has been updated and documented.
    Now the Sample Generator idea is fully implemented

*/

#ifndef ql_lecuyer_random_generator_h
#define ql_lecuyer_random_generator_h

#include "qldefines.h"
#include <string>
#include <ctime>
#include <algorithm>
#include <vector>

namespace QuantLib {

    namespace MonteCarlo {

    /*!  Random number generator of L'Ecuyer with added Bays-Durham shuffle.
        See Section 7.1 of Numerical Recipes in C, 2nd Edition, Cambridge University Press
        for more details. 
	*/

        class LecuyerRandomGenerator {
        public:
            // constructor
            explicit LecuyerRandomGenerator(long seed = 0);
            // random number uniformly chosen from (0.0,1.0)
			typedef double SampleType;
            double next() const;
            double weight() const;
        private:
            mutable long temp1, temp2;
            mutable long y;
            mutable std::vector<long> buffer;
            static const long m1, a1, q1, r1;
            static const long m2;
            static const long a2;
            static const long q2;
            static const long r2;
            static const int bufferSize;
            static const long bufferNormalizer;
            static const long double maxRandom;
        };

        inline LecuyerRandomGenerator::LecuyerRandomGenerator(long seed) : buffer(LecuyerRandomGenerator::bufferSize) {
            temp2 = temp1 = (seed != 0 ? seed : long(QL_CLOCK()));
            for (int j=bufferSize+7; j>=0; j--) {
                long k = temp1/q1;
                temp1 = a1*(temp1-k*q1)-k*r1;
                if (temp1 < 0)
                    temp1 += m1;
                if (j < bufferSize)
                    buffer[j] = temp1;
            }
            y = buffer[0];
        }

        inline double LecuyerRandomGenerator::next() const {
            long k = temp1/q1;
            temp1 = a1*(temp1-k*q1)-k*r1;
            if (temp1 < 0)
                temp1 += m1;
            k = temp2/q2;
            temp2 = a2*(temp2-k*q2)-k*r2;
            if (temp2 < 0)
                temp2 += m2;
            int j = y/bufferNormalizer;
            y = buffer[j]-temp2;
            buffer[j] = temp1;
            if (y < 1)
                y += m1-1;
            double result = y/double(m1);
            return (result > maxRandom ? maxRandom : result);
        }

        inline double LecuyerRandomGenerator::weight() const {
            return 1.0;
        }
    }

}


#endif
