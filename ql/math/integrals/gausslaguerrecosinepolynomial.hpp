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

/*! \file gausslaguerrecosinepolynomial.hpp
    \brief Laguerre-Cosine Gaussian quadrature
*/

#ifndef quantlib_gauss_laguerre_cosine_polynomial_hpp
#define quantlib_gauss_laguerre_cosine_polynomial_hpp

#include <ql/math/functional.hpp>
#include <ql/math/integrals/momentbasedgaussianpolynomial.hpp>
#include <cmath>

namespace QuantLib {

	template <class mp_real>
	class GaussLaguerreTrigonometricBase
			: public MomentBasedGaussianPolynomial<mp_real> {
	  public:
		explicit GaussLaguerreTrigonometricBase(Real u)
		: u_(u) { }

	  protected:
		virtual mp_real m0() const = 0;
		virtual mp_real m1() const = 0;

		mp_real moment_(Size n) const { //NOLINT(bugprone-virtual-near-miss)
			if (m_.size() <= n)
				m_.resize(n+1, std::numeric_limits<mp_real>::quiet_NaN());

			if (std::isnan(m_[n])) {
				if (n == 0)
					m_[0] = m0();
				else if (n == 1)
					m_[1] = m1();
				else
					m_[n] = (2*n*moment_(n-1)
						- n*(n-1)*moment_(n-2))/(1+u_*u_);
			}

			return m_[n];
		}
		mp_real fact(Size n) const {
			if (f_.size() <= n)
				f_.resize(n+1, std::numeric_limits<mp_real>::quiet_NaN());

			if (std::isnan(f_[n])) {
				if (n == 0)
					f_[0] = 1.0;
				else
					f_[n] = n*fact(n-1);
			}
			return f_[n];

		}
		const Real u_;

	  private:
		mutable std::vector<mp_real> m_, f_;
	};

    //! Gauss-Laguerre Cosine integration

    /*! This class performs a 1-dimensional Gauss-Laguerre-Cosine integration.
        \f[
        \int_{0}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;u)=e^{-x}*\cos{u*x}
        \f]
    */
    template <class mp_real>
    class GaussLaguerreCosinePolynomial
            : public GaussLaguerreTrigonometricBase<mp_real> {
      public:
        explicit GaussLaguerreCosinePolynomial(Real u)
        : GaussLaguerreTrigonometricBase<mp_real>(u),
          m0_(1.0+1.0/(1.0+u*u)) { }

        mp_real moment(Size n) const override { return (this->moment_(n) + this->fact(n)) / m0_; }
        Real w(Real x) const override { return std::exp(-x) * (1 + std::cos(this->u_ * x)) / m0_; }

      protected:
        mp_real m0() const override { return 1 / (1 + this->u_ * this->u_); }
        mp_real m1() const override {
            return (1 - this->u_*this->u_)
                    /square<mp_real>()(1 + this->u_*this->u_);
        }

      private:
        const Real m0_;
    };

    //! Gauss-Laguerre Sine integration

    /*! This class performs a 1-dimensional Gauss-Laguerre-Cosine integration.
        \f[
        \int_{0}^{\inf} f(x) \mathrm{d}x
        \f]
        The weighting function is
        \f[
            w(x;u)=e^{-x}*\sin{u*x}
        \f]
    */
    template <class mp_real>
    class GaussLaguerreSinePolynomial
            : public GaussLaguerreTrigonometricBase<mp_real> {
      public:
        explicit GaussLaguerreSinePolynomial(Real u)
        : GaussLaguerreTrigonometricBase<mp_real>(u),
          m0_(1.0+u/(1.0+u*u)) { }

        mp_real moment(Size n) const override { return (this->moment_(n) + this->fact(n)) / m0_; }
        Real w(Real x) const override { return std::exp(-x) * (1 + std::sin(this->u_ * x)) / m0_; }

      protected:
        mp_real m0() const override { return this->u_ / (1 + this->u_ * this->u_); }
        mp_real m1() const override {
            return 2*this->u_/square<mp_real>()(1 + this->u_*this->u_);
        }

      private:
        const Real m0_;
    };
}

#endif


#ifndef id_a6307109d2963ebe4cc87261af27300f
#define id_a6307109d2963ebe4cc87261af27300f
inline bool test_a6307109d2963ebe4cc87261af27300f(int* i) { return i != 0; }
#endif
