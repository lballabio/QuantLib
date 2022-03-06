/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
//
// Copyright (C) 2002 Peter Jäckel "Monte Carlo Methods in Finance".
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

/*! \file primenumbers.hpp
    \brief Prime numbers calculator
*/

#ifndef quantlib_primenumbers_h
#define quantlib_primenumbers_h

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Prime numbers calculator
    /*! Taken from "Monte Carlo Methods in Finance", by Peter Jäckel
     */
    class PrimeNumbers {
      public:
        //! Get and store one after another.
        static BigNatural get(Size absoluteIndex);
      private:
        PrimeNumbers() = default;
        static BigNatural nextPrimeNumber();
        static std::vector<BigNatural> primeNumbers_;
    };

}

#endif


#ifndef id_dd29a7efc9a7d62357de419e6a217529
#define id_dd29a7efc9a7d62357de419e6a217529
inline bool test_dd29a7efc9a7d62357de419e6a217529(int* i) { return i != 0; }
#endif
