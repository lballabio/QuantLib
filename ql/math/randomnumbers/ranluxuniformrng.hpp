/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file ranluxuniformrng.hpp
    \brief "Luxury" random number generator.
*/

#ifndef quantlib_ranlux_uniform_rng_h
#define quantlib_ranlux_uniform_rng_h

#include <ql/methods/montecarlo/sample.hpp>

#include <random>

namespace QuantLib {

    //! Uniform random number generator
    /*! M. Luescher's "luxury" random number generator

        Implementation is a proxy for the corresponding boost random
        number generator. For more detail see the boost documentation and:
          M.Luescher, A portable high-quality random number generator for
          lattice field theory simulations, Comp. Phys. Comm. 79 (1994) 100
          
        Available luxury levels:
        Ranlux3: Any theoretically possible correlations have very small change
                 of being observed.
        Ranlux4: highest possible luxury.         
    */
    template <std::size_t P, std::size_t R>
    class Ranlux64UniformRng {
      public:
        typedef Sample<Real> sample_type;

        explicit Ranlux64UniformRng(std::uint_fast64_t seed = 19780503U)
        : ranlux_(ranlux64_base_01(seed)) {}

        sample_type next() const { return {ranlux_()*nx, 1.0}; }

      private:
        const double nx = 1.0/(std::uint_fast64_t(1) << 48);
        typedef std::subtract_with_carry_engine<std::uint_fast64_t, 48, 10, 24>
            ranlux64_base_01;
        mutable std::discard_block_engine<ranlux64_base_01, P, R> ranlux_;
    };

    typedef Ranlux64UniformRng<223, 24> Ranlux3UniformRng;
    typedef Ranlux64UniformRng<389, 24> Ranlux4UniformRng;
}


#endif
