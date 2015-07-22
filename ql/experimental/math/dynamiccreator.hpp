/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/* This is a wrapper for the original C code "Dynamic Creator of Mersenne
 * Twisters Ver. 0.6 (2009/12/15)"
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/DC/dc.html
*/

/*
  Copyright (C) 2001-2009 Makoto Matsumoto and Takuji Nishimura.
  Copyright (C) 2009 Mutsuo Saito
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*! \file dynamiccreator.hpp
    \brief dynamic creator library
*/

#ifndef quantlib_dynamic_creator_hpp
#define quantlib_dynamic_creator_hpp

#include <ql/methods/montecarlo/sample.hpp>
#include <ql/errors.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <boost/cstdint.hpp>

namespace QuantLib {

// original code is placed in namespace mt_detail
// macros N and M were renamed to dcmt_N and dcmt_M respectively
// macros can not be hidden in namespaces, we still leave the
// others defined in this header unchanged
// (NOT_REJECTED, REJECTED, REDU, IRRED, NONREDU)
// we commented out the "old" interface since we do not use it
// in the wrapper anyway as well as code activated in debug mode
// only

namespace mt_detail {
// dc.h
typedef struct {
    uint32_t aaa;
    int mm, nn, rr, ww;
    uint32_t wmask, umask, lmask;
    int shift0, shift1, shiftB, shiftC;
    uint32_t maskB, maskC;
    int i;
    uint32_t *state;
} mt_struct;
} // namespace mt_detail

// to store a precomputed mt instance
struct MersenneTwisterDynamicRngDescription {
    uint32_t creatorSeed_; // just to make it reproduceable
    uint16_t id_;          // just to make it reproducable
    int w, p;
    uint32_t aaa;
    int mm, nn, rr, ww;
    uint32_t wmask, umask, lmask;
    int shift0, shift1, shiftB, shiftC;
    uint32_t maskB, maskC;
    int i;
};

// output description as struct that can
// be used to setup an instance by a
// template parameter
inline std::ostream &operator<<(std::ostream &out,
                                const MersenneTwisterDynamicRngDescription &d) {
    return out << "struct Mtdesc {\n"
               << "/* created with creator seed " << d.creatorSeed_
               << " and id " << d.id_ << " */\n"
               << "static const int w = " << d.w << ";\n"
               << "static const int p = " << d.p << ";\n"
               << "static const uint32_t aaa = " << d.aaa << "UL;\n"
               << "static const int mm = " << d.mm << ";\n"
               << "static const int nn = " << d.nn << ";\n"
               << "static const int rr = " << d.rr << ";\n"
               << "static const int ww = " << d.ww << ";\n"
               << "static const uint32_t wmask = " << d.wmask << "UL;\n"
               << "static const uint32_t umask = " << d.umask << "UL;\n"
               << "static const uint32_t lmask = " << d.lmask << "UL;\n"
               << "static const int shift0 = " << d.shift0 << ";\n"
               << "static const int shift1 = " << d.shift1 << ";\n"
               << "static const int shiftB = " << d.shiftB << ";\n"
               << "static const int shiftC = " << d.shiftC << ";\n"
               << "static const uint32_t maskB = " << d.maskB << "UL;\n"
               << "static const uint32_t maskC = " << d.maskC << "UL;\n"
               << "};\n";
}

// Use this class only if you want to dynamically create a mt during runtime
// It is faster to use precomputed instances with the class
// MersenneTwisterCustomRng below. The constructor taking a description is
// kept for convenience reasons here only

class MersenneTwisterDynamicRng {

  public:
    typedef Sample<Real> sample_type;

    // if given seed is 0 then a clock based seed is used

    // create a mt instance with word size w (31, 32) and period 2^p-1
    // this can take very long for a larger p
    // the id is incorporated into the rng such that rng with different
    // ids are highly independent
    // usable p for periods 2^p-1 are 521   607  1279  2203
    // 2281  3217  4253  4423 9689  9941 11213 19937 21701
    // 23209 44497
    MersenneTwisterDynamicRng(const int w = 32, const int p = 521,
                              const uint32_t creatorSeed = 42,
                              const uint16_t id = 0, const uint32_t seed = 0);

    // create mt from saved description
    MersenneTwisterDynamicRng(const MersenneTwisterDynamicRngDescription &d,
                              const uint32_t seed = 0);

    ~MersenneTwisterDynamicRng();

    void resetSeed(const uint32_t seed);
    sample_type next();
    Real nextReal();
    unsigned long operator()();
    unsigned long nextInt32();
    void discard(uint64_t z);
    MersenneTwisterDynamicRngDescription description() const;

