
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

/*! \file primenumbers.cpp
    \brief Prime numbers calculator
*/

// $Id$

#include "ql/Math/primenumbers.hpp"
#include <cmath>

namespace QuantLib {

    namespace Math {

        namespace {
            const unsigned long firstPrimes[] = {
                // the first two primes are necessary for bootstrapping
                 2,  3, 
                // additional precomputed primes
                 5,  7, 11, 13, 17, 19, 23, 29,
                31, 37, 41, 43, 47 };
        }

        std::vector<unsigned long> PrimeNumbers::primeNumbers_;

        unsigned long PrimeNumbers::get(Size absoluteIndex) {
            if (primeNumbers_.empty()) {
                Size n = sizeof(firstPrimes)/sizeof(firstPrimes[0]);
                std::copy(firstPrimes, firstPrimes+n,
                          std::back_inserter(primeNumbers_));
            }
            while (primeNumbers_.size()<=absoluteIndex)
                nextPrimeNumber();
            return primeNumbers_[absoluteIndex];
        }

        Size PrimeNumbers::nextPrimeNumber() {
            Size p, n, m = primeNumbers_.back();
            do {
                // skip the even numbers
                m += 2;
                n = static_cast<Size>(QL_SQRT(double(m)));
                // it can be i==1 since the even numbers have been skipped
                Size i = 1;
                do {
                    p = primeNumbers_[i];
                    ++i;
                } while ( m%p && p<=n );
            } while ( p<=n );
            primeNumbers_.push_back(m);
            return m;
        }

    }

}
