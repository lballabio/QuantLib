
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cubicspline.hpp
    \brief cubic spline interpolation between discrete points
*/

#ifndef quantlib_cubic_spline_hpp
#define quantlib_cubic_spline_hpp

#include <ql/Math/interpolation.hpp>
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/Utilities/null.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        template <class I1, class I2> class CubicSplineImpl;

        class CoefficientHolder {
          public:
            CoefficientHolder(Size n)
            : n_(n), primitiveConst_(n-1), a_(n-1), b_(n-1), c_(n-1) {}
            virtual ~CoefficientHolder() {};
            Size n_;
            // P[i](x) = y[i] +
            //           a[i]*(x-x[i]) +
            //           b[i]*(x-x[i])^2 +
            //           c[i]*(x-x[i])^3
            std::vector<Real> primitiveConst_, a_, b_, c_;
        };

    }

    //! %Cubic spline interpolation between discrete points.
    /*! It implements different type of end conditions: not-a-knot,
        first derivative value, second derivative value.

        It also implements Hyman's monotonicity constraint filter
        which ensures that the interpolating spline remains monotonic
        at the expense of the second derivative of the curve which will no
        longer be continuous where the filter has been applied.
        If the interpolating spline is already monotonic, the Hyman filter
        leaves it unchanged.

        See R. L. Dougherty, A. Edelman, and J. M. Hyman,
        "Nonnegativity-, Monotonicity-, or Convexity-Preserving Cubic and
        Quintic Hermite Interpolation"
        Mathematics Of Computation, v. 52, n. 186, April 1989, pp. 471-494.

        \test the correctness of the returned values is tested by
              reproducing results available in literature.
    */
    class CubicSpline : public Interpolation {
      public:
        enum BoundaryCondition {
            //! Make second(-last) point an inactive knot
            NotAKnot,
            //! Match value of end-slope
            FirstDerivative,
            //! Match value of second derivative at end
            SecondDerivative,
            //! Match first and second derivative at either end
            Periodic,
            /*! Match end-slope to the slope of the cubic that matches
                the first four data at the respective end
            */
            Lagrange
        };
      protected:
      private:
        boost::shared_ptr<detail::CoefficientHolder> coeffs_;
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CubicSpline(const I1& xBegin, const I1& xEnd, const I2& yBegin,
                    CubicSpline::BoundaryCondition leftCondition,
                    Real leftConditionValue,
                    CubicSpline::BoundaryCondition rightCondition,
                    Real rightConditionValue,
                    bool monotonicityConstraint) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                        new detail::CubicSplineImpl<I1,I2>(
                                          xBegin, xEnd, yBegin,
                                          leftCondition, leftConditionValue,
                                          rightCondition, rightConditionValue,
                                          monotonicityConstraint));
            coeffs_ =
                boost::dynamic_pointer_cast<detail::CoefficientHolder>(impl_);
        }
        const std::vector<Real>& aCoefficients() const {
            return coeffs_->a_;
        }
        const std::vector<Real>& bCoefficients() const {
            return coeffs_->b_;
        }
        const std::vector<Real>& cCoefficients() const {
            return coeffs_->c_;
        }
    };


    // convenience classes

    //! %Cubic spline with monotonicity constraint
    class MonotonicCubicSpline : public CubicSpline {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MonotonicCubicSpline(const I1& xBegin, const I1& xEnd,
                             const I2& yBegin,
                             CubicSpline::BoundaryCondition leftCondition,
                             Real leftConditionValue,
                             CubicSpline::BoundaryCondition rightCondition,
                             Real rightConditionValue)
        : CubicSpline(xBegin,xEnd,yBegin,
                      leftCondition,leftConditionValue,
                      rightCondition,rightConditionValue,
                      true) {}
    };

    //! %Cubic spline with null second derivative at end points
    class NaturalCubicSpline : public CubicSpline {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        NaturalCubicSpline(const I1& xBegin, const I1& xEnd,
                           const I2& yBegin)
        : CubicSpline(xBegin,xEnd,yBegin,
                      SecondDerivative, 0.0,
                      SecondDerivative, 0.0,
                      false) {}
    };

    //! Natural cubic spline with monotonicity constraint
    class NaturalMonotonicCubicSpline : public CubicSpline {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        NaturalMonotonicCubicSpline(const I1& xBegin, const I1& xEnd,
                                    const I2& yBegin)
        : CubicSpline(xBegin,xEnd,yBegin,
                      SecondDerivative, 0.0,
                      SecondDerivative, 0.0,
                      true) {}
    };

    //! %cubic-spline interpolation factory
    class Cubic {
      public:
        Cubic(CubicSpline::BoundaryCondition leftCondition
                                              = CubicSpline::SecondDerivative,
              Real leftConditionValue = 0.0,
              CubicSpline::BoundaryCondition rightCondition
                                              = CubicSpline::SecondDerivative,
              Real rightConditionValue = 0.0,
              bool monotonicityConstraint = false)
        : leftCondition_(leftCondition), leftValue_(leftConditionValue),
          rightCondition_(rightCondition), rightValue_(rightConditionValue),
          monotone_(monotonicityConstraint) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return CubicSpline(xBegin,xEnd,yBegin,
                               leftCondition_,leftValue_,
                               rightCondition_,rightValue_,
                               monotone_);
        }
      private:
        CubicSpline::BoundaryCondition leftCondition_;
        Real leftValue_;
        CubicSpline::BoundaryCondition rightCondition_;
        Real rightValue_;
        bool monotone_;
    };


    namespace detail {

        template <class I1, class I2>
        class CubicSplineImpl : public Interpolation::templateImpl<I1,I2>,
                                public CoefficientHolder {
          public:
            CubicSplineImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin,
                            CubicSpline::BoundaryCondition leftCondition,
                            Real leftConditionValue,
                            CubicSpline::BoundaryCondition rightCondition,
                            Real rightConditionValue,
                            bool monotonicityConstraint)
            : Interpolation::templateImpl<I1,I2>(xBegin,xEnd,yBegin),
              CoefficientHolder(xEnd-xBegin),
              monotone_(false), constrained_(monotonicityConstraint),
              leftType_(leftCondition), rightType_(rightCondition),
              leftValue_(leftConditionValue),
              rightValue_(rightConditionValue) {
                calculate();
            }

            void calculate() {

                TridiagonalOperator L(n_);
                Array tmp(n_);
                std::vector<Real> dx(n_-1), S(n_-1);

                Size i=0;
                dx[i] = this->xBegin_[i+1] - this->xBegin_[i];
                S[i] = (this->yBegin_[i+1] - this->yBegin_[i])/dx[i];
                for (i=1; i<n_-1; i++) {
                    dx[i] = this->xBegin_[i+1] - this->xBegin_[i];
                    S[i] = (this->yBegin_[i+1] - this->yBegin_[i])/dx[i];

                    L.setMidRow(i, dx[i], 2.0*(dx[i]+dx[i-1]), dx[i-1]);
                    tmp[i] = 3.0*(dx[i]*S[i-1] + dx[i-1]*S[i]);
                }

                /**** BOUNDARY CONDITIONS ****/

                // left condition
                switch (leftType_) {
                  case CubicSpline::NotAKnot:
                    // ignoring end condition value
                    L.setFirstRow(dx[1]*(dx[1]+dx[0]),
                                  (dx[0]+dx[1])*(dx[0]+dx[1]));
                    tmp[0] = S[0]*dx[1]*(2.0*dx[1]+3.0*dx[0]) +
                             S[1]*dx[0]*dx[0];
                    break;
                  case CubicSpline::FirstDerivative:
                    L.setFirstRow(1.0, 0.0);
                    tmp[0] = leftValue_;
                    break;
                  case CubicSpline::SecondDerivative:
                    L.setFirstRow(2.0, 1.0);
                    tmp[0] = 3.0*S[0] - leftValue_*dx[0]/2.0;
                    break;
                  case CubicSpline::Periodic:
                  case CubicSpline::Lagrange:
                    // ignoring end condition value
                    QL_FAIL("this end condition is not implemented yet");
                  default:
                    QL_FAIL("unknown end condition");
                }

                // right condition
                switch (rightType_) {
                  case CubicSpline::NotAKnot:
                    // ignoring end condition value
                    L.setLastRow(-(dx[n_-2]+dx[n_-3])*(dx[n_-2]+dx[n_-3]),
                                 -dx[n_-3]*(dx[n_-3]+dx[n_-2]));
                    tmp[n_-1] = -S[n_-3]*dx[n_-2]*dx[n_-2] -
                                 S[n_-2]*dx[n_-3]*(3.0*dx[n_-2]+2.0*dx[n_-3]);
                    break;
                  case CubicSpline::FirstDerivative:
                    L.setLastRow(0.0, 1.0);
                    tmp[n_-1] = rightValue_;
                    break;
                  case CubicSpline::SecondDerivative:
                    L.setLastRow(1.0, 2.0);
                    tmp[n_-1] = 3.0*S[n_-2] + rightValue_*dx[n_-2]/2.0;
                    break;
                  case CubicSpline::Periodic:
                  case CubicSpline::Lagrange:
                    // ignoring end condition value
                    QL_FAIL("this end condition is not implemented yet");
                  default:
                    QL_FAIL("unknown end condition");
                }

                // solve the system
                tmp = L.solveFor(tmp);

                if (constrained_) {
                    Real correction;
                    Real pm, pu, pd, M;
                    for (i=0; i<n_; i++) {
                        if (i==0) {
                            if (tmp[i]*S[0]>0.0) {
                                correction = tmp[i]/std::fabs(tmp[i]) *
                                    std::min<Real>(std::fabs(tmp[i]),
                                                   std::fabs(3.0*S[0]));
                            } else {
                                correction = 0.0;
                            }
                            if (correction!=tmp[i]) {
                                tmp[i] = correction;
                                monotone_ = true;
                            }
                        } else if (i==n_-1) {
                            if (tmp[i]*S[n_-2]>0.0) {
                                correction = tmp[i]/std::fabs(tmp[i]) *
                                    std::min<Real>(std::fabs(tmp[i]),
                                                   std::fabs(3.0*S[n_-2]));
                            } else {
                                correction = 0.0;
                            }
                            if (correction!=tmp[i]) {
                                tmp[i] = correction;
                                monotone_ = true;
                            }
                        } else {
                            pm=(S[i-1]*dx[i]+S[i]*dx[i-1])/
                                (dx[i-1]+dx[i]);
                            M = 3.0 * std::min(std::min(std::fabs(S[i-1]),
                                                        std::fabs(S[i])),
                                               std::fabs(pm));
                            if (i>1) {
                                if ((S[i-1]-S[i-2])*(S[i]-S[i-1])>0.0) {
                                    pd=(S[i-1]*(2.0*dx[i-1]+dx[i-2])
                                        -S[i-2]*dx[i-1])/
                                        (dx[i-2]+dx[i-1]);
                                    if (pm*pd>0.0 && pm*(S[i-1]-S[i-2])>0.0) {
                                        M = std::max<Real>(M, 1.5*std::min(
                                                std::fabs(pm),std::fabs(pd)));
                                    }
                                }
                            }
                            if (i<n_-2) {
                                if ((S[i]-S[i-1])*(S[i+1]-S[i])>0.0) {
                                    pu=(S[i]*(2.0*dx[i]+dx[i+1])-S[i+1]*dx[i])/
                                        (dx[i]+dx[i+1]);
                                    if (pm*pu>0.0 && -pm*(S[i]-S[i-1])>0.0) {
                                        M = std::max<Real>(M, 1.5*std::min(
                                                std::fabs(pm),std::fabs(pu)));
                                    }
                                }
                            }
                            if (tmp[i]*pm>0.0) {
                                correction = tmp[i]/std::fabs(tmp[i]) *
                                    std::min(std::fabs(tmp[i]), M);
                            } else {
                                correction = 0.0;
                            }
                            if (correction!=tmp[i]) {
                                tmp[i] = correction;
                                monotone_ = true;
                            }
                        }
                    }
                }

                for (i=0; i<n_-1; i++) {
                    a_[i] = tmp[i];
                    b_[i] = (3.0*S[i] - tmp[i+1] - 2.0*tmp[i])/dx[i];
                    c_[i] = (tmp[i+1] + tmp[i] - 2.0*S[i])/(dx[i]*dx[i]);
                }

                primitiveConst_[0] = 0.0;
                for (i=1; i<n_-1; i++) {
                    primitiveConst_[i] = primitiveConst_[i-1]
                        + dx[i-1] *
                        (this->yBegin_[i-1] + dx[i-1] *
                         (a_[i-1]/2.0 + dx[i-1] *
                          (b_[i-1]/3.0 + dx[i-1] * c_[i-1]/4.0)));
                }
            }
            Real value(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return this->yBegin_[j] + dx*(a_[j] + dx*(b_[j] + dx*c_[j]));
            }
            Real primitive(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return primitiveConst_[j]
                    + dx*(this->yBegin_[j] + dx*(a_[j]/2.0
                    + dx*(b_[j]/3.0 + dx*c_[j]/4.0)));
            }
            Real derivative(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return a_[j] + (2.0*b_[j] + 3.0*c_[j]*dx)*dx;
            }
            Real secondDerivative(Real x) const {
                Size j = this->locate(x);
                Real dx = x-this->xBegin_[j];
                return 2.0*b_[j] + 6.0*c_[j]*dx;
            }
          private:
            bool monotone_, constrained_;
            CubicSpline::BoundaryCondition leftType_, rightType_;
            Real leftValue_, rightValue_;
        };

    }

}


#endif
