/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file abcdinterpolation.hpp
    \brief Abcd interpolation interpolation between discrete points
*/

#ifndef quantlib_abcd_interpolation_hpp
#define quantlib_abcd_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/termstructures/volatility/abcdcalibration.hpp>
#include <utility>

namespace QuantLib {

    class EndCriteria;
    class OptimizationMethod;

    namespace detail {

        class AbcdCoeffHolder {
          public:
            AbcdCoeffHolder(Real a,
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
              k_(std::vector<Real>()),
              error_(Null<Real>()),
              maxError_(Null<Real>()),
              abcdEndCriteria_(EndCriteria::None) {
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

                AbcdMathFunction::validate(a, b, c, d);
            }
            virtual ~AbcdCoeffHolder() = default;
            Real a_, b_, c_, d_;
            bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
            std::vector<Real> k_;
            Real error_, maxError_;
            EndCriteria::Type abcdEndCriteria_;
        };

        template <class I1, class I2>
        class AbcdInterpolationImpl : public Interpolation::templateImpl<I1,I2>,
                                      public AbcdCoeffHolder {
          public:
            AbcdInterpolationImpl(const I1& xBegin,
                                  const I1& xEnd,
                                  const I2& yBegin,
                                  Real a,
                                  Real b,
                                  Real c,
                                  Real d,
                                  bool aIsFixed,
                                  bool bIsFixed,
                                  bool cIsFixed,
                                  bool dIsFixed,
                                  bool vegaWeighted,
                                  ext::shared_ptr<EndCriteria> endCriteria,
                                  ext::shared_ptr<OptimizationMethod> optMethod)
            : Interpolation::templateImpl<I1, I2>(xBegin, xEnd, yBegin),
              AbcdCoeffHolder(a, b, c, d, aIsFixed, bIsFixed, cIsFixed, dIsFixed),
              endCriteria_(std::move(endCriteria)), optMethod_(std::move(optMethod)),
              vegaWeighted_(vegaWeighted) {}

            void update() override {
                auto x = this->xBegin_;
                auto y = this->yBegin_;
                std::vector<Real> times, blackVols;
                for ( ; x!=this->xEnd_; ++x, ++y) {
                    times.push_back(*x);
                    blackVols.push_back(*y);
                }
                abcdCalibrator_ = ext::shared_ptr<AbcdCalibration>(
                    new AbcdCalibration(times, blackVols,
                                        a_, b_, c_, d_,
                                        aIsFixed_, bIsFixed_,
                                        cIsFixed_, dIsFixed_,
                                        vegaWeighted_,
                                        endCriteria_,
                                        optMethod_));
                abcdCalibrator_->compute();
                a_ = abcdCalibrator_->a();
                b_ = abcdCalibrator_->b();
                c_ = abcdCalibrator_->c();
                d_ = abcdCalibrator_->d();
                k_ = abcdCalibrator_->k(times, blackVols);
                error_ = abcdCalibrator_->error();
                maxError_ = abcdCalibrator_->maxError();
                abcdEndCriteria_ = abcdCalibrator_->endCriteria();
            }
            Real value(Real x) const override {
                QL_REQUIRE(x>=0.0, "time must be non negative: " <<
                                   x << " not allowed");
                return abcdCalibrator_->value(x);
            }
            Real primitive(Real) const override { QL_FAIL("Abcd primitive not implemented"); }
            Real derivative(Real) const override { QL_FAIL("Abcd derivative not implemented"); }
            Real secondDerivative(Real) const override {
                QL_FAIL("Abcd secondDerivative not implemented");
            }
            Real k(Time t) const {
                LinearInterpolation li(this->xBegin_, this->xEnd_, this->yBegin_);
                return li(t);
            }

          private:
            const ext::shared_ptr<EndCriteria> endCriteria_;
            const ext::shared_ptr<OptimizationMethod> optMethod_;
            bool vegaWeighted_;
            ext::shared_ptr<AbcdCalibration> abcdCalibrator_;

        };

    }

    //! %Abcd interpolation between discrete points.
    /*! \ingroup interpolations
        \warning See the Interpolation class for information about the
                 required lifetime of the underlying data.
    */
    class AbcdInterpolation : public Interpolation {
      public:
        /*! Constructor */
        template <class I1, class I2>
        AbcdInterpolation(const I1& xBegin,  // x = times
                          const I1& xEnd,
                          const I2& yBegin,  // y = volatilities
                          Real a = -0.06,
                          Real b =  0.17,
                          Real c =  0.54,
                          Real d =  0.17,
                          bool aIsFixed = false,
                          bool bIsFixed = false,
                          bool cIsFixed = false,
                          bool dIsFixed = false,
                          bool vegaWeighted = false,
                          const ext::shared_ptr<EndCriteria>& endCriteria
                              = ext::shared_ptr<EndCriteria>(),
                          const ext::shared_ptr<OptimizationMethod>& optMethod
                              = ext::shared_ptr<OptimizationMethod>()) {

            impl_ = ext::shared_ptr<Interpolation::Impl>(new
                detail::AbcdInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                     a, b, c, d,
                                                     aIsFixed, bIsFixed,
                                                     cIsFixed, dIsFixed,
                                                     vegaWeighted,
                                                     endCriteria,
                                                     optMethod));
            impl_->update();
        }
        //! \name Inspectors
        //@{
        Real a() const { return coeffs().a_; }
        Real b() const { return coeffs().b_; }
        Real c() const { return coeffs().c_; }
        Real d() const { return coeffs().d_; }
        std::vector<Real> k() const { return coeffs().k_; }
        Real rmsError() const { return coeffs().error_; }
        Real maxError() const { return coeffs().maxError_; }
        EndCriteria::Type endCriteria(){ return coeffs().abcdEndCriteria_; }
        template <class I1>
        Real k(Time t, const I1& xBegin, const I1& xEnd) const {
            LinearInterpolation li(xBegin, xEnd, (coeffs().k_).begin());
            return li(t);
        }
      private:
        const detail::AbcdCoeffHolder& coeffs() const {
          return *dynamic_cast<detail::AbcdCoeffHolder*>(impl_.get());
        }
    };

    //! %Abcd interpolation factory and traits
    /*! \ingroup interpolations */
    class Abcd {
      public:
        Abcd(Real a,
             Real b,
             Real c,
             Real d,
             bool aIsFixed,
             bool bIsFixed,
             bool cIsFixed,
             bool dIsFixed,
             bool vegaWeighted = false,
             ext::shared_ptr<EndCriteria> endCriteria = ext::shared_ptr<EndCriteria>(),
             ext::shared_ptr<OptimizationMethod> optMethod = ext::shared_ptr<OptimizationMethod>())
        : a_(a), b_(b), c_(c), d_(d), aIsFixed_(aIsFixed), bIsFixed_(bIsFixed), cIsFixed_(cIsFixed),
          dIsFixed_(dIsFixed), vegaWeighted_(vegaWeighted), endCriteria_(std::move(endCriteria)),
          optMethod_(std::move(optMethod)) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return AbcdInterpolation(xBegin, xEnd, yBegin,
                                     a_, b_, c_, d_,
                                     aIsFixed_, bIsFixed_,
                                     cIsFixed_, dIsFixed_,
                                     vegaWeighted_,
                                     endCriteria_, optMethod_);
        }
        static const bool global = true;
      private:
        Real a_, b_, c_, d_;
        bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
        bool vegaWeighted_;
        const ext::shared_ptr<EndCriteria> endCriteria_;
        const ext::shared_ptr<OptimizationMethod> optMethod_;
    };

}

#endif
