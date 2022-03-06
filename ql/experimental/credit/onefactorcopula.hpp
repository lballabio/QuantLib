/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file onefactorcopula.hpp
    \brief One-factor copula base class
*/

#ifndef quantlib_one_factor_copula_hpp
#define quantlib_one_factor_copula_hpp

#include <ql/experimental/credit/distribution.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/quote.hpp>
#include <utility>

namespace QuantLib {

    //! Abstract base class for one-factor copula models
    /*! Reference: John Hull and Alan White, The Perfect Copula, June 2006

        Let \f$Q_i(t)\f$ be the cumulative probability of default of
        counterparty i before time t.

        In a one-factor model, consider random variables
        \f[ Y_i = a_i\,M+\sqrt{1-a_i^2}\:Z_i \f]
        where \f$M\f$ and \f$Z_i\f$ have independent zero-mean
        unit-variance distributions and \f$-1\leq a_i \leq 1\f$.  The
        correlation between \f$Y_i\f$ and \f$Y_j\f$ is then
        \f$a_i a_j\f$.

        Let \f$F_Y(y)\f$ be the cumulative distribution function of \f$Y_i\f$.
        \f$y\f$ is mapped to \f$t\f$ such that percentiles match, i.e.
        \f$F_Y(y)=Q_i(t)\f$ or \f$y=F_Y^{-1}(Q_i(t))\f$.

        Now let \f$F_Z(z)\f$ be the cumulated distribution function of
        \f$Z_i\f$.  For given realization of \f$M\f$, this determines
        the distribution of \f$y\f$:
        \f[
        Prob \,(Y_i < y|M) = F_Z \left( \frac{y-a_i\,M}{\sqrt{1-a_i^2}}\right)
        \qquad
        \mbox{or}
        \qquad
        Prob \,(t_i < t|M) = F_Z \left( \frac{F_Y^{-1}(Q_i(t))-a_i\,M}
        {\sqrt{1-a_i^2}}
        \right)
        \f]

        The distribution functions of \f$ M, Z_i \f$ are specified in
        derived classes. The distribution function of \f$ Y \f$ is
        then given by the convolution
        \f[
        F_Y(y) = Prob\,(Y<y) = \int_{-\infty}^\infty\,\int_{-\infty}^{\infty}\:
        D_Z(z)\,D_M(m) \quad
        \Theta \left(y - a\,m - \sqrt{1-a^2}\,z\right)\,dm\,dz,
        \qquad
        \Theta (x) = \left\{
        \begin{array}{ll}
        1 & x \geq 0 \\
        0 & x < 0
        \end{array}\right.
        \f]
        where \f$ D_Z(z) \f$ and \f$ D_M(m) \f$ are the probability
        densities of \f$ Z\f$ and \f$ M, \f$ respectively.

        This convolution can also be written
        \f[
        F(y) = Prob \,(Y < y) =
        \int_{-\infty}^\infty D_M(m)\,dm\:
        \int_{-\infty}^{g(y,a,m)} D_Z(z)\,dz, \qquad
        g(y,a,m) = \frac{y - a\cdot m}{\sqrt{1-a^2}}, \qquad a < 1
        \f]

        or

        \f[
        F(y) = Prob \,(Y < y) =
        \int_{-\infty}^\infty D_Z(z)\,dz\:
        \int_{-\infty}^{h(y,a,z)} D_M(m)\,dm, \qquad
        h(y,a,z) = \frac{y - \sqrt{1 - a^2}\cdot z}{a}, \qquad a > 0.
        \f]

        In general, \f$ F_Y(y) \f$ needs to be computed numerically.

        \todo Improve on simple Euler integration
    */
    class OneFactorCopula : public LazyObject {
      public:
        OneFactorCopula(Handle<Quote> correlation,
                        Real maximum = 5.0,
                        Size integrationSteps = 50,
                        Real minimum = -5.0)
        : correlation_(std::move(correlation)), max_(maximum), steps_(integrationSteps),
          min_(minimum) {
            QL_REQUIRE(correlation_->value() >= -1
                       && correlation_->value() <= 1,
                       "correlation out of range [-1, +1]");
            registerWith(correlation_);
        }

        //! Density function of M.
        /*! Derived classes must override this method and ensure zero
            mean and unit variance.
        */
        virtual Real density(Real m) const = 0;
        //! Cumulative distribution of Z.
        /*! Derived classes must override this method and ensure zero
            mean and unit variance.
        */
        virtual Real cumulativeZ(Real z) const = 0;
        //! Cumulative distribution of Y.
        /*! This is the default implementation based on tabulated
            data. The table needs to be filled by derived classes. If
            analytic calculation is feasible, this method can also be
            overridden.
        */
        virtual Real cumulativeY(Real y) const;
        //! Inverse cumulative distribution of Y.
        /*! This is the default implementation based on tabulated
            data. The table needs to be filled by derived classes. If
            analytic calculation is feasible, this method can also be
            overridden.
        */
        virtual Real inverseCumulativeY(Real p) const;

