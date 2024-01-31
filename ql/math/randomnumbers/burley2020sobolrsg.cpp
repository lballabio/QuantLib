/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Peter Caspers

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

#include <ql/math/randomnumbers/burley2020sobolrsg.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Burley2020SobolRsg::Burley2020SobolRsg(Size dimensionality,
                                           unsigned long seed,
                                           SobolRsg::DirectionIntegers directionIntegers,
                                           unsigned long scrambleSeed)
    : dimensionality_(dimensionality), seed_(seed), directionIntegers_(directionIntegers),
      integerSequence_(dimensionality), sequence_(std::vector<Real>(dimensionality), 1.0) {
        reset();
        group4Seeds_.resize((dimensionality_ - 1) / 4 + 1);
        MersenneTwisterUniformRng mt(scrambleSeed);
        for (auto& s : group4Seeds_) {
            s = static_cast<std::uint32_t>(mt.nextInt32());
        }
    }

    void Burley2020SobolRsg::reset() const {
        sobolRsg_ = ext::make_shared<SobolRsg>(dimensionality_, seed_, directionIntegers_, false);
        nextSequenceCounter_ = 0;
    }

    const std::vector<std::uint32_t>& Burley2020SobolRsg::skipTo(std::uint32_t n) const {
        reset();
        for (Size k = 0; k < n + 1; ++k) {
            nextInt32Sequence();
        }
        return integerSequence_;
    }

    namespace {

        // for reverseBits() see http://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable

        const std::uint8_t bitReverseTable[] = {
            0U,   128U, 64U,  192U, 32U,  160U, 96U,  224U, 16U,  144U, 80U,  208U, 48U,  176U,
            112U, 240U, 8U,   136U, 72U,  200U, 40U,  168U, 104U, 232U, 24U,  152U, 88U,  216U,
            56U,  184U, 120U, 248U, 4U,   132U, 68U,  196U, 36U,  164U, 100U, 228U, 20U,  148U,
            84U,  212U, 52U,  180U, 116U, 244U, 12U,  140U, 76U,  204U, 44U,  172U, 108U, 236U,
            28U,  156U, 92U,  220U, 60U,  188U, 124U, 252U, 2U,   130U, 66U,  194U, 34U,  162U,
            98U,  226U, 18U,  146U, 82U,  210U, 50U,  178U, 114U, 242U, 10U,  138U, 74U,  202U,
            42U,  170U, 106U, 234U, 26U,  154U, 90U,  218U, 58U,  186U, 122U, 250U, 6U,   134U,
            70U,  198U, 38U,  166U, 102U, 230U, 22U,  150U, 86U,  214U, 54U,  182U, 118U, 246U,
            14U,  142U, 78U,  206U, 46U,  174U, 110U, 238U, 30U,  158U, 94U,  222U, 62U,  190U,
            126U, 254U, 1U,   129U, 65U,  193U, 33U,  161U, 97U,  225U, 17U,  145U, 81U,  209U,
            49U,  177U, 113U, 241U, 9U,   137U, 73U,  201U, 41U,  169U, 105U, 233U, 25U,  153U,
            89U,  217U, 57U,  185U, 121U, 249U, 5U,   133U, 69U,  197U, 37U,  165U, 101U, 229U,
            21U,  149U, 85U,  213U, 53U,  181U, 117U, 245U, 13U,  141U, 77U,  205U, 45U,  173U,
            109U, 237U, 29U,  157U, 93U,  221U, 61U,  189U, 125U, 253U, 3U,   131U, 67U,  195U,
            35U,  163U, 99U,  227U, 19U,  147U, 83U,  211U, 51U,  179U, 115U, 243U, 11U,  139U,
            75U,  203U, 43U,  171U, 107U, 235U, 27U,  155U, 91U,  219U, 59U,  187U, 123U, 251U,
            7U,   135U, 71U,  199U, 39U,  167U, 103U, 231U, 23U,  151U, 87U,  215U, 55U,  183U,
            119U, 247U, 15U,  143U, 79U,  207U, 47U,  175U, 111U, 239U, 31U,  159U, 95U,  223U,
            63U,  191U, 127U, 255U};

        inline std::uint32_t reverseBits(std::uint32_t x) {
            return (bitReverseTable[x & 0xff] << 24) | (bitReverseTable[(x >> 8) & 0xff] << 16) |
                   (bitReverseTable[(x >> 16) & 0xff] << 8) | (bitReverseTable[(x >> 24) & 0xff]);
        }

        inline std::uint32_t laine_karras_permutation(std::uint32_t x, std::uint32_t seed) {
            x += seed;
            x ^= x * 0x6c50b47cU;
            x ^= x * 0xb82f1e52U;
            x ^= x * 0xc7afe638U;
            x ^= x * 0x8d22f6e6U;
            return x;
        }

        inline std::uint32_t nested_uniform_scramble(std::uint32_t x, std::uint32_t seed) {
            x = reverseBits(x);
            x = laine_karras_permutation(x, seed);
            x = reverseBits(x);
            return x;
        }

        // the results depend a lot on the details of the hash_combine() function that is used
        // we use hash_combine() calling hash(), hash_mix() as implemented here:
        // https://github.com/boostorg/container_hash/blob/boost-1.83.0/include/boost/container_hash/hash.hpp#L560
        // https://github.com/boostorg/container_hash/blob/boost-1.83.0/include/boost/container_hash/hash.hpp#L115
        // https://github.com/boostorg/container_hash/blob/boost-1.83.0/include/boost/container_hash/detail/hash_mix.hpp#L67

        inline std::uint64_t local_hash_mix(std::uint64_t x) {
            const std::uint64_t m = 0xe9846af9b1a615d;
            x ^= x >> 32;
            x *= m;
            x ^= x >> 32;
            x *= m;
            x ^= x >> 28;
            return x;
        }

        inline std::uint64_t local_hash(const std::uint64_t v) {
            std::uint64_t seed = 0;
            seed = (v >> 32) + local_hash_mix(seed);
            seed = (v & 0xFFFFFFFF) + local_hash_mix(seed);
            return seed;
        }

        inline std::uint64_t local_hash_combine(std::uint64_t x, const uint64_t v) {
            return local_hash_mix(x + 0x9e3779b9 + local_hash(v));
        }
    }

    const std::vector<std::uint32_t>& Burley2020SobolRsg::nextInt32Sequence() const {
        auto n = nested_uniform_scramble(nextSequenceCounter_, group4Seeds_[0]);
        const auto& seq = sobolRsg_->skipTo(n);
        std::copy(seq.begin(), seq.end(), integerSequence_.begin());
        Size i = 0, group = 0;
        do {
            std::uint64_t seed = group4Seeds_[group++];
            for (Size g = 0; g < 4 && i < dimensionality_; ++g, ++i) {
                seed = local_hash_combine(seed, g);
                integerSequence_[i] =
                    nested_uniform_scramble(integerSequence_[i], static_cast<std::uint32_t>(seed));
            }
        } while (i < dimensionality_);
        QL_REQUIRE(++nextSequenceCounter_ != 0,
                   "Burley2020SobolRsg::nextIn32Sequence(): period exceeded");
        return integerSequence_;
    }

    const SobolRsg::sample_type& Burley2020SobolRsg::nextSequence() const {
        const std::vector<std::uint32_t>& v = nextInt32Sequence();
        // normalize to get a double in (0,1)
        for (Size k = 0; k < dimensionality_; ++k) {
            sequence_.value[k] = static_cast<double>(v[k]) / 4294967296.0;
        }
        return sequence_;
    }
}