  private:
    // hide copy and assignment constructors
    MersenneTwisterDynamicRng(const MersenneTwisterDynamicRng &);
    MersenneTwisterDynamicRng &operator=(const MersenneTwisterDynamicRng &);
    const int w_, p_;
    uint32_t creatorSeed_;
    uint16_t id_;
    mt_detail::mt_struct *m_;
};

inline unsigned long MersenneTwisterDynamicRng::operator()() {
    return nextInt32();
}

template <class Description> class MersenneTwisterCustomRng {

  public:
    typedef Sample<Real> sample_type;

    // if given seed is 0 then a clock based seed is used
    MersenneTwisterCustomRng(const uint32_t seed = 0);
    ~MersenneTwisterCustomRng();

    void resetSeed(const uint32_t seed);
    sample_type next();
    Real nextReal();
    unsigned long operator()();
    unsigned long nextInt32();
    void discard(uint64_t z);

  private:
    void twist();
    // hide copy and assignment constructors
    MersenneTwisterCustomRng(const MersenneTwisterCustomRng &);
    MersenneTwisterCustomRng &operator=(const MersenneTwisterCustomRng &);
    uint32_t *state_;
    int i_;
};

template <class Description>
inline unsigned long MersenneTwisterCustomRng<Description>::operator()() {
    return nextInt32();
}

template <class Description>
MersenneTwisterCustomRng<Description>::MersenneTwisterCustomRng(
    const uint32_t seed) {
    state_ = (uint32_t *)malloc((Description::p / Description::w + 1) *
                                sizeof(uint32_t));
    QL_REQUIRE(state_ != NULL, "can not allocate state space");
    resetSeed(seed);
}

template <class Description>
MersenneTwisterCustomRng<Description>::~MersenneTwisterCustomRng() {
    free(state_);
}

template <class Description>
void MersenneTwisterCustomRng<Description>::resetSeed(const uint32_t seed) {
    uint32_t tmpSeed = seed;
    if (tmpSeed == 0)
        tmpSeed = SeedGenerator::instance().get();
    // sgenrand_mt
    for (int i = 0; i < Description::nn; ++i) {
        state_[i] = tmpSeed;
        tmpSeed = (UINT32_C(1812433253) * (tmpSeed ^ (tmpSeed >> 30))) + i + 1;
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
    }
    i_ = Description::nn;
    for (int i = 0; i < Description::nn; ++i)
        state_[i] &= Description::wmask;
    // end sgendrand_mt
}

template <class Description>
typename MersenneTwisterCustomRng<Description>::sample_type
MersenneTwisterCustomRng<Description>::next() {
    return sample_type(nextReal(), 1.0);
}

template <class Description>
Real MersenneTwisterCustomRng<Description>::nextReal() {
    return (Real(nextInt32()) + 0.5) /
           (Description::w == 32 ? 4294967296.0 : 2147483648.0);
}

template <class Description>
void MersenneTwisterCustomRng<Description>::discard(uint64_t z) {
    for (; z != 0ULL; --z)
        // genrant_mt
        if (i_++ >= Description::nn) {
            twist();
        }
    // end genrand_mt
}

template <class Description>
unsigned long MersenneTwisterCustomRng<Description>::nextInt32() {
    // genrant_mt
    uint32_t x;
    if (i_ >= Description::nn) {
        twist();
    }
    x = state_[i_++];
    x ^= x >> Description::shift0;
    x ^= (x << Description::shiftB) & Description::maskB;
    x ^= (x << Description::shiftC) & Description::maskC;
    x ^= x >> Description::shift1;
    return static_cast<unsigned long>(x);
    // end genrand_mt
}

template <class Description>
void MersenneTwisterCustomRng<Description>::twist() {
    uint32_t x;
    int k;
    for (k = 0; k < Description::nn - Description::mm; ++k) {
        x = (state_[k] & Description::umask) |
            (state_[k + 1] & Description::lmask);
        state_[k] = state_[k + Description::mm] ^ (x >> 1) ^
                    (x & 1U ? Description::aaa : 0U);
    }
    for (; k < Description::nn - 1; ++k) {
        x = (state_[k] & Description::umask) |
            (state_[k + 1] & Description::lmask);
        state_[k] = state_[k + Description::mm - Description::nn] ^ (x >> 1) ^
                    (x & 1U ? Description::aaa : 0U);
    }
    x = (state_[Description::nn - 1] & Description::umask) |
        (state_[0] & Description::lmask);
    state_[Description::nn - 1] = state_[Description::mm - 1] ^ (x >> 1) ^
                                  (x & 1U ? Description::aaa : 0U);
    i_ = 0;
}

// precomputed instances

// 8 instances with p=19937, w=32

struct Mtdesc19937_0 {
    /* created with creator seed 42 and id 4145 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 2711425073UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 4007967616UL;
    static const uint32_t maskC = 2008121344UL;
};

struct Mtdesc19937_1 {
    /* created with creator seed 42 and id 4147 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 2990673971UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 769062528UL;
    static const uint32_t maskC = 3470098432UL;
};

struct Mtdesc19937_2 {
    /* created with creator seed 42 and id 4146 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 3279884338UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 3151424896UL;
    static const uint32_t maskC = 3749019648UL;
};

struct Mtdesc19937_3 {
    /* created with creator seed 42 and id 4140 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 2902986796UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 2607987584UL;
    static const uint32_t maskC = 4149641216UL;
};

struct Mtdesc19937_4 {
    /* created with creator seed 42 and id 4143 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 3975614511UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 3142380672UL;
    static const uint32_t maskC = 4023877632UL;
};

struct Mtdesc19937_5 {
    /* created with creator seed 42 and id 4142 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 3431665710UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 827782784UL;
    static const uint32_t maskC = 3621027840UL;
};

struct Mtdesc19937_6 {
    /* created with creator seed 42 and id 4138 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 2760052778UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 861204096UL;
    static const uint32_t maskC = 3721887744UL;
};

struct Mtdesc19937_7 {
    /* created with creator seed 42 and id 4139 */
    static const int w = 32;
    static const int p = 19937;
    static const uint32_t aaa = 2708803627UL;
    static const int mm = 312;
    static const int nn = 624;
    static const int rr = 31;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 2147483648UL;
    static const uint32_t lmask = 2147483647UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 995441536UL;
    static const uint32_t maskC = 4158029824UL;
};

// 8 instances with p=9941, w=32

struct Mtdesc9941_0 {
    /* created with creator seed 42 and id 4143 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 3040612399UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 1933540096UL;
    static const uint32_t maskC = 4157898752UL;
};

struct Mtdesc9941_1 {
    /* created with creator seed 42 and id 4140 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 2782138412UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 981172096UL;
    static const uint32_t maskC = 4140892160UL;
};

struct Mtdesc9941_2 {
    /* created with creator seed 42 and id 4144 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 2874019888UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 592805760UL;
    static const uint32_t maskC = 4008509440UL;
};

struct Mtdesc9941_3 {
    /* created with creator seed 42 and id 4138 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 3864399914UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 2339864448UL;
    static const uint32_t maskC = 4009656320UL;
};

struct Mtdesc9941_4 {
    /* created with creator seed 42 and id 4139 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 2904821803UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 3668671104UL;
    static const uint32_t maskC = 4022960128UL;
};

struct Mtdesc9941_5 {
    /* created with creator seed 42 and id 4146 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 4196995122UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 1705631104UL;
    static const uint32_t maskC = 3219619840UL;
};

struct Mtdesc9941_6 {
    /* created with creator seed 42 and id 4147 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 3764391987UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 3149782912UL;
    static const uint32_t maskC = 4009066496UL;
};

struct Mtdesc9941_7 {
    /* created with creator seed 42 and id 4141 */
    static const int w = 32;
    static const int p = 9941;
    static const uint32_t aaa = 2208960557UL;
    static const int mm = 155;
    static const int nn = 311;
    static const int rr = 11;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4294965248UL;
    static const uint32_t lmask = 2047UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 1721326464UL;
    static const uint32_t maskC = 4025843712UL;
};

// 8 instances with p=521, w=32

struct Mtdesc521_0 {
    /* created with creator seed 42 and id 4149 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 2177306677UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 1263876864UL;
    static const uint32_t maskC = 4294246400UL;
};

struct Mtdesc521_1 {
    /* created with creator seed 42 and id 4138 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 3929280554UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 653613952UL;
    static const uint32_t maskC = 4288479232UL;
};

struct Mtdesc521_2 {
    /* created with creator seed 42 and id 4140 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 3792113708UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 1999985280UL;
    static const uint32_t maskC = 4292182016UL;
};

struct Mtdesc521_3 {
    /* created with creator seed 42 and id 4146 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 2733314098UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 2800016000UL;
    static const uint32_t maskC = 4292182016UL;
};

struct Mtdesc521_4 {
    /* created with creator seed 42 and id 4148 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 2777092148UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 3875330688UL;
    static const uint32_t maskC = 4292182016UL;
};

struct Mtdesc521_5 {
    /* created with creator seed 42 and id 4142 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 3349024814UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 3061111680UL;
    static const uint32_t maskC = 4288479232UL;
};

struct Mtdesc521_6 {
    /* created with creator seed 42 and id 4139 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 2886799403UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 1991326336UL;
    static const uint32_t maskC = 4294901760UL;
};

struct Mtdesc521_7 {
    /* created with creator seed 42 and id 4141 */
    static const int w = 32;
    static const int p = 521;
    static const uint32_t aaa = 3332837421UL;
    static const int mm = 8;
    static const int nn = 17;
    static const int rr = 23;
    static const int ww = 32;
    static const uint32_t wmask = 4294967295UL;
    static const uint32_t umask = 4286578688UL;
    static const uint32_t lmask = 8388607UL;
    static const int shift0 = 12;
    static const int shift1 = 18;
    static const int shiftB = 7;
    static const int shiftC = 15;
    static const uint32_t maskB = 2006399872UL;
    static const uint32_t maskC = 4284841984UL;
};

namespace mt_detail {

// dc.h

// we have this above
// typedef struct {
//     uint32_t aaa;
//     int mm, nn, rr, ww;
//     uint32_t wmask, umask, lmask;
//     int shift0, shift1, shiftB, shiftC;
//     uint32_t maskB, maskC;
//     int i;
//     uint32_t *state;
// } mt_struct;

/* old interface */

// we disable the old interface, because not used in our wrapper anyway

// void init_dc(uint32_t seed);
// mt_struct *get_mt_parameter(int w, int p);
// mt_struct *get_mt_parameter_id(int w, int p, int id);
// mt_struct **get_mt_parameters(int w, int p, int max_id, int *count);

/* new interface */
mt_struct *get_mt_parameter_st(int w, int p, uint32_t seed);
mt_struct *get_mt_parameter_id_st(int w, int p, int id, uint32_t seed);
mt_struct **get_mt_parameters_st(int w, int p, int start_id, int max_id,
                                 uint32_t seed, int *count);
/* common */
void free_mt_struct(mt_struct *mts);
void free_mt_struct_array(mt_struct **mtss, int count);
void sgenrand_mt(uint32_t seed, mt_struct *mts);
uint32_t genrand_mt(mt_struct *mts);

// mt19337.h

#define dcmt_N 624

typedef struct _ORG_STATE {
    uint32_t mt[dcmt_N];
    int mti;
} _org_state;

void _sgenrand_dc(_org_state *st, uint32_t seed);
uint32_t _genrand_dc(_org_state *st);

// dci.h

#define NOT_REJECTED 1
#define REJECTED 0
#define REDU 0
#define IRRED 1
#define NONREDU 1

extern _org_state global_mt19937;
typedef struct {
    int *x;
    int deg;
} Polynomial;

typedef struct PRESCR_T {
    int sizeofA; /* parameter size */
    uint32_t **modlist;
    Polynomial **preModPolys;
} prescr_t;

typedef struct CHECK32_T {
    uint32_t upper_mask;
    uint32_t lower_mask;
    uint32_t word_mask;
} check32_t;

typedef struct EQDEG_T {
    uint32_t bitmask[32];
    uint32_t mask_b;
    uint32_t mask_c;
    uint32_t upper_v_bits;
    int shift_0;
    int shift_1;
    int shift_s;
    int shift_t;
    int mmm;
    int nnn;
    int rrr;
    int www;
    uint32_t aaa[2];
    uint32_t gupper_mask; /** most significant  (WWW - RRR) bits **/
    uint32_t glower_mask; /** least significant RRR bits **/
    uint32_t greal_mask;  /** upper WWW bitmask **/
    int ggap; /** difference between machine wordsize and dest wordsize **/
    int gcur_maxlengs[32]; /** for optimize_v_hard **/
    uint32_t gmax_b, gmax_c;
} eqdeg_t;

int _prescreening_dc(prescr_t *pre, uint32_t aaa);
void _InitPrescreening_dc(prescr_t *pre, int m, int n, int r, int w);
void _EndPrescreening_dc(prescr_t *pre);
int _CheckPeriod_dc(check32_t *ck, _org_state *st, uint32_t a, int m, int n,
                    int r, int w);
void _get_tempering_parameter_dc(mt_struct *mts);
void _get_tempering_parameter_hard_dc(mt_struct *mts);
void _InitCheck32_dc(check32_t *ck, int r, int w);

} // namesapce mt_detail
} // namespace QuantLib

#endif
