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

/*! \file knuthuniformrng.hpp
    \brief Knuth uniform random number generator
*/

#ifndef quantlib_knuth_uniform_rng_h
#define quantlib_knuth_uniform_rng_h

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    //! Uniform random number generator
    /*! Random number generator by Knuth.
        For more details see Knuth, Seminumerical Algorithms,
        3rd edition, Section 3.6.
        \note This is <b>not</b> Knuth's original implementation which
              is available at
              http://www-cs-faculty.stanford.edu/~knuth/programs.html,
              but rather a slightly modified version wrapped in a C++ class.
              Such modifications did not affect the code but only the data
              structures used, which were converted to their standard C++
              equivalents.
    */
    class KnuthUniformRng {
      public:
        typedef Sample<Real> sample_type;
        /*! if the given seed is 0, a random seed will be chosen
            based on clock() */
        explicit KnuthUniformRng(long seed = 0);
        /*! returns a sample with weight 1.0 containing a random number
          uniformly chosen from (0.0,1.0) */
        sample_type next() const;
      private:
        static const int KK, LL, TT, QUALITY;
        mutable std::vector<double> ranf_arr_buf;
        mutable size_t ranf_arr_ptr, ranf_arr_sentinel;
        mutable std::vector<double> ran_u;
        double mod_sum(double x, double y) const;
        bool is_odd(int s) const;
        void ranf_start(long seed);
        void ranf_array(std::vector<double>& aa, int n) const;
        double ranf_arr_cycle() const;
    };


    // inline definitions

    inline KnuthUniformRng::sample_type KnuthUniformRng::next() const {
        double result = (ranf_arr_ptr != ranf_arr_sentinel ?
                         ranf_arr_buf[ranf_arr_ptr++] :
                         ranf_arr_cycle());
        return {result, 1.0};
    }

    inline double KnuthUniformRng::mod_sum(double x, double y) const {
        return (x+y)-int(x+y);
    }

    inline bool KnuthUniformRng::is_odd(int s) const {
        return (s&1) != 0;
    }

}


#endif
