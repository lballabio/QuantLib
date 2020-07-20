/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file kahalesmilesection.hpp
    \brief Arbitrage free smile section using a C^1 inter- and extrapolation
   method proposed by Kahale, see
   http://www.risk.net/data/Pay_per_view/risk/technical/2004/0504_tech_option2.pdf
   Exponential extrapolation for high strikes can be used alternatively to avoid
   a too slowly decreasing call price function. Note that in the leftmost
   interval and right from the last grid point the input smile is always
   replaced by the extrapolating functional forms, so if you are sure that the
   input smile is globally arbitrage free and you do not want to change it in
   these strike regions you should not use this class at all.
   Input smile sections with a shift are handled accordingly, normal input
   smile section are not possible though.
*/

#ifndef quantlib_kahale_smile_section_hpp
#define quantlib_kahale_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/termstructures/volatility/smilesectionutils.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/math/distributions/normal.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif
#include <vector>
#include <utility>

// numerical constants, still experimental
#define QL_KAHALE_FMAX QL_MAX_REAL
#define QL_KAHALE_SMAX 5.0
#define QL_KAHALE_ACC 1E-12
#define QL_KAHALE_EPS QL_EPSILON

namespace QuantLib {

    class KahaleSmileSection : public SmileSection {

      public:
        struct cFunction {
            // this is just a helper class where we do not want virtual
            // functions
            cFunction(Real f, Real s, Real a, Real b)
                : f_(f), s_(s), a_(a), b_(b), exponential_(false) {}
            cFunction(Real a, Real b) : a_(a), b_(b), exponential_(true) {}
            Real operator()(Real k) const {
                if (exponential_)
                    return std::exp(-a_ * k + b_);
                if (s_ < QL_EPSILON)
                    return std::max(f_ - k, 0.0) + a_ * k + b_;
                boost::math::normal normal;
                Real d1 = std::log(f_ / k) / s_ + s_ / 2.0;
                Real d2 = d1 - s_;
                return f_ * boost::math::cdf(normal, d1) -
                       k * boost::math::cdf(normal, d2) + a_ * k + b_;
            }
            Real f_, s_, a_, b_;
            const bool exponential_;
        };

        struct aHelper {
            aHelper(Real k0, Real k1, Real c0, Real c1, Real c0p, Real c1p)
                : k0_(k0), k1_(k1), c0_(c0), c1_(c1), c0p_(c0p), c1p_(c1p) {}
            Real operator()(Real a) const {
                boost::math::normal normal;
                Real d20 = boost::math::quantile(normal, -c0p_ + a);
                Real d21 = boost::math::quantile(normal, -c1p_ + a);
                Real alpha = (d20 - d21) / (std::log(k0_) - std::log(k1_));
                Real beta = d20 - alpha * std::log(k0_);
                s_ = -1.0 / alpha;
                f_ = std::exp(s_ * (beta + s_ / 2.0));
                QL_REQUIRE(f_ < QL_KAHALE_FMAX, "dummy"); // this is caught
                cFunction cTmp(f_, s_, a, 0.0);
                b_ = c0_ - cTmp(k0_);
                cFunction c(f_, s_, a, b_);
                return c(k1_) - c1_;
            }
            Real k0_, k1_, c0_, c1_, c0p_, c1p_;
            mutable Real s_, f_, b_;
        };

        struct sHelper {
            sHelper(Real k0, Real c0, Real c0p) : k0_(k0), c0_(c0), c0p_(c0p) {}
            Real operator()(Real s) const {
                s = std::max(s, 0.0);
                boost::math::normal normal;
                Real d20 = boost::math::quantile(normal, -c0p_);
                f_ = k0_ * std::exp(s * d20 + s * s / 2.0);
                QL_REQUIRE(f_ < QL_KAHALE_FMAX, "dummy"); // this is caught
                cFunction c(f_, s, 0.0, 0.0);
                return c(k0_) - c0_;
            }
            Real k0_, c0_, c0p_;
            mutable Real f_;
        };

        struct sHelper1 {
            sHelper1(Real k1, Real c0, Real c1, Real c1p)
                : k1_(k1), c0_(c0), c1_(c1), c1p_(c1p) {}
            Real operator()(Real s) const {
                s = std::max(s, 0.0);
                boost::math::normal normal;
                Real d21 = boost::math::quantile(normal, -c1p_);
                f_ = k1_ * std::exp(s * d21 + s * s / 2.0);
                QL_REQUIRE(f_ < QL_KAHALE_FMAX, "dummy"); // this is caught
                b_ = c0_ - f_;
                cFunction c(f_, s, 0.0, b_);
                return c(k1_) - c1_;
            }
            Real k1_, c0_, c1_, c1p_;
            mutable Real f_, b_;
        };

        KahaleSmileSection(const ext::shared_ptr<SmileSection>& source,
                           Real atm = Null<Real>(),
                           bool interpolate = false,
                           bool exponentialExtrapolation = false,
                           bool deleteArbitragePoints = false,
                           const std::vector<Real>& moneynessGrid = std::vector<Real>(),
                           Real gap = 1.0E-5,
                           int forcedLeftIndex = -1,
                           int forcedRightIndex = QL_MAX_INTEGER);

        Real minStrike() const { return -shift(); }
        Real maxStrike() const { return QL_MAX_REAL; }
        Real atmLevel() const { return f_; }
        const Date& exerciseDate() const { return source_->exerciseDate(); }
        Time exerciseTime() const { return source_->exerciseTime(); }
        const DayCounter& dayCounter() const { return source_->dayCounter(); }
        const Date& referenceDate() const { return source_->referenceDate(); }
        VolatilityType volatilityType() const {
            return source_->volatilityType();
        }
        Real shift() const { return source_->shift(); }

        Real leftCoreStrike() const { return k_[leftIndex_]; }
        Real rightCoreStrike() const { return k_[rightIndex_]; }

        std::pair<Size, Size> coreIndices() const {
            return std::make_pair(leftIndex_, rightIndex_);
        }

        Real optionPrice(Rate strike, Option::Type type = Option::Call,
                         Real discount = 1.0) const;

      protected:
        Volatility volatilityImpl(Rate strike) const;

      private:
        Size index(Rate strike) const;
        void compute();
        ext::shared_ptr<SmileSection> source_;
        std::vector<Real> moneynessGrid_, k_, c_;
        Real f_;
        const Real gap_;
        Size leftIndex_, rightIndex_;
        std::vector<ext::shared_ptr<cFunction> > cFunctions_;
        const bool interpolate_, exponentialExtrapolation_;
        int forcedLeftIndex_, forcedRightIndex_;
        ext::shared_ptr<SmileSectionUtils> ssutils_;
    };
}

#endif
