/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2022 Chester Wong
 Copyright (C) 2022 Wojciech Czernous

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

/*! \file fractionalnoisersg.hpp
    \brief Fractional Gaussian noise (sequence) generator
*/

#ifndef quantlib_fractional_gaussian_noise_rsg_hpp
#define quantlib_fractional_gaussian_noise_rsg_hpp

#include <ql/math/fastfouriertransform.hpp>
#include <ql/math/matrix.hpp>
#include <ql/methods/montecarlo/sample.hpp>
#include <complex>

using std::pow;

namespace QuantLib {

    //! Fractional Gaussian noise (sequence) generator
    /*! Using the method by Davies & Harte, improved by Chan & Wood, of finding
        the square root of the autocovariance matrix by means of embedding 
        it in a circulant matrix.
        -# Andrew T. A. Wood, & Chan, G. (1994). Simulation of Stationary 
        Gaussian Processes in [0,1]^d. Journal of Computational and Graphical
        Statistics, 3(4), 409–432. https://doi.org/10.2307/1390903

        \test the correctness of the returned values is tested by 
              a likelihood-ratio test (LRT) of covariance matrix 
    */
    template <class GSG>
    class FractionalGaussianNoiseRsg {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        FractionalGaussianNoiseRsg(Real HurstParameter,
                                   GSG generator);
        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const;
        Size dimension() const;
        Real gamma(Size k) const { return k==0 ? 1.0 : 0.5 * (pow(k-1,2*H_)-2*pow(k,2*H_)+pow(k+1,2*H_)); };
      private:
        Real H_;
        GSG generator_;
        Size n_;
        Size m_;
        Size log2_m_;
        FastFourierTransform fft_;
        /// @brief Square roots of eigenvalues of circulant matrix
        std::vector<Real> sqrt_lambda_;
        mutable std::vector<std::complex<Real> > fft_in_;
        mutable std::vector<std::complex<Real> > fft_out_;
        mutable sample_type next_;
        /// @brief Floor of log_2(n).
        Size log_2(Size n) const { Size l=0; while(n>1) {n/=2;l++;} return l;}
    };

    // template definitions

    template <class GSG>
    FractionalGaussianNoiseRsg<GSG>::FractionalGaussianNoiseRsg(Real HurstParameter, GSG generator)
    : H_(HurstParameter),
      generator_(std::move(generator)), 
      n_(generator_.dimension()),
      m_(2*n_),
      log2_m_(log_2(m_)),
      fft_(log2_m_),
      sqrt_lambda_(m_), 
      fft_in_(m_), fft_out_(m_), 
      next_(std::vector<Real> (n_), 1.0)
    {
        Size bin_floor_m_ = static_cast<std::size_t>(1) << log2_m_;
        QL_REQUIRE(bin_floor_m_ == m_,
                   "sequence generator dimensionality (" << n_
                   << ") is not a power of 2");

        std::vector<std::complex<Real> > c(m_);
        std::vector<std::complex<Real> > lambda(m_);
        
        for (Size i = 0; i <= n_; ++i)
            c[i] = gamma(i);
        for (Size i = n_ + 1; i < m_; ++i)
            c[i] = gamma(m_ - i);
        fft_.transform(c.begin(), c.end(), lambda.begin());
        for (Size i = 0; i < m_; ++i)
        {
            QL_REQUIRE(lambda[i].real() >= 0.0, "all eigenvalues must be non-negative");
            sqrt_lambda_[i] = std::sqrt(lambda[i].real());
        }
    }

    template <class GSG>
    const typename FractionalGaussianNoiseRsg<GSG>::sample_type& 
    FractionalGaussianNoiseRsg<GSG>::nextSequence() const {
        typedef typename GSG::sample_type sequence_type;
        std::complex<Real> i1(0, 1);
        const sequence_type& S = generator_.nextSequence();
        const sequence_type& T = generator_.nextSequence();
        fft_in_[0]  = S.value[0];
        fft_in_[n_] = T.value[0];
        for (Size j = 1; j < n_; ++j)
        {
             fft_in_[j]      = std::sqrt(0.5) * (S.value[j] + i1 * T.value[j]);
             fft_in_[m_ - j] = std::sqrt(0.5) * (S.value[j] - i1 * T.value[j]);
        }
        for (Size j = 0; j < m_; ++j)
            fft_in_[j] *= sqrt_lambda_[j];

        fft_.transform(fft_in_.begin(), fft_in_.end(), fft_out_.begin());

        for (Size j = 0; j < n_; ++j)
            next_.value[j] = fft_out_[j].real() / std::sqrt(m_);

        return next_;
    }

    template <class GSG>
    const typename FractionalGaussianNoiseRsg<GSG>::sample_type& 
    FractionalGaussianNoiseRsg<GSG>::lastSequence() const {
        return next_;
    }

    template <class GSG>
    Size 
    FractionalGaussianNoiseRsg<GSG>::dimension() const {
        return n_;
    }

}

#endif