        //! Single correlation parameter
        Real correlation() const;

        //! Conditional probability
        /*! \f[
            \hat p(m) = F_Z \left( \frac{F_Y^{-1}(p)-a\,m}{\sqrt{1-a^2}}\right)
            \f]
        */
        Real conditionalProbability(Real prob,
                                    Real m) const;

        //! Vector of conditional probabilities
        /*! \f[
            \hat p_i(m) = F_Z \left( \frac{F_Y^{-1}(p_i)-a\,m}{\sqrt{1-a^2}}
            \right)
            \f]
        */
        std::vector<Real> conditionalProbability(const std::vector<Real>& prob,
                                                 Real m) const;

        /*! Integral over the density \f$ \rho(m) \f$ of M and the conditional
            probability related to p:

            \f[
            \int_{-\infty}^\infty\,dm\,\rho(m)\,
            F_Z \left( \frac{F_Y^{-1}(p)-a\,m}{\sqrt{1-a^2}}\right)
            \f]
        */
        Real integral(Real p) const {
            QL_REQUIRE(p >= 0 && p <= 1, "probability p=" << p
                       << " out of range [0,1]");
            calculate();

            Real avg = 0;
            for (Size k = 0; k < steps(); k++) {
                Real pp = conditionalProbability(p, m(k));
                avg += pp * densitydm(k);
            }
            return avg;
        }

        /*! Integral over the density \f$ \rho(m) \f$ of M and a
            one-dimensional function \f$ f \f$ of conditional
            probabilities related to the input vector of probabilities p:

            \f[
            \int_{-\infty}^\infty\,dm\,\rho(m)\, f (\hat p_1, \hat p_2, \dots,
            \hat p_N), \qquad
            \hat p_i (m) = F_Z \left( \frac{F_Y^{-1}(p_i)-a\,m}{\sqrt{1-a^2}}
            \right)
            \f]
        */
        template <class F>
        Real integral(const F& f, std::vector<Real>& probabilities) const {
            calculate();

            Real avg = 0.0;
            for (Size i = 0; i < steps_; i++) {
                std::vector<Real> conditional
                    = conditionalProbability(probabilities, m(i));
                Real prob = f(conditional);
                avg += prob * densitydm(i);
            }
            return avg;
        }

        /*! Integral over the density \f$ \rho(m) \f$ of M and a
            multi-dimensional function \f$ f \f$ of conditional
            probabilities related to the input vector of probabilities p:

            \f[
            \int_{-\infty}^\infty\,dm\,\rho(m)\, f (\hat p_1, \hat p_2, \dots,
            \hat p_N), \qquad
            \hat p_i = F_Z \left( \frac{F_Y^{-1}(p_i)-a\,m}{\sqrt{1-a^2}}\right)
            \f]
        */
        template <class F>
        Distribution integral(const F& f,
                              const std::vector<Real>& nominals,
                              const std::vector<Real>& probabilities) const {
            calculate();

            Distribution dist(f.buckets(), 0.0, f.maximum());
            for (Size i = 0; i < steps(); i++) {
                std::vector<Real> conditional
                    = conditionalProbability(probabilities, m(i));
                Distribution d = f(nominals, conditional);
                for (Size j = 0; j < dist.size(); j++)
                    dist.addDensity(j, d.density(j) * densitydm(i));
            }
            return dist;
        }

        /*! Check moments (unit norm, zero mean and unit variance) of
            the distributions of M, Z, and Y by numerically
            integrating the respective density.  Parameter tolerance
            is the maximum tolerable absolute error.
        */
        int checkMoments(Real tolerance) const;

      protected:
        Handle<Quote> correlation_;
        mutable Real max_;
        mutable Size steps_;
        mutable Real min_;

        // Tabulated numerical solution of the cumulated distribution of Y
        mutable std::vector<Real> y_;
        mutable std::vector<Real> cumulativeY_;

        //private:
        // utilities for simple Euler integrations over the density of M
        Size steps() const;

        // i not used yet, might allow varying grid size
        // for the copula integration in the future
        Real dm(Size i) const;

        Real m(Size i) const;
        Real densitydm(Size i) const;
    };

    inline Real OneFactorCopula::correlation() const {
        calculate();
        return correlation_->value();
    }

    inline Size OneFactorCopula::steps() const {
        return steps_;
    }

    inline Real OneFactorCopula::dm(Size) const {
        return (max_ - min_)/ steps_;
    }

    inline Real OneFactorCopula::m(Size i) const {
        QL_REQUIRE(i < steps_, "index out of range");
        return min_ + dm(i) * i + dm(i) / 2;
    }

    inline Real OneFactorCopula::densitydm(Size i) const {
        QL_REQUIRE(i < steps_, "index out of range");
        return density(m(i)) * dm(i);
    }

}

#endif


#ifndef id_1559d95a8e3e13de582615038e007afb
#define id_1559d95a8e3e13de582615038e007afb
inline bool test_1559d95a8e3e13de582615038e007afb(const int* i) {
    return i != nullptr;
}
#endif
