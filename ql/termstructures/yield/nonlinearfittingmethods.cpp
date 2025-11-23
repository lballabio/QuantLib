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
        bool constrainAtZero,
        const Array& weights,
        const ext::shared_ptr<OptimizationMethod>& optimizationMethod,
        const Array& l2,
        Real minCutoffTime,
        Real maxCutoffTime,
        Constraint constraint)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, optimizationMethod, l2,
                                             minCutoffTime, maxCutoffTime, std::move(constraint)),
      knotTimes_(knotTimes) {
        std::sort(knotTimes_.begin(), knotTimes_.end());
        auto last = std::unique(knotTimes_.begin(), knotTimes_.end(),
                                [](Time a, Time b){ return std::fabs(a-b) <= 1e-14; });
        knotTimes_.erase(last, knotTimes_.end());
        if (knotTimes_.size() < 2) {
            QL_REQUIRE(knotTimes_.size() >= 2, "NaturalCubicFitting: at least two knot times required");
        }
        Size k_s = knotTimes_.size();
        if (constrainAtZero){
            size_ = k_s - 1;
        }
        else {
            size_ = k_s;
        }
        const Size n = k_s;
        h_.assign(n - 1, 0.0);
        for (Size i = 0; i + 1 < n; ++i) {
            h_[i] = knotTimes_[i+1] - knotTimes_[i];
            QL_REQUIRE(h_[i] > 1e-14, "NaturalCubicFitting::init(): knot times must be strictly increasing (non-zero spacing)");
            QL_REQUIRE(std::isfinite(h_[i]), "NaturalCubicFitting::init(): non-finite knot spacing");
        }

        const Size m = (n >= 3) ? n - 2 : 0;
        a_.assign(m, 0.0);
        b_.assign(m, 0.0);
        c_.assign(m, 0.0);

        if (m > 0) {
            for (Size j = 0; j < m; ++j) {
                const Real h_im1 = h_[j];
                const Real h_i   = h_[j+1];
                QL_REQUIRE(std::isfinite(h_im1) && std::isfinite(h_i),
                           "NaturalCubicFitting::init(): non-finite h values");
                b_[j] = 2.0 * (h_im1 + h_i);
                a_[j] = (j == 0) ? 0.0 : h_im1;
                c_[j] = (j + 1 == m) ? 0.0 : h_i;
                QL_REQUIRE(b_[j] > 1e-16, "NaturalCubicFitting::init(): diagonal too small (degenerate knot spacing?)");
            }
        }
    }
    NaturalCubicFitting::NaturalCubicFitting(
        const std::vector<Time>& knots,
        bool constrainAtZero,
        const Array& weights,
        const Array& l2,
        const Real minCutoffTime,
        const Real maxCutoffTime,
        Constraint constraint)
    : NaturalCubicFitting(knots, constrainAtZero, weights, {}, l2,
                           minCutoffTime, maxCutoffTime, std::move(constraint)) {}

    std::unique_ptr<FittedBondDiscountCurve::FittingMethod>
    NaturalCubicFitting::clone() const {
        return std::make_unique<NaturalCubicFitting>(*this);
    }

    Size NaturalCubicFitting::size() const {
        return size_;
    }

    Array NaturalCubicFitting::solveTridiagonal(const Array& rhs) const {
        const Size m = rhs.size();
        Array out(m, 0.0);
        if (m == 0) return out;

        QL_REQUIRE(a_.size() == m && b_.size() == m && c_.size() == m,
                   "NaturalCubicFitting::solveTridiagonal(): tridiagonal coefficient size mismatch");

        std::vector<Real> cp(m, 0.0);
        Array dp(m, 0.0);

        Real denom = b_[0];
        QL_REQUIRE(std::fabs(denom) > 1e-18, "NaturalCubicFitting::solveTridiagonal(): diagonal too small");
        cp[0] = (m > 1) ? c_[0] / denom : 0.0;
        dp[0] = rhs[0] / denom;

        for (Size i = 1; i < m; ++i) {
            denom = b_[i] - a_[i] * cp[i-1];
            QL_REQUIRE(std::fabs(denom) > 1e-18, "NaturalCubicFitting::solveTridiagonal(): tridiagonal matrix nearly singular");
            cp[i] = (i + 1 < m) ? c_[i] / denom : 0.0;
            dp[i] = (rhs[i] - a_[i] * dp[i-1]) / denom;
        }

        out[m-1] = dp[m-1];
        for (int i = (int)m - 2; i >= 0; --i)
            out[i] = dp[i] - cp[i] * out[i+1];

        return out;
    }

    Size NaturalCubicFitting::findInterval(Time t) const {
        const Size n = knotTimes_.size();
        if (t <= knotTimes_.front()) return 0;
        if (t >= knotTimes_.back()) return n - 2;
        auto it = std::upper_bound(knotTimes_.begin(), knotTimes_.end(), t);
        Size idx = std::distance(knotTimes_.begin(), it);
        QL_REQUIRE(idx > 0, "NaturalCubicFitting::findInterval(): internal error");
        idx = idx - 1;
        if (idx >= n - 1) idx = n - 2;
        return idx;
    }

    DiscountFactor NaturalCubicFitting::discountFunction(const Array& x, Time t) const {
        const Size n = knotTimes_.size();
        QL_REQUIRE(n >= 2, "NaturalCubicFitting::discountFunction(): insufficient knotTimes");
        const Size expected = size();
        QL_REQUIRE(x.size() == expected,
                   "NaturalCubicFitting::discountFunction(): parameter size mismatch: expected "
                   << expected << " got " << x.size());

        Array y(n, 0.0);
        if (constrainAtZero_) {
            y[0] = 1.0;
            for (Size i = 1; i < n; ++i) y[i] = x[i - 1];
        } else {
            for (Size i = 0; i < n; ++i) y[i] = x[i];
        }

        for (Size i = 0; i < n; ++i) {
            QL_REQUIRE(std::isfinite(y[i]), "NaturalCubicFitting::discountFunction(): non-finite nodal value");
        }
        if (t <= knotTimes_.front()) return y.front();
        if (t >= knotTimes_.back())  return y.back();

        if (n == 2) {
            const Real t0 = knotTimes_[0], t1 = knotTimes_[1];
            const Real h = t1 - t0;
            QL_REQUIRE(std::fabs(h) > 0.0, "NaturalCubicFitting::discountFunction(): zero interval width");
            const Real w0 = (t1 - t) / h, w1 = (t - t0) / h;
            return y[0] * w0 + y[1] * w1;
        }
        for (Size idx = 0; idx < y.size(); ++idx) {
            QL_REQUIRE(std::isfinite(y[idx]),
                       "y[" << idx << "] is non-finite (value=" << y[idx] << ")");
        }

        const Size m = n - 2;
       
        Array rhs(m, 0.0);
        for (Size j = 0; j < m; ++j) {
            const Size k = j + 1;
            const Real h_im1 = h_.at(k-1);
            const Real h_i   = h_.at(k);
            rhs[j] = 6.0 * ( (y[k+1] - y[k]) / h_i - (y[k] - y[k-1]) / h_im1 );
            QL_REQUIRE(std::isfinite(rhs[j]),
               "RHS non-finite at j=" << j
               << " (h_im1=" << h_im1 << ", h_i=" << h_i << ")");
        }
        Array M_interior = solveTridiagonal(rhs);

        Array M(n, 0.0);
        for (Size j = 0; j < m; ++j)
            M[j+1] = M_interior[j];

        const Size idx = findInterval(t);
        const Real xi = knotTimes_.at(idx);
        const Real xi1 = knotTimes_.at(idx+1);
        const Real hi = xi1 - xi;
        QL_REQUIRE(hi > 0.0, "NaturalCubicFitting::discountFunction(): zero interval width detected");
        const Real ti = t - xi;
        const Real ti1 = xi1 - t;

        const Real yi = y.at(idx);
        const Real yi1 = y.at(idx+1);
        const Real Mi = M.at(idx);
        const Real Mi1 = M.at(idx+1);

        const Real term1 = (Mi * ti1 * ti1 * ti1 + Mi1 * ti * ti * ti) / (6.0 * hi);
        const Real term2 = (yi - Mi * hi * hi / 6.0) * (ti1 / hi);
        const Real term3 = (yi1 - Mi1 * hi * hi / 6.0) * (ti / hi);

        const Real result = term1 + term2 + term3;
        QL_REQUIRE(std::isfinite(result), "NaturalCubicFitting::discountFunction(): non-finite result");
        return result;
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

