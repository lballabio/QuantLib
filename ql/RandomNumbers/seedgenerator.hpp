
/*
 Copyright (C) 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file seedgenerator.hpp
    \brief Random seed generator
*/

#ifndef quantlib_seedgenerator_hpp
#define quantlib_seedgenerator_hpp

#include <ql/RandomNumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    //! Random seed generator
    /*! Random number generator used for automatic generation of
        initialization seeds.
    */
    class SeedGenerator {
      public:
        //! Get a new seed
        static unsigned long get();
      private:
        SeedGenerator();
        static MersenneTwisterUniformRng rng_;
    };

}


#endif
