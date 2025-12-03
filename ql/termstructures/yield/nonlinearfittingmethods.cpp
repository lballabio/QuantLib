/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo
 Copyright (C) 2010 Alessandro Roveda
 Copyright (C) 2015 Andres Hernandez

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/bernsteinpolynomial.hpp>
#include <ql/termstructures/yield/nonlinearfittingmethods.hpp>
#include <utility>

namespace QuantLib {

    ExponentialSplinesFitting::ExponentialSplinesFitting(
        bool constrainAtZero,
        const Array& weights,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        const Size numCoeffs,
        const Real fixedKappa,
        Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)),
      numCoeffs_(numCoeffs), fixedKappa_(fixedKappa) {
        QL_REQUIRE(ExponentialSplinesFitting::size() > 0, "At least 1 unconstrained coefficient required");
    }

    ExponentialSplinesFitting::ExponentialSplinesFitting(
        bool constrainAtZero,
        const Array& weights,
        const Array& l2, const Real minCutoffTime, const Real maxCutoffTime,
        const Size numCoeffs, const Real fixedKappa,
        Constraint constraint)
    : ExponentialSplinesFitting(constrainAtZero, weights, {}, l2,
                                minCutoffTime, maxCutoffTime,
                                numCoeffs, fixedKappa, std::move(constraint)) {}

    ExponentialSplinesFitting::ExponentialSplinesFitting(
        bool constrainAtZero,
        const Size numCoeffs,
        const Real fixedKappa,
        const Array& weights,
        Constraint constraint)
    : ExponentialSplinesFitting(constrainAtZero, weights, {}, Array(),
                                0.0, QL_MAX_REAL,
                                numCoeffs, fixedKappa, std::move(constraint)) {}

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    ExponentialSplinesFitting::clone() const {
        return std::make_unique<ExponentialSplinesFitting>(*this); 
    }

    Size ExponentialSplinesFitting::size() const {
        Size N = constrainAtZero_ ? numCoeffs_ : numCoeffs_ + 1;
        
        return (fixedKappa_ != Null<Real>()) ? N-1 : N; //One fewer optimization parameters if kappa is fixed
    }

    DiscountFactor ExponentialSplinesFitting::discountFunction(const Array& x,
                                                               Time t) const {
        DiscountFactor d = 0.0;
        Size N = size();
        //Use the interal fixedKappa_ member if non-zero, otherwise take kappa from the passed x[] array
        Real kappa = (fixedKappa_ != Null<Real>()) ? fixedKappa_: x[N-1];
        Real coeff = 0;

        if (!constrainAtZero_) {
            for (Size i = 0; i < N - 1; ++i) {
                d += x[i] * std::exp(-kappa * (i + 1) * t);
            }
        } else {
            //  notation:
            //  d(t) = coeff* exp(-kappa*1*t) + x[0]* exp(-kappa*2*t) +
            //  x[1]* exp(-kappa*3*t) + ..+ x[7]* exp(-kappa*9*t)
            for (Size i = 0; i < N - 1; i++) {
                d += x[i] * std::exp(-kappa * (i + 2) * t);
                coeff += x[i];
            }
            coeff = 1.0 - coeff;
            d += coeff * std::exp(-kappa * t);
        }

        return d;
    }


    NelsonSiegelFitting::NelsonSiegelFitting(
        const Array& weights,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(true, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    NelsonSiegelFitting::NelsonSiegelFitting(
        const Array& weights,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : NelsonSiegelFitting(weights, {}, l2,
                          minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    NelsonSiegelFitting::clone() const {
        return std::make_unique<NelsonSiegelFitting>(*this);
    }

    Size NelsonSiegelFitting::size() const {
        return 4;
    }

    DiscountFactor NelsonSiegelFitting::discountFunction(const Array& x,
                                                         Time t) const {
        Real kappa = x[size()-1];
        Real zeroRate = x[0] + (x[1] + x[2])*
                        (1.0 - std::exp(-kappa*t))/
                        ((kappa+QL_EPSILON)*(t+QL_EPSILON)) -
                        (x[2])*std::exp(-kappa*t);
        DiscountFactor d = std::exp(-zeroRate * t) ;
        return d;
    }


    SvenssonFitting::SvenssonFitting(const Array& weights,
                                     const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
                                     const Array& l2,
                                     const Real minCutoffTime,
                                     const Real maxCutoffTime,
                                     Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(true, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    SvenssonFitting::SvenssonFitting(const Array& weights,
                                     const Array& l2,
                                     const Real minCutoffTime,
                                     const Real maxCutoffTime,
                                     Constraint constraint)
    : SvenssonFitting(weights, {}, l2,
                      minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    SvenssonFitting::clone() const {
        return std::make_unique<SvenssonFitting>(*this);
    }

    Size SvenssonFitting::size() const {
        return 6;
    }

    DiscountFactor SvenssonFitting::discountFunction(const Array& x,
                                                     Time t) const {
        Real kappa = x[size()-2];
        Real kappa_1 = x[size()-1];

        Real zeroRate = x[0] + (x[1] + x[2])*
                        (1.0 - std::exp(-kappa*t))/
                        ((kappa+QL_EPSILON)*(t+QL_EPSILON)) -
                        (x[2])*std::exp(-kappa*t) +
                        x[3]* (((1.0 - std::exp(-kappa_1*t))/((kappa_1+QL_EPSILON)*(t+QL_EPSILON)))- std::exp(-kappa_1*t));
        DiscountFactor d = std::exp(-zeroRate * t) ;
        return d;
    }


    CubicBSplinesFitting::CubicBSplinesFitting(
        const std::vector<Time>& knots,
        bool constrainAtZero,
        const Array& weights,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)),
      splines_(3, knots.size() - 5, knots) {

        QL_REQUIRE(knots.size() >= 8,
                   "At least 8 knots are required" );
        Size basisFunctions = knots.size() - 4;

        if (constrainAtZero) {
            size_ = basisFunctions-1;

            // Note: A small but nonzero N_th basis function at t=0 may
            // lead to an ill conditioned problem
            N_ = 1;

            QL_REQUIRE(std::abs(splines_(N_, 0.0)) > QL_EPSILON,
                       "N_th cubic B-spline must be nonzero at t=0");
        } else {
            size_ = basisFunctions;
            N_ = 0;
        }
    }

    CubicBSplinesFitting::CubicBSplinesFitting(
        const std::vector<Time>& knots,
        bool constrainAtZero,
        const Array& weights,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : CubicBSplinesFitting(knots, constrainAtZero, weights, {}, l2,
                           minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    Real CubicBSplinesFitting::basisFunction(Integer i, Time t) const {
        return splines_(i,t);
    }

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    CubicBSplinesFitting::clone() const {
        return std::make_unique<CubicBSplinesFitting>(*this);
    }

    Size CubicBSplinesFitting::size() const {
        return size_;
    }

    DiscountFactor CubicBSplinesFitting::discountFunction(const Array& x,
                                                          Time t) const {
        DiscountFactor d = 0.0;

        if (!constrainAtZero_) {
            for (Size i=0; i<size_; ++i) {
                d += x[i] * splines_(i,t);
            }
        } else {
            const Real T = 0.0;
            Real sum = 0.0;
            for (Size i=0; i<size_; ++i) {
                if (i < N_) {
                    d += x[i] * splines_(i,t);
                    sum += x[i] * splines_(i,T);
                } else {
                    d += x[i] * splines_(i+1,t);
                    sum += x[i] * splines_(i+1,T);
                }
            }
            Real coeff = 1.0 - sum;
            coeff /= splines_(N_,T);
            d += coeff * splines_(N_,t);
        }

        return d;
    }

    NaturalCubicFitting::NaturalCubicFitting(
        const std::vector<Time>& knotTimes,
        const Array& weights,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const Array& l2,
        Real minCutoffTime,
        Real maxCutoffTime,
        Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(true, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)),
      knotTimes_(knotTimes) {
        knotTimes_.push_back(0.0);
        std::sort(knotTimes_.begin(), knotTimes_.end());

        auto last = std::unique(knotTimes_.begin(), knotTimes_.end(),
                                [](Time a, Time b){ return std::fabs(a - b) <= 1e-14; });
        knotTimes_.erase(last, knotTimes_.end());

        QL_REQUIRE(knotTimes_.size() >= 2,
                   "NaturalCubicFitting: at least two knot times required");

        const Size n = knotTimes_.size();
        size_ = n - 1;

        h_.assign(n - 1, 0.0);
        for (Size i = 0; i + 1 < n; ++i) {
            h_[i] = knotTimes_[i+1] - knotTimes_[i];
            QL_REQUIRE(h_[i] > 1e-14,
                       "NaturalCubicFitting: knot times must be strictly increasing (non-zero spacing)");
            QL_REQUIRE(std::isfinite(h_[i]),
                       "NaturalCubicFitting: non-finite knot spacing");
        }
    }

    NaturalCubicFitting::NaturalCubicFitting(
        const std::vector<Time>& knotTimes,
        const Array& weights,
        const Array& l2,
        Real minCutoffTime,
        Real maxCutoffTime,
        Constraint constraint)
    : NaturalCubicFitting(knotTimes, weights, {}, l2,
                          minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    NaturalCubicFitting::clone() const {
        return std::make_unique<NaturalCubicFitting>(*this);
    }

    Size NaturalCubicFitting::size() const {
        return size_;
    }

    DiscountFactor NaturalCubicFitting::discountFunction(const Array& x, Time t) const {
        const Size n = knotTimes_.size();
        QL_REQUIRE(n >= 2, "NaturalCubicFitting::discountFunction(): insufficient knotTimes");

        const Size expected = size();
        QL_REQUIRE(x.size() == expected,
                   "NaturalCubicFitting::discountFunction(): parameter size mismatch: expected "
                   << expected << " got " << x.size());

        Array y(n, 0.0);
        if (this->constrainAtZero()) {
            y[0] = 1.0;
            for (Size i = 1; i < n; ++i)
                y[i] = x[i - 1];
        } else {
            for (Size i = 0; i < n; ++i)
                y[i] = x[i];
        }

        for (Size i = 0; i < n; ++i)
            QL_REQUIRE(std::isfinite(y[i]), "NaturalCubicFitting::discountFunction(): non-finite nodal value");

        CubicNaturalSpline spline(knotTimes_.begin(), knotTimes_.end(), y.begin());
        spline.update();
        return spline(std::clamp(t, knotTimes_.front(), knotTimes_.back()));
    }


    SimplePolynomialFitting::SimplePolynomialFitting(
        Natural degree,
        bool constrainAtZero,
        const Array& weights,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)),
      size_(constrainAtZero ? degree : degree + 1) {}

    SimplePolynomialFitting::SimplePolynomialFitting(
        Natural degree,
        bool constrainAtZero,
        const Array& weights,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : SimplePolynomialFitting(degree, constrainAtZero, weights, {}, l2,
                              minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    SimplePolynomialFitting::clone() const {
        return std::make_unique<SimplePolynomialFitting>(*this);
    }

    Size SimplePolynomialFitting::size() const {
        return size_;
    }

    DiscountFactor SimplePolynomialFitting::discountFunction(const Array& x,
                                                             Time t) const {
        DiscountFactor d = 0.0;

        if (!constrainAtZero_) {
            for (Size i=0; i<size_; ++i)
                d += x[i] * BernsteinPolynomial::get(i,i,t);
        } else {
            d = 1.0;
            for (Size i=0; i<size_; ++i)
                d += x[i] * BernsteinPolynomial::get(i+1,i+1,t);
        }
        return d;
    }

    SpreadFittingMethod::SpreadFittingMethod(const ext::shared_ptr<FittingMethod>& method,
                                             Handle<YieldTermStructure> discountCurve,
                                             const Real minCutoffTime,
                                             const Real maxCutoffTime)
    : FittedBondDiscountCurve::FittingMethod(
          method != nullptr ? method->constrainAtZero() : true,
          method != nullptr ? method->weights() : Array(),
          method != nullptr ? method->optimizationMethod() : ext::shared_ptr<OptimizationMethod>(),
          method != nullptr ? method->l2() : Array(),
          minCutoffTime,
          maxCutoffTime),
      method_(method), discountingCurve_(std::move(discountCurve)) {
        QL_REQUIRE(method, "Fitting method is empty");
        QL_REQUIRE(!discountingCurve_.empty(), "Discounting curve cannot be empty");
    }

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    SpreadFittingMethod::clone() const {
        return std::make_unique<SpreadFittingMethod>(*this);
    }

    Size SpreadFittingMethod::size() const {
        return method_->size();
    }

    DiscountFactor SpreadFittingMethod::discountFunction(const Array& x, Time t) const{
        return method_->discount(x, t)*discountingCurve_->discount(t, true)/rebase_;
    }

    void SpreadFittingMethod::init(){
        //In case discount curve has a different reference date,
        //discount to this curve's reference date
        if (curve_->referenceDate() != discountingCurve_->referenceDate()){
            rebase_ = discountingCurve_->discount(curve_->referenceDate());
        }
        else{
            rebase_ = 1.0;
        }
        //Call regular init
        FittedBondDiscountCurve::FittingMethod::init();
    }
}

