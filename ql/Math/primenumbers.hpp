
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
            PrimeNumbers() : primeNumbers_(2) {
                primeNumbers_[0]=2;
                primeNumbers_[1]=3;
            }
            //! Get and store one after another.
            Size nextPrimeNumber();

            Size indexedPrimeNumber(Size absoluteIndex) {
                while (primeNumbers_.size()<=absoluteIndex)
                    nextPrimeNumber();
                return primeNumbers_[absoluteIndex];
            }

            Size operator[](Size absoluteIndex) {
                return indexedPrimeNumber(absoluteIndex);
            }
          private:
            std::vector<Size> primeNumbers_;
        };

    }

}

#endif
