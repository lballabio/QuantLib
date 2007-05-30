/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

// Based on public domain code by  Christopher Diggins 

#ifdef quantlib_fast_fourier_transform_h
#define quantlib_fast_fourier_transform_h

#include <ql/types.hpp>

namespace QuantLib {
    class FastFourierTransform {
    public:
    
        template<int Log2_N, class Iter_T>
        void fft(Iter_T a, Iter_T A)
        {
            typedef std::iterator_traits<Iter_T>::value_type complex;
            const int N = 1 << Log2_N;
            for (unsigned int i=0; i<N; ++i) {
                A[bit_reverse<Log2_N>(i)] = a[i];
            }
            for (int s = 1; s <= Log2_N; ++s) {
                int m = 1 << s;
                complex w(1, 0);
                complex wm(cos(2 * M_PI / m), sin(2 * M_PI / m));
                for (int j=0; j < m/2; ++j) {
                    for (int k=j; k < N; k += m) {
                        complex t = w * A[k + m/2];
                        complex u = A[k];
                        A[k] = u + t;
                        A[k + m/2] = u - t;
                    }
                    w *= wm;
                }
            }
        }

    private:
        template<int N>
        unsigned int bit_reverse(unsigned int x) {
            int n = 0;
            int mask = 0x1;
            for (int i=0; i < N; i++) {
                n <<= 1;
                n |= (x & 1);
                x >>= 1;
            }
            return n;
        }
    };
}
#endif
