
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

#include <ql/RandomNumbers/seedgenerator.hpp>

namespace QuantLib {

    MersenneTwisterUniformRng SeedGenerator::rng_(QL_TIME(0));

    /*
    SeedGenerator::SeedGenerator() {

        // firstSeed is chosen based on clock() and used for the first rng
        unsigned long firstSeed = long(QL_TIME(0));
        MersenneTwisterUniformRng first(firstSeed);

        std::vector<unsigned long> init(4);
        init[0]=first.nextInt32();
        init[1]=first.nextInt32();
        init[2]=first.nextInt32();
        init[3]=first.nextInt32();
        MersenneTwisterUniformRng second(init);

        // use the second rng to initialize the final one
        long skip = long(second.next().value * 10000);
        init[0]=second.nextInt32();
        init[1]=second.nextInt32();
        init[2]=second.nextInt32();
        init[3]=second.nextInt32();

        rng_ = MersenneTwisterUniformRng(init);

        for (long i=0; i<skip ; i++)
            rng_.nextInt32();
    }
    */

    unsigned long SeedGenerator::get() {
        return rng_.nextInt32();
    }
}
