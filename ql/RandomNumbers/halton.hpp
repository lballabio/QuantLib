
// ===========================================================================
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

/*! \file halton.hpp
    \brief Halton low-discrepancy sequence generator

    \fullpath
    ql/RandomNumbers/%halton.hpp
*/

// $Id$

#ifndef quantlib_halton_lds_h
#define quantlib_halton_lds_h

#include "ql/Math/primenumbers.hpp"
#include <cmath>

namespace QuantLib {

    namespace RandomNumbers {
        //! Halton low-discrepancy sequence generator
        /*! Halton algorithm for low-discrepancy sequence.
            For more details see chapter 8, paragraph 2 of
            "Monte Carlo Methods in Finance", by Peter Jäckel
        */
        class Halton {
          public:
            Halton(Size dimensionality)
            : dimensionality_(dimensionality), sequenceCounter_(0),
              sequenceVector_(dimensionality) {}

            const std::vector<double>& nextUniformVector();
          private:
            Size dimensionality_, sequenceCounter_;
            std::vector<double> sequenceVector_;
            Math::PrimeNumbers primeNumbers_;
        };
    }

}

#endif
