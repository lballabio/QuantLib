/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file lecuyeruniformrng.hpp
    \brief L'Ecuyer uniform random number generator
*/

#ifndef quantlib_lecuyer_uniform_rng_h
#define quantlib_lecuyer_uniform_rng_h

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    //! Uniform random number generator
    /*! Random number generator of L'Ecuyer with added Bays-Durham
        shuffle (know as ran2 in Numerical recipes)

        For more details see Section 7.1 of Numerical Recipes in C, 2nd
        Edition, Cambridge University Press (available at
        http://www.nr.com/)
    */
    class LecuyerUniformRng {
      public:
        typedef Sample<Real> sample_type;
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
    };

}


#endif
