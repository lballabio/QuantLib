/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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
#include <ql/voltermstructures/interestrate/abcd.hpp>
#include <ql/voltermstructures/interestrate/abcdcalibration.hpp>


namespace QuantLib {

    class EndCriteria;
    class OptimizationMethod;

    namespace detail {

        template <class I1, class I2>
        class AbcdInterpolationImpl : public Interpolation::templateImpl<I1,I2> {
        public:

            AbcdInterpolationImpl(
                const I1& xBegin, const I1& xEnd,
                const I2& yBegin,
                Real aGuess, Real bGuess,
                Real cGuess, Real dGuess,
                bool aIsFixed,
                bool bIsFixed,
                bool cIsFixed,
                bool dIsFixed,
                bool vegaWeighted,
                const boost::shared_ptr<EndCriteria>& endCriteria,
                const boost::shared_ptr<OptimizationMethod>& method)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
                aGuess_(aGuess), bGuess_(bGuess), cGuess_(cGuess), dGuess_(dGuess),
                aIsFixed_(aIsFixed), bIsFixed_(bIsFixed), cIsFixed_(cIsFixed), dIsFixed_(dIsFixed),
                vegaWeighted_(vegaWeighted),
                endCriteria_(endCriteria), method_(method) { }

            void update() {
                std::vector<Real>::const_iterator x = this->xBegin_;
                std::vector<Real>::const_iterator y = this->yBegin_;
                std::vector<Real> times, blackVols;
                for ( ; x!=this->xEnd_; ++x, ++y) {
                        times.push_back(*x);
                        blackVols.push_back(*y);
                }
                abcdCalibrator_ = AbcdCalibration(times, blackVols,
                                aGuess_, bGuess_,
                                cGuess_, dGuess_,
                                aIsFixed_, bIsFixed_,
                                cIsFixed_, dIsFixed_,
                                vegaWeighted_,
                                endCriteria_,
                                method_);
                abcdCalibrator_.compute();

            }

            Real value(Real x) const {
                QL_REQUIRE(x>=0.0, "time must be non negative: " <<
                                   x << " not allowed");
                return abcdCalibrator_.value(x);
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
          private:
            AbcdCalibration abcdCalibrator_;
        private:
            Real aGuess_, bGuess_, cGuess_, dGuess_;
            bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
            bool vegaWeighted_;
            const boost::shared_ptr<EndCriteria> endCriteria_;
            const boost::shared_ptr<OptimizationMethod> method_;

        };

    }

    //! %Abcd interpolation between discrete volatility points.
    class AbcdInterpolation : public Interpolation {
      public:
        template <class I1, class I2>
        AbcdInterpolation(const I1& xBegin,  // x = times
                          const I1& xEnd,
                          const I2& yBegin,  // y = volatilities
                          Real aGuess = -0.06,
                          Real bGuess =  0.17,
                          Real cGuess =  0.54,
                          Real dGuess =  0.17,
                          bool aIsFixed = false,
                          bool bIsFixed = false,
                          bool cIsFixed = false,
                          bool dIsFixed = false,
                          bool vegaWeighted = false,
                          const boost::shared_ptr<EndCriteria>& endCriteria
                                  = boost::shared_ptr<EndCriteria>(),
                          const boost::shared_ptr<OptimizationMethod>& method
                                  = boost::shared_ptr<OptimizationMethod>()) {


            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::AbcdInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                     aGuess, bGuess,
                                                     cGuess, dGuess,
                                                     aIsFixed, bIsFixed,
                                                     cIsFixed, dIsFixed,
                                                     vegaWeighted,
                                                     endCriteria,
                                                     method));
            impl_->update();
        }

    };

}

#endif
