/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file abcdinterpolation.hpp
    \brief Abcd interpolation interpolation between discrete points
*/

#ifndef quantlib_abcd_interpolation_hpp
#define quantlib_abcd_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/optimization/method.hpp>
#include <ql/optimization/problem.hpp>
#include <ql/optimization/armijo.hpp>
#include <ql/optimization/conjugategradient.hpp>
#include <ql/optimization/simplex.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/utilities/null.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/volatilities/abcd.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2> class AbcdInterpolationImpl;

        class AbcdCoefficientHolder {
          public:
            AbcdCoefficientHolder(Real a,
                                  Real b,
                                  Real c,
                                  Real d,
                                  bool aIsFixed,
                                  bool bIsFixed,
                                  bool cIsFixed,
                                  bool dIsFixed)
            : a_(a), b_(b), c_(c), d_(d),
              aIsFixed_(false), bIsFixed_(false),
              cIsFixed_(false), dIsFixed_(false),
              error_(Null<Real>()), maxError_(Null<Real>()),
              AbcdEndCriteria_(EndCriteria::none)
            {
                if (a_ != Null<Real>())
                    aIsFixed_ = aIsFixed;
                else a_ = -0.06;

                if (b_ != Null<Real>())
                    bIsFixed_ = bIsFixed;
                else b_ = 0.17;

                if (c_ != Null<Real>())
                    cIsFixed_ = cIsFixed;
                else c_ = 0.54;

                if (d_ != Null<Real>())
                    dIsFixed_ = dIsFixed;
                else d_ = 0.17;

                validateAbcdParameters(a_, b_, c_, d_);
            }
            virtual ~AbcdCoefficientHolder() {}

            /*! Abcd parameters */
            Real a_, b_, c_, d_;
            bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
            Real error_, maxError_;
            EndCriteria::Type AbcdEndCriteria_;
        };

    }

    //! %Abcd interpolation between discrete volatility points.
    class AbcdInterpolation : public Interpolation {
      public:
        template <class I1, class I2>
        AbcdInterpolation(const I1& xBegin,  // x = times
                          const I1& xEnd,
                          const I2& yBegin,  // y = volatilities
                          Real a,
                          Real b,
                          Real c,
                          Real d,
                          bool aIsFixed,
                          bool bIsFixed,
                          bool cIsFixed,
                          bool dIsFixed,
                          bool vegaWeighted = false,
                          const boost::shared_ptr<OptimizationMethod>& method
                                  = boost::shared_ptr<OptimizationMethod>(),
                          bool calculate = true) {

            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::AbcdInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                     a, b, c, d,
                                                     aIsFixed, bIsFixed,
                                                     cIsFixed, dIsFixed,
                                                     vegaWeighted,
                                                     method,
                                                     calculate));
            coeffs_ =
                boost::dynamic_pointer_cast<detail::AbcdCoefficientHolder>(
                                                                       impl_);
        }

        Real a() const { return coeffs_->a_; }
        Real b() const { return coeffs_->b_; }
        Real c() const { return coeffs_->c_; }
        Real d() const { return coeffs_->d_; }

        Real interpolationError() const { return coeffs_->error_; }
        Real interpolationMaxError() const { return coeffs_->maxError_; }
        //const std::vector<Real>& interpolationWeights() const {
        //    return impl_->weights_; }
        EndCriteria::Type endCriteria(){ return coeffs_->AbcdEndCriteria_; }

      private:
        boost::shared_ptr<detail::AbcdCoefficientHolder> coeffs_;
    };

    //! %Abcd interpolation factory
    class Abcd {
      public:
        Abcd(Real a, Real b, Real c, Real d,
             bool aIsFixed, bool bIsFixed, bool cIsFixed, bool dIsFixed,
             bool vegaWeighted = false,
             const boost::shared_ptr<OptimizationMethod> method
                = boost::shared_ptr<OptimizationMethod>())
        : a_(a), b_(b), c_(c), d_(d),
          aIsFixed_(aIsFixed), bIsFixed_(bIsFixed),
          cIsFixed_(cIsFixed), dIsFixed_(dIsFixed),
          vegaWeighted_(vegaWeighted), method_(method) {}
        Abcd() {};

        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return AbcdInterpolation(xBegin, xEnd, yBegin,
                                     a_, b_, c_, d_,
                                     aIsFixed_, bIsFixed_,
                                     cIsFixed_, dIsFixed_,
                                     vegaWeighted_, method_);
        }
      private:
        Real a_, b_, c_, d_;
        bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
        bool vegaWeighted_;
        const boost::shared_ptr<OptimizationMethod> method_;
    };



    namespace detail {

        template <class I1, class I2>
        class AbcdInterpolationImpl : public Interpolation::templateImpl<I1,I2>,
                                      public AbcdCoefficientHolder {
          private:

            // function to minimize
            class AbcdError;
            friend class AbcdError;
            class AbcdError : public CostFunction {
              public:
                AbcdError(AbcdInterpolationImpl* abcd)
                : abcd_(abcd) {}
                Real value(const Array& x) const {

                    // INVERSE TRANSFORMATION
                    if (!abcd_->bIsFixed_) abcd_->b_ = x[1];
                    if (!abcd_->cIsFixed_) abcd_->c_ = x[2]*x[2];
                    if (!abcd_->dIsFixed_) abcd_->d_ = x[3]*x[3];
                    if (!abcd_->aIsFixed_) abcd_->a_ = x[0]*x[0]-abcd_->d_;

                    return abcd_->interpolationSquaredError();
                }
              private:
                AbcdInterpolationImpl* abcd_;
            };

            // optimization method used for fitting
            boost::shared_ptr<OptimizationMethod> method_;
            std::vector<Real> weights_;
          public:
            AbcdInterpolationImpl(
                const I1& xBegin, const I1& xEnd,
                const I2& yBegin,
                Real a, Real b, Real c, Real d,
                bool aIsFixed,
                bool bIsFixed,
                bool cIsFixed,
                bool dIsFixed,
                bool vegaWeighted,
                const boost::shared_ptr<OptimizationMethod>& method,
                bool compute)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              AbcdCoefficientHolder(a, b, c, d,
                                    aIsFixed, bIsFixed, cIsFixed, dIsFixed),
              method_(method), weights_(xEnd-xBegin, 1.0) {
                Real weightsSum = this->xEnd_-this->xBegin_;
                if (vegaWeighted) {
                    std::vector<Real>::const_iterator x = this->xBegin_;
                    std::vector<Real>::const_iterator y = this->yBegin_;
                    std::vector<Real>::iterator w = weights_.begin();
                    weightsSum = 0.0;
                    for ( ; x!=this->xEnd_; ++x, ++y, ++w) {
                        // unable to vcalculate vega:
                        // discount and forward are missing
                        //Real stdDev = std::sqrt((*y)*(*y)*t);
                        //*w = blackStdDevDerivative(*x, forward, stdDev);
                        *w = 1.0;
                        weightsSum += *w;
                    }
                }
                // weight normalization
                std::vector<Real>::iterator w = weights_.begin();
                for ( ; w!=weights_.end(); ++w)
                    *w /= weightsSum;

                // volatility values might be not set at construction time
                if (compute)
                    calculate();
            }

            void calculate()
            {
                // there is nothing to optimize
                if (aIsFixed_ && bIsFixed_ && cIsFixed_ && dIsFixed_) {
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError();
                    AbcdEndCriteria_ = EndCriteria::none;
                    return;
                } else {
                    NoConstraint constraint;
                    AbcdError costFunction(this);

                    if (!method_) {
                        // DIRECT TRANSFORMATION
                        Array guess(4);
                        guess[0] = std::sqrt(a_+d_);
                        guess[1] = b_;
                        guess[2] = std::sqrt(c_);
                        guess[3] = std::sqrt(d_);

                        EndCriteria endCriteria(60000, 100, 1e-8, 1e-8, 1e-8);

                        //boost::shared_ptr<LineSearch> lineSearch(new
                        //    ArmijoLineSearch(1e-12, 0.15, 0.55));

                        //method_ = boost::shared_ptr<OptimizationMethod>(new
                        //    ConjugateGradient(guess, endCriteria, lineSearch));

                        method_ = boost::shared_ptr<OptimizationMethod>(new
                            Simplex(0.01, guess, endCriteria));
                    } else { // transform the guess
                        // DIRECT TRANSFORMATION
                        Array guess(4);
                        const Array& abcdParameters = method_->x();
                        guess[0] = std::sqrt(abcdParameters[0]+abcdParameters[3]);
                        guess[1] = abcdParameters[1];
                        guess[2] = std::sqrt(abcdParameters[2]);
                        guess[3] = std::sqrt(abcdParameters[3]);
                        method_->setInitialValue(guess);
                    }

                    Problem problem(costFunction, constraint, *method_);
                    problem.minimize();
                    const Array& x = problem.minimumValue();

                    // INVERSE TRANSFORMATION
                    if (!bIsFixed_) b_ = x[1];
                    if (!cIsFixed_) c_ = x[2]*x[2];
                    if (!dIsFixed_) d_ = x[3]*x[3];
                    if (!aIsFixed_) a_ = x[0]*x[0]-d_;
                }

                AbcdEndCriteria_ = endCriteria();
                error_ = interpolationError();
                maxError_ = interpolationMaxError();
            }

            Real value(Real x) const {
                QL_REQUIRE(x>=0.0, "time must be non negative: " <<
                                   x << " not allowed");
                return abcdVolatility(x, forward_, t_,
                                      a_, b_, c_, d_);
            }
            Real primitive(Real) const {
                QL_FAIL("Abcd primitive not implemented");
            }
            Real derivative(Real) const {
                QL_FAIL("Abcd derivative not implemented");
            }
            Real secondDerivative(Real) const {
                QL_FAIL("Abcd secondDerivative not implemented");
            }

            Real interpolationSquaredError() const {
                Real error, totalError = 0.0;
                std::vector<Real>::const_iterator x = this->xBegin_;
                std::vector<Real>::const_iterator y = this->yBegin_;
                std::vector<Real>::const_iterator w = weights_.begin();
                for (; x != this->xEnd_; ++x, ++y, ++w) {
                    error = (value(*x) - *y);
                    totalError += error*error * (*w);
                }
                return totalError;
            }

            Real interpolationError() const {
                Size n = this->xEnd_-this->xBegin_;
                Real squaredError = interpolationSquaredError();
                return std::sqrt(n*squaredError/(n-1));
            }

            Real interpolationMaxError() const {
                Real error, maxError = QL_MIN_REAL;
                I1 i = this->xBegin_;
                I2 j = this->yBegin_;
                for (; i != this->xEnd_; ++i, ++j) {
                    error = std::fabs(value(*i) - *j);
                    maxError = std::max(maxError, error);
                }
                return maxError;
            }

            EndCriteria::Type endCriteria() {
                return method_->endCriteria().criteria();
            }
        };

    }

}

#endif
