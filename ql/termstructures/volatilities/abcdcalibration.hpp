/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco

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

#ifndef quantlib_abcdcalibration_hpp
#define quantlib_abcdcalibration_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatilities/abcd.hpp>

#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/utilities/null.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace QuantLib {
    
    class Quote;

    //! %AbcdCalibration
    class AbcdCalibration {
      public:
        AbcdCalibration(
             const std::vector<Real>& t,
             const std::vector<Real>& blackVols,
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
                      = boost::shared_ptr<OptimizationMethod>());

        //! adjustment factors needed to match Black vols
        std::vector<Real> k(const std::vector<Real>& t,
                            const std::vector<Real>& blackVols) const;
        void compute();
        //calibration results
        Real value(Real x) const;
        Real error() const;
        Real maxError() const;
        Disposable<Array> errors() const;
        EndCriteria::Type endCriteria() const;

        Real a() const;
        Real b() const;
        Real c() const;
        Real d() const;

        bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
        Real a_, b_, c_, d_;

      private:
       
        //! Parameters
        std::vector<Real> times_, blackVols_;

        //! optimization constraints
        class AbcdConstraint : public Constraint {
            private:
            class Impl : public Constraint::Impl {
                public:
                bool test(const Array& params) const {
                    return params[0] + params[3] > 0.0  // a + d
                        && params[2] > 0.0              // c
                        && params[3] > 0.0;             // d
                }
            };
            public:
            AbcdConstraint()
                : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl)) {}
        };
        //! function to minimize
        class AbcdCostFunction;
        friend class AbcdCostFunction;
        class AbcdCostFunction : public CostFunction {
          public:
            AbcdCostFunction(AbcdCalibration* abcd)
            : abcd_(abcd) {}

            Real value(const Array& x) const {
                if (!abcd_->aIsFixed_) abcd_->a_ = x[0];
                if (!abcd_->bIsFixed_) abcd_->b_ = x[1];
                if (!abcd_->cIsFixed_) abcd_->c_ = x[2];
                if (!abcd_->dIsFixed_) abcd_->d_ = x[3];
                return abcd_->error();
            }
            Disposable<Array> values(const Array& x) const {
                if (!abcd_->aIsFixed_) abcd_->a_ = x[0];
                if (!abcd_->bIsFixed_) abcd_->b_ = x[1];
                if (!abcd_->cIsFixed_) abcd_->c_ = x[2];
                if (!abcd_->dIsFixed_) abcd_->d_ = x[3];
                return abcd_->errors();
            }
          private:
            AbcdCalibration* abcd_;
        };


        // optimization method used for fitting
        mutable EndCriteria::Type abcdEndCriteria_;
        boost::shared_ptr<EndCriteria> endCriteria_;
        boost::shared_ptr<OptimizationMethod> method_;
        mutable std::vector<Real> weights_;
        bool vegaWeighted_;
    };


}

#endif
