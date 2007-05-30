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

#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ctime>
#if defined(BOOST_NO_STDC_NAMESPACE)
    namespace std { using ::time; }
#endif

namespace QuantLib {

    // we need to prevent rng from being default-initialized
    SeedGenerator::SeedGenerator() : rng_(42UL) {
        initialize();
    }

    void SeedGenerator::initialize() {

        // firstSeed is chosen based on clock() and used for the first rng
        unsigned long firstSeed = (unsigned long)(std::time(0));
        MersenneTwisterUniformRng first(firstSeed);

        // secondSeed is as random as it could be
        // feel free to suggest improvements
        unsigned long secondSeed = first.nextInt32();

        MersenneTwisterUniformRng second(secondSeed);

        // use the second rng to initialize the final one
        unsigned long skip = second.nextInt32() % 1000;
        std::vector<unsigned long> init(4);
        init[0]=second.nextInt32();
        init[1]=second.nextInt32();
        init[2]=second.nextInt32();
        init[3]=second.nextInt32();

        rng_ = MersenneTwisterUniformRng(init);

        for (unsigned long i=0; i<skip ; i++)
            rng_.nextInt32();
    }

    unsigned long SeedGenerator::get() {
        return rng_.nextInt32();
    }

}
