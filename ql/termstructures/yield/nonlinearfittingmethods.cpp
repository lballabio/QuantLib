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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/termstructures/yield/nonlinearfittingmethods.hpp>
#include <ql/math/bernsteinpolynomial.hpp>

namespace QuantLib {

    ExponentialSplinesFitting::ExponentialSplinesFitting(bool constrainAtZero,
                                                         const Array& weights,
                                                         const Array& l2,
                                                         boost::shared_ptr<OptimizationMethod> optimizationMethod)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, l2, optimizationMethod) {}

    std::auto_ptr<FittedBondDiscountCurve::FittingMethod>
    ExponentialSplinesFitting::clone() const {
        return std::auto_ptr<FittedBondDiscountCurve::FittingMethod>(
                                        new ExponentialSplinesFitting(*this));
    }

    Size ExponentialSplinesFitting::size() const {
        return constrainAtZero_ ? 9 : 10;
    }

    DiscountFactor ExponentialSplinesFitting::discountFunction(const Array& x,
                                                               Time t) const {
        DiscountFactor d = 0.0;
        Size N = size();
        Real kappa = x[N-1];
        Real coeff = 0;

        if (!constrainAtZero_) {
            for (Size i=0; i<N-1; ++i) {
                d += x[i]* std::exp(-kappa * (i+1) * t);
            }
        } else {
            //  notation:
            //  d(t) = coeff* exp(-kappa*1*t) + x[0]* exp(-kappa*2*t) +
            //  x[1]* exp(-kappa*3*t) + ..+ x[7]* exp(-kappa*9*t)
            for (Size i=0; i<N-1; i++) {
                d += x[i]* std::exp(-kappa * (i+2) * t);
                coeff += x[i];
            }
            coeff = 1.0- coeff;
            d += coeff * std::exp(-kappa * t);
        }
        return d;
    }

    Array ExponentialSplinesFitting::gradientFunction(const Array& x,
        Time t) const {

        DiscountFactor dKappa = 0.0;
        Size N = size();
        Array gradients(N);
        Real kappa = x[N - 1];

        if (!constrainAtZero_) {
            for (Size i = 0; i<N - 1; ++i) {
                gradients[i] = std::exp(-kappa * (i + 1) * t);
                dKappa -= (i + 1) * t * x[i] * gradients[i];
            }
        }
        else {
            //  notation:
            //  d(t) = coeff* exp(-kappa*1*t) + x[0]* exp(-kappa*2*t) +
            //  x[1]* exp(-kappa*3*t) + ..+ x[7]* exp(-kappa*9*t)
            for (Size i = 0; i<N - 1; i++) {
                gradients[i] = std::exp(-kappa * (i + 2) * t);
                dKappa -= (i + 2) * t * x[i] * gradients[i];
                gradients[i] -= std::exp(-kappa * t);
                dKappa += t * x[i] * std::exp(-kappa * t);
            }
        }
        gradients[N - 1] = dKappa;
        return gradients;
    }



    NelsonSiegelFitting::NelsonSiegelFitting(const Array& weights,
                                             const Array& l2,
                                             boost::shared_ptr<OptimizationMethod> optimizationMethod)
    : FittedBondDiscountCurve::FittingMethod(true, weights, l2, optimizationMethod) {}

    std::auto_ptr<FittedBondDiscountCurve::FittingMethod>
    NelsonSiegelFitting::clone() const {
        return std::auto_ptr<FittedBondDiscountCurve::FittingMethod>(
                                              new NelsonSiegelFitting(*this));
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

    Array NelsonSiegelFitting::gradientFunction(const Array& x, Time t) const {
        Array gradients(size());
        Real kappa = x[size() - 1];
        Real exp = std::exp(-kappa*t);
        Real Z = (1.0 - exp) / ((kappa + QL_EPSILON)*(t + QL_EPSILON));
        Real zeroRate = x[0] + (x[1] + x[2]) * Z - x[2] * exp;
        Real dDdR = -t * std::exp(-zeroRate * t);

        gradients[0] = dDdR;
        gradients[1] = dDdR * Z;
        gradients[2] = dDdR * (Z - exp);
        gradients[3] = (x[1] + x[2]) / t / kappa;
        gradients[3] *= t * exp - (1 - exp) / kappa / kappa;
        gradients[3] += x[2] * t * exp;
        gradients[3] *= dDdR;

        return gradients;
    }


    SvenssonFitting::SvenssonFitting(const Array& weights,
                                     const Array& l2,
                                     boost::shared_ptr<OptimizationMethod> optimizationMethod)
    : FittedBondDiscountCurve::FittingMethod(true, weights, l2, optimizationMethod) {}

    std::auto_ptr<FittedBondDiscountCurve::FittingMethod>
    SvenssonFitting::clone() const {
        return std::auto_ptr<FittedBondDiscountCurve::FittingMethod>(
                                              new SvenssonFitting(*this));
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

    Array SvenssonFitting::gradientFunction(const Array& x, Time t) const {
        Array gradients(size());
        Real kappa = x[size() - 2];
        Real kappa_1 = x[size() - 1];
        Real exp = std::exp(-kappa*t);
        Real exp_1 = std::exp(-kappa_1*t);
        Real Z = (1.0 - exp) / ((kappa + QL_EPSILON)*(t + QL_EPSILON));
        Real Z_1 = (1.0 - exp_1) / ((kappa_1 + QL_EPSILON)*(t + QL_EPSILON));
        Real zeroRate = x[0] + (x[1] + x[2]) * Z - x[2] * exp + x[3] * (Z_1 - exp_1);
        Real dDdR = -t * std::exp(-zeroRate * t);

        gradients[0] = dDdR;
        gradients[1] = dDdR * Z;
        gradients[2] = dDdR * (Z - exp);
        gradients[3] = dDdR * (Z_1 - exp_1);
        gradients[4] = (x[1] + x[2]) / t / kappa;
        gradients[4] *= t * exp - (1 - exp) / kappa;
        gradients[4] += x[2] * t * exp;
        gradients[4] *= dDdR;
        gradients[5] = - (1 - exp_1) / t / kappa_1 / kappa_1;
        gradients[5] += exp_1 / kappa_1;
        gradients[5] += t * exp_1;
        gradients[5] *= x[3] * dDdR;

        return gradients;
    }



    CubicBSplinesFitting::CubicBSplinesFitting(const std::vector<Time>& knots,
                                               bool constrainAtZero,
                                               const Array& weights,
                                               const Array& l2,
                                               boost::shared_ptr<OptimizationMethod> optimizationMethod)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, l2, optimizationMethod),
      splines_(3, knots.size()-5, knots) {

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

    Real CubicBSplinesFitting::basisFunction(Integer i, Time t) const {
        return splines_(i,t);
    }

    std::auto_ptr<FittedBondDiscountCurve::FittingMethod>
    CubicBSplinesFitting::clone() const {
        return std::auto_ptr<FittedBondDiscountCurve::FittingMethod>(
                                             new CubicBSplinesFitting(*this));
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

    Array CubicBSplinesFitting::gradientFunction(const Array& x,
        Time t) const {

        Array gradients(size());

        if (!constrainAtZero_) {
            for (Size i = 0; i<size_; ++i) {
                gradients[i] = splines_(i, t);
            }
        }
        else {
            const Real T = 0.0;
            Real sum = 0.0;
            for (Size i = 0; i<size_; ++i) {
                if (i < N_) {
                    gradients[i] = splines_(i, t) - splines_(i, T) / splines_(N_, T);
                }
                else {
                    gradients[i] = splines_(i + 1, t) - splines_(i + 1, T) / splines_(N_, T);
                }
            }
        }

        return gradients;
    }


    SimplePolynomialFitting::SimplePolynomialFitting(Natural degree,
                                                     bool constrainAtZero,
                                                     const Array& weights,
                                                     const Array& l2,
                                                     boost::shared_ptr<OptimizationMethod> optimizationMethod)
    : FittedBondDiscountCurve::FittingMethod(constrainAtZero, weights, l2, optimizationMethod),
      size_(constrainAtZero ? degree : degree+1) {}

    std::auto_ptr<FittedBondDiscountCurve::FittingMethod>
    SimplePolynomialFitting::clone() const {
        return std::auto_ptr<FittedBondDiscountCurve::FittingMethod>(
                                          new SimplePolynomialFitting(*this));
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

    Array SimplePolynomialFitting::gradientFunction(const Array& x,
        Time t) const {
        Array gradients(size());

        if (!constrainAtZero_) {
            for (Size i = 0; i<size_; ++i)
                gradients[i] = BernsteinPolynomial::get(i, i, t);
        }
        else {
            for (Size i = 0; i<size_; ++i)
                gradients[i] = BernsteinPolynomial::get(i + 1, i + 1, t);
        }
        return gradients;
    }
    
    SpreadFittingMethod::SpreadFittingMethod(boost::shared_ptr<FittingMethod> method,
                        Handle<YieldTermStructure> discountCurve)
    : FittedBondDiscountCurve::FittingMethod(method ? method->constrainAtZero() : true, method ? method->weights() : Array(), 
                                             method ? method->l2() : Array(),
                                             method ? method->optimizationMethod() : boost::shared_ptr<OptimizationMethod>()),
      method_(method), discountingCurve_(discountCurve) {
        QL_REQUIRE(method, "Fitting method is empty");
        QL_REQUIRE(!discountingCurve_.empty(), "Discounting curve cannot be empty");
    }

    std::auto_ptr<FittedBondDiscountCurve::FittingMethod>
    SpreadFittingMethod::clone() const {
        return std::auto_ptr<FittedBondDiscountCurve::FittingMethod>(
                                          new SpreadFittingMethod(*this));
    }

    Size SpreadFittingMethod::size() const {
        return method_->size();
    }

    DiscountFactor SpreadFittingMethod::discountFunction(const Array& x, Time t) const{
        return method_->discount(x, t)*discountingCurve_->discount(t, true)/rebase_;
    }

    Array SpreadFittingMethod::gradientFunction(const Array& x, Time t) const {
        Array gradients = method_->gradients(x, t);
        Real crv_df = discountingCurve_->discount(t, true) / rebase_;
        Size n = size();

        for(Size i=0; i<n; i++)
        {
            gradients[i] *= crv_df;
        }
        return gradients;
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

