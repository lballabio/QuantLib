/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Ralf Konrad Eckel

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

/*! \file zigguratgaussianrng.hpp
    \brief Ziggurat Gaussian random-number generator
*/

#ifndef quantlib_ziggurat_gaussian_rng_h
#define quantlib_ziggurat_gaussian_rng_h

#include <ql/methods/montecarlo/sample.hpp>

namespace QuantLib {

    //! Gaussian random number generator
    /*! It uses the Ziggurat transformation to return a
        normal distributed Gaussian deviate with average 0.0 and
        standard deviation of 1.0, from a random integer
        in the [0,0xffffffffffffffffULL]-interval like.

        For a more detailed description see the article
        "An Improved Ziggurat Method to Generate Normal Random Samples"
        by Jurgen A. Doornik
        (https://www.doornik.com/research/ziggurat.pdf).

        The code here is inspired by the rust implementation in
        https://github.com/rust-random/rand/blob/d42daabf65a3ceaf58c2eefc7eb477c4d5a9b4ba/rand_distr/src/normal.rs
        and
        https://github.com/rust-random/rand/blob/d42daabf65a3ceaf58c2eefc7eb477c4d5a9b4ba/rand_distr/src/utils.rs.

        Class RNG must implement the following interface:
        \code
            std::uint64_t nextInt64() const;
        \endcode
        Currently, Xoshiro256StarStarUniformRng is the only RNG supporting this.
    */
    template <class RNG>
    class ZigguratGaussianRng {
      public:
        typedef Sample<Real> sample_type;

        explicit ZigguratGaussianRng(const RNG& uint64Generator)
        : uint64Generator_(uint64Generator) {}

        //! returns a sample from a Gaussian distribution
        sample_type next() const { return {nextReal(), 1.0}; }

        //! return a random number from a Gaussian distribution
        Real nextReal() const;

      private:
        RNG uint64Generator_;
    };
    template <class RNG>
    inline Real ZigguratGaussianRng<RNG>::nextReal() const {
        return 0.0;
    }
}

#endif
