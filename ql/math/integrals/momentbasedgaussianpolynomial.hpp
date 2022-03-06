/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Klaus Spanderen

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

/*! \file momentbasedgaussianpolynomial.hpp
    \brief Gaussian quadrature defined by the moments of the distribution
*/

#ifndef quantlib_moment_based_gaussian_polynomial_hpp
#define quantlib_moment_based_gaussian_polynomial_hpp

#include <ql/math/comparison.hpp>
#include <ql/math/integrals/gaussianorthogonalpolynomial.hpp>
#include <ql/errors.hpp>
#include <cmath>
#include <vector>

namespace QuantLib {
    /*! References:
        Gauss quadratures and orthogonal polynomials

        G.H. Gloub and J.H. Welsch: Calculation of Gauss quadrature rule.
        Math. Comput. 23 (1986), 221-230,
        http://web.stanford.edu/class/cme335/spr11/S0025-5718-69-99647-1.pdf

        M. Morandi Cecchi and M. Redivo Zaglia, Computing the coefficients
        of a recurrence formula for numerical integration by moments and
        modified moments.
        http://ac.els-cdn.com/0377042793901522/1-s2.0-0377042793901522-main.pdf?_tid=643d5dca-a05d-11e6-9a56-00000aab0f27&acdnat=1478023545_cf7c87cba4cc9e37a136e68a2564d411
    */

    template <class mp_real>
    class MomentBasedGaussianPolynomial
            : public GaussianOrthogonalPolynomial {
      public:
        MomentBasedGaussianPolynomial();

        Real mu_0() const override;
        Real alpha(Size i) const override;
        Real beta(Size i) const override;

        virtual mp_real moment(Size i) const = 0;

      private:
        mp_real alpha_(Size i) const;
        mp_real beta_(Size i) const;

        mp_real z(Integer k, Integer i) const;

        mutable std::vector<mp_real> b_, c_;
        mutable std::vector<std::vector<mp_real> > z_;
    };

    template <class mp_real> inline
    MomentBasedGaussianPolynomial<mp_real>::MomentBasedGaussianPolynomial()
    : z_(1, std::vector<mp_real>()) {}

    template <class mp_real> inline
    mp_real MomentBasedGaussianPolynomial<mp_real>::z(Integer k, Integer i) const {
        if (k == -1) return mp_real(0.0);

        const Integer rows = z_.size();
        const Integer cols = z_[0].size();

        if (cols <= i) {
            for (Integer l=0; l<rows; ++l)
                z_[l].resize(i+1, std::numeric_limits<mp_real>::quiet_NaN());
        }
        if (rows <= k) {
            z_.resize(k+1, std::vector<mp_real>(
                z_[0].size(), std::numeric_limits<mp_real>::quiet_NaN()));
        }

        if (std::isnan(z_[k][i])) {
            if (k == 0)
                z_[k][i] = moment(i);
            else {
                const mp_real tmp = z(k-1, i+1)
                    - alpha_(k-1)*z(k-1, i) - beta_(k-1)*z(k-2, i);
                z_[k][i] = tmp;
            }
        }

        return z_[k][i];
    };

    template <class mp_real> inline
    mp_real MomentBasedGaussianPolynomial<mp_real>::alpha_(Size u) const {

        if (b_.size() <= u)
            b_.resize(u+1, std::numeric_limits<mp_real>::quiet_NaN());

        if (std::isnan(b_[u])) {
            if (u == 0)
                b_[u] = moment(1);
            else {
                const Integer iu(u);
                const mp_real tmp =
                    -z(iu-1, iu)/z(iu-1, iu-1) + z(iu, iu+1)/z(iu, iu);
                b_[u] = tmp;
            }
        }
        return b_[u];
    }

    template <class mp_real> inline
    mp_real MomentBasedGaussianPolynomial<mp_real>::beta_(Size u) const {
        if (u == 0)
            return mp_real(1.0);

        if (c_.size() <= u)
            c_.resize(u+1, std::numeric_limits<mp_real>::quiet_NaN());

        if (std::isnan(c_[u])) {
            const Integer iu(u);
            const mp_real tmp = z(iu, iu) / z(iu-1, iu-1);
            c_[u] = tmp;
        }
        return c_[u];
    }

    template <> inline
    Real MomentBasedGaussianPolynomial<Real>::alpha(Size u) const {
        return alpha_(u);
    }

    template <class mp_real> inline
    Real MomentBasedGaussianPolynomial<mp_real>::alpha(Size u) const {
        return alpha_(u).template convert_to<Real>();
    }

    template <> inline
    Real MomentBasedGaussianPolynomial<Real>::beta(Size u) const {
        return beta_(u);
    }

    template <class mp_real> inline
    Real MomentBasedGaussianPolynomial<mp_real>::beta(Size u) const {
        mp_real b = beta_(u);
        return b.template convert_to<Real>();
    }

    template <> inline
    Real MomentBasedGaussianPolynomial<Real>::mu_0() const {
        const Real m0 = moment(0);
        QL_REQUIRE(close_enough(m0, 1.0), "zero moment must by one.");

        return moment(0);
    }

    template <class mp_real> inline
    Real MomentBasedGaussianPolynomial<mp_real>::mu_0() const {
        return moment(0).template convert_to<Real>();
    }
}

#endif


#ifndef id_1155394c0e82daa75303be4b89d89973
#define id_1155394c0e82daa75303be4b89d89973
inline bool test_1155394c0e82daa75303be4b89d89973(const int* i) {
    return i != nullptr;
}
#endif
