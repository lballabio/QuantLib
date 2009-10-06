/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang
 Copyright (C) 2009 Liquidnet Holdings, Inc.

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

/*! \file fastfouriertransform.hpp
    \brief Fast Fourier Transform
*/

// Based on public domain code by Christopher Diggins

#ifndef quantlib_fast_fourier_transform_hpp
#define quantlib_fast_fourier_transform_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <vector>
#include <iterator>

namespace QuantLib {

    class FastFourierTransform {
      public:
        FastFourierTransform(std::size_t log2_N)
        : cs_(log2_N), sn_(log2_N) {
            std::size_t m = 1 << log2_N;
            cs_[log2_N - 1] = cos (2 * M_PI / m);
            sn_[log2_N - 1] = sin (2 * M_PI / m);
            for (std::size_t i = log2_N - 1; i > 0; --i) {
                cs_ [i - 1] = cs_[i]*cs_[i] - sn_[i]*sn_[i];
                sn_ [i - 1] = 2*sn_[i]*cs_[i];
            }
        }

        template<typename InputIterator, typename RandomAccessIterator>
        void transform(InputIterator inBegin, InputIterator inEnd,
                       RandomAccessIterator out) const {
            typedef
                typename std::iterator_traits<RandomAccessIterator>::value_type
                                                                       complex;
            const std::size_t log2_N = cs_.size();
            const std::size_t N = 1 << log2_N;
            std::size_t i = 0;
            for (; inBegin != inEnd; ++i, ++inBegin) {
                *(out + bit_reverse(i, log2_N)) = *inBegin;
            }
            QL_REQUIRE (i <= N, "insufficient frequency for FFT");
            for (std::size_t s = 1; s <= log2_N; ++s) {
                std::size_t m = 1 << s;
                complex w(1.0);
                complex wm(cs_[s-1], -sn_[s-1]);
                for (std::size_t j = 0; j < m/2; ++j) {
                    for (std::size_t k = j; k < N; k += m) {
                        complex t = w * (*(out + k + m/2));
                        complex u = *(out + k);
                        *(out + k) = u + t;
                        *(out + k + m/2) = u - t;
                    }
                    w *= wm;
                }
            }
        }

        template<typename InputIterator, typename RandomAccessIterator>
        void transform(InputIterator inBegin, InputIterator inEnd,
                       RandomAccessIterator out) {
            transform_impl(inBegin, inEnd, out, false);
        }

        template<typename InputIterator, typename RandomAccessIterator>
        void inverse_transform(InputIterator inBegin, InputIterator inEnd,
                               RandomAccessIterator out) {
            transform_impl(inBegin, inEnd, out, true);
        }

      private:
        std::vector<double> cs_, sn_;

        template<typename InputIterator, typename RandomAccessIterator>
        void transform_impl(InputIterator inBegin, InputIterator inEnd,
                            RandomAccessIterator out,
                            bool inverse) const {
            typedef
                typename std::iterator_traits<RandomAccessIterator>::value_type
                                                                       complex;
            const std::size_t log2_N = cs_.size();
            const std::size_t N = 1 << log2_N;
            std::size_t i = 0;
            for (; inBegin != inEnd; ++i, ++inBegin) {
                *(out + bit_reverse(i, log2_N)) = *inBegin;
            }
            QL_REQUIRE (i <= N, "insufficient frequency for FFT");
            for (std::size_t s = 1; s <= log2_N; ++s) {
                std::size_t m = 1 << s;
                complex w(1.0);
                complex wm(cs_[s-1], inverse ? sn_[s-1] : -sn_[s-1]);
                for (std::size_t j = 0; j < m/2; ++j) {
                    for (std::size_t k = j; k < N; k += m) {
                        complex t = w * (*(out + k + m/2));
                        complex u = *(out + k);
                        *(out + k) = u + t;
                        *(out + k + m/2) = u - t;
                    }
                    w *= wm;
                }
            }
        }

        static std::size_t bit_reverse(std::size_t x, std::size_t log2_N) {
            std::size_t n = 0;
            for (std::size_t i = 0; i < log2_N; ++i) {
                n <<= 1;
                n |= (x & 1);
                x >>= 1;
            }
            return n;
        }
    };

}

#endif
