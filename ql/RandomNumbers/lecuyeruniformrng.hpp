
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file lecuyeruniformrng.hpp
    \brief L'Ecuyer uniform random number generator

    \fullpath
    ql/RandomNumbers/%lecuyeruniformrng.hpp
*/

// $Id$

#ifndef quantlib_lecuyer_uniform_rng_h
#define quantlib_lecuyer_uniform_rng_h

#include "ql/MonteCarlo/sample.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace QuantLib {

    namespace RandomNumbers {

        //! Uniform random number generator
        /*! Random number generator of L'Ecuyer with added Bays-Durham 
            shuffle.
            For more details see Section 7.1 of Numerical Recipes in C, 2nd
            Edition, Cambridge University Press (available at
            http://www.nr.com/)
        */
        class LecuyerUniformRng {
          public:
            typedef MonteCarlo::Sample<double> sample_type;
            /*! if the given seed is 0, a random seed will be chosen
                based on clock() */
            explicit LecuyerUniformRng(long seed = 0);
            /*! returns a sample with weight 1.0 containing a random number 
                uniformly chosen from (0.0,1.0) */
            sample_type next() const;
          private:
            mutable long temp1, temp2;
            mutable long y;
            mutable std::vector<long> buffer;
            static const long m1;
            static const long a1;
            static const long q1;
            static const long r1;
            static const long m2;
            static const long a2;
            static const long q2;
            static const long r2;
            static const int bufferSize;
            static const long bufferNormalizer;
            static const long double maxRandom;
        };

        inline LecuyerUniformRng::LecuyerUniformRng(long seed) 
        : buffer(LecuyerUniformRng::bufferSize) {
            temp2 = temp1 = (seed != 0 ? seed : long(QL_TIME(0)));
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

        inline LecuyerUniformRng::sample_type 
        LecuyerUniformRng::next() const {
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
            if (result > maxRandom)
                result = maxRandom;
            return sample_type(result,1.0);
        }

    }

}


#endif
