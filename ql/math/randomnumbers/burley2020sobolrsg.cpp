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

        static const std::uint8_t bitReverseTable[] = {
            0u,   128u, 64u,  192u, 32u,  160u, 96u,  224u, 16u,  144u, 80u,  208u, 48u,  176u,
            112u, 240u, 8u,   136u, 72u,  200u, 40u,  168u, 104u, 232u, 24u,  152u, 88u,  216u,
            56u,  184u, 120u, 248u, 4u,   132u, 68u,  196u, 36u,  164u, 100u, 228u, 20u,  148u,
            84u,  212u, 52u,  180u, 116u, 244u, 12u,  140u, 76u,  204u, 44u,  172u, 108u, 236u,
            28u,  156u, 92u,  220u, 60u,  188u, 124u, 252u, 2u,   130u, 66u,  194u, 34u,  162u,
            98u,  226u, 18u,  146u, 82u,  210u, 50u,  178u, 114u, 242u, 10u,  138u, 74u,  202u,
            42u,  170u, 106u, 234u, 26u,  154u, 90u,  218u, 58u,  186u, 122u, 250u, 6u,   134u,
            70u,  198u, 38u,  166u, 102u, 230u, 22u,  150u, 86u,  214u, 54u,  182u, 118u, 246u,
            14u,  142u, 78u,  206u, 46u,  174u, 110u, 238u, 30u,  158u, 94u,  222u, 62u,  190u,
            126u, 254u, 1u,   129u, 65u,  193u, 33u,  161u, 97u,  225u, 17u,  145u, 81u,  209u,
            49u,  177u, 113u, 241u, 9u,   137u, 73u,  201u, 41u,  169u, 105u, 233u, 25u,  153u,
            89u,  217u, 57u,  185u, 121u, 249u, 5u,   133u, 69u,  197u, 37u,  165u, 101u, 229u,
            21u,  149u, 85u,  213u, 53u,  181u, 117u, 245u, 13u,  141u, 77u,  205u, 45u,  173u,
            109u, 237u, 29u,  157u, 93u,  221u, 61u,  189u, 125u, 253u, 3u,   131u, 67u,  195u,
            35u,  163u, 99u,  227u, 19u,  147u, 83u,  211u, 51u,  179u, 115u, 243u, 11u,  139u,
            75u,  203u, 43u,  171u, 107u, 235u, 27u,  155u, 91u,  219u, 59u,  187u, 123u, 251u,
            7u,   135u, 71u,  199u, 39u,  167u, 103u, 231u, 23u,  151u, 87u,  215u, 55u,  183u,
            119u, 247u, 15u,  143u, 79u,  207u, 47u,  175u, 111u, 239u, 31u,  159u, 95u,  223u,
            63u,  191u, 127u, 255u};

        std::uint32_t reverseBits(std::uint32_t x) {
            return (bitReverseTable[x & 0xff] << 24) | (bitReverseTable[(x >> 8) & 0xff] << 16) |
                   (bitReverseTable[(x >> 16) & 0xff] << 8) | (bitReverseTable[(x >> 24) & 0xff]);
        }

        std::uint32_t laine_karras_permutation(std::uint32_t x, std::uint32_t seed) {
            x += seed;
            x ^= x * 0x6c50b47cu;
            x ^= x * 0xb82f1e52u;
            x ^= x * 0xc7afe638u;
            x ^= x * 0x8d22f6e6u;
            return x;
        }

        std::uint32_t nested_uniform_scramble(std::uint32_t x, std::uint32_t seed) {
            x = reverseBits(x);
            x = laine_karras_permutation(x, seed);
            x = reverseBits(x);
            return x;
        }

        // the results depend a lot on the details of the hash_combine() function that is used
        // we use the 64bit version of hash_combine() as it is implemented here:
        // https://github.com/boostorg/container_hash/blob/boost-1.83.0/include/boost/container_hash/hash.hpp#L560
        // https://github.com/boostorg/container_hash/blob/boost-1.83.0/include/boost/container_hash/detail/hash_mix.hpp#L67

        void local_hash_combine(std::uint64_t& x, const uint64_t v) {
            const std::uint64_t m = 0xe9846af9b1a615d;
            x += 0x9e3779b9 + std::hash<std::uint64_t>()(v);
            x ^= x >> 32;
            x *= m;
            x ^= x >> 32;
            x *= m;
            x ^= x >> 28;
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
                local_hash_combine(seed, g);
                integerSequence_[i] =
                    nested_uniform_scramble(integerSequence_[i], static_cast<std::uint32_t>(seed));
            }
        } while (i < dimensionality_);
        ++nextSequenceCounter_;
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
