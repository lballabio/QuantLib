
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
// which was taken from "Monte Carlo Methods in Finance",
// by Peter Jäckel
//
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

/*! \file primenumbers.hpp
    \brief Prime numbers calculator

    \fullpath
    ql/RandomNumbers/%primenumbers.hpp
*/

// $Id$

#ifndef quantlib_primenumbers_h
#define quantlib_primenumbers_h

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    namespace Math {
        //! Prime numbers calculator
        /*! Prime numbers calculator.
            Taken from "Monte Carlo Methods in Finance", by Peter Jäckel
        */
        class PrimeNumbers {
          public:
            PrimeNumbers() : primeNumbers_(15) {
                // the first 2 prime numbeers are necessary
                // for the algorithm to work
                primeNumbers_[ 0] =  2;
                primeNumbers_[ 1] =  3;
                // precomputed .....
                primeNumbers_[ 2] =  5;
                primeNumbers_[ 3] =  7;
                primeNumbers_[ 4] = 11;
                primeNumbers_[ 5] = 13;
                primeNumbers_[ 6] = 17;
                primeNumbers_[ 7] = 19;
                primeNumbers_[ 8] = 23;
                primeNumbers_[ 9] = 29;
                primeNumbers_[10] = 31;
                primeNumbers_[11] = 37;
                primeNumbers_[12] = 41;
                primeNumbers_[13] = 43;
                primeNumbers_[14] = 47;

            }
            //! Get and store one after another.
            Size nextPrimeNumber();
            Size operator[](Size absoluteIndex) {
                while (primeNumbers_.size()<=absoluteIndex)
                    nextPrimeNumber();
                return primeNumbers_[absoluteIndex];
            }
          private:
            std::vector<Size> primeNumbers_;
        };

    }

}

#endif
