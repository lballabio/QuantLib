/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file seedgenerator.hpp
    \brief Random seed generator
*/

#ifndef quantlib_seed_generator_hpp
#define quantlib_seed_generator_hpp

#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/patterns/singleton.hpp>

namespace QuantLib {

    //! Random seed generator
    /*! Random number generator used for automatic generation of
        initialization seeds.

        \test correct initialization of the single instance is tested.
    */
    class SeedGenerator : public Singleton<SeedGenerator> {
        friend class Singleton<SeedGenerator>;
      public:
        unsigned long get();
      private:
        SeedGenerator();
        void initialize();
        MersenneTwisterUniformRng rng_;
    };

}


#endif


#ifndef id_c7de10d8d227908400f2407da7b5f3c2
#define id_c7de10d8d227908400f2407da7b5f3c2
inline bool test_c7de10d8d227908400f2407da7b5f3c2(int* i) { return i != 0; }
#endif
