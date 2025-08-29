/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_mt_brownian_generator_hpp
#define quantlib_mt_brownian_generator_hpp

#include <ql/models/marketmodels/browniangenerator.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    //! Mersenne-twister Brownian generator for market-model simulations
    /*! Incremental Brownian generator using a Mersenne-twister
        uniform generator and inverse-cumulative Gaussian method.

        \note At this time, generation of the underlying uniform
              sequence is eager, while its transformation into
              Gaussian variates is lazy.  Further optimization might
              be possible by using the Mersenne twister directly
              instead of a RandomSequenceGenerator; however, it is not
              clear how much of a difference this would make when
              compared to the inverse-cumulative Gaussian calculation.
    */
    class MTBrownianGenerator : public BrownianGenerator {
      public:
        MTBrownianGenerator(Size factors,
                            Size steps,
                            unsigned long seed = 0);

        Real nextStep(std::vector<Real>&) override;
        Real nextPath() override;

        Size numberOfFactors() const override;
        Size numberOfSteps() const override;

      private:
        Size factors_, steps_;
        Size lastStep_ = 0;
        RandomSequenceGenerator<MersenneTwisterUniformRng> generator_;
        InverseCumulativeNormal inverseCumulative_;
    };

    class MTBrownianGeneratorFactory : public BrownianGeneratorFactory {
      public:
        MTBrownianGeneratorFactory(unsigned long seed = 0);
        ext::shared_ptr<BrownianGenerator> create(Size factors, Size steps) const override;

      private:
        unsigned long seed_;
    };

}


#endif
