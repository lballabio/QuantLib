
// ===========================================================================
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
            PrimeNumbers() {}
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
