
// ===========================================================================
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#include "ql/Math/primenumbers.hpp"
#include <cmath>

namespace QuantLib {

    namespace Math {

        Size PrimeNumbers::nextPrimeNumber() {
            Size m;
            if (primeNumbers_.size()) {
                Size i, n, p;
                m = primeNumbers_.back();
                do {
                    i = 0;
                    ++m;
                    n = static_cast<Size>(QL_SQRT(m));
                    do {
                        p = primeNumbers_[i];
                        ++i;
                    } while ( m%p && p<=n );
                } while ( p<=n );
            } else {
              m=2;
            }
            primeNumbers_.push_back(m);
            return m;
        }

    }

}
