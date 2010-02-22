/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2004, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2009 Sylvain Bertrand

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

/*! \file cubicinterpolation.hpp
    \brief cubic interpolation between discrete points
*/

#ifndef quantlib_cubic_interpolation_hpp
#define quantlib_cubic_interpolation_hpp

#include <ql/math/matrix.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <vector>

namespace QuantLib {

    namespace detail {

        class CoefficientHolder {
          public:
            CoefficientHolder(Size n)
            : n_(n), primitiveConst_(n-1), a_(n-1), b_(n-1), c_(n-1),
              monotonicityAdjustments_(n) {}
            virtual ~CoefficientHolder() {}
            Size n_;
            // P[i](x) = y[i] +
            //           a[i]*(x-x[i]) +
            //           b[i]*(x-x[i])^2 +
            //           c[i]*(x-x[i])^3
            std::vector<Real> primitiveConst_, a_, b_, c_;
            std::vector<bool> monotonicityAdjustments_;
        };

        template <class I1, class I2> class CubicInterpolationImpl;

    }

    //! %Cubic interpolation between discrete points.
    /*! Cubic interpolation is fully defined when the ${f_i}$ function values
        at points ${x_i}$ are supplemented with ${f^'_i}$ function derivative
        values.

        Different type of first derivative approximations are implemented,
        both local and non-local. Local schemes (Fourth-order, Parabolic,
        Modified Parabolic, Fritsch-Butland, Akima, Kruger) use only $f$ values
        near $x_i$ to calculate each $f^'_i$. Non-local schemes (Spline with
        different boundary conditions) use all ${f_i}$ values and obtain
        ${f^'_i}$ by solving a linear system of equations. Local schemes
        produce $C^1$ interpolants, while the spline schemes generate $C^2$
        interpolants.

        Hyman's monotonicity constraint filter is also implemented: it can be
        applied to all schemes to ensure that in the regions of local
        monotoniticity of the input (three successive increasing or decreasing
        values) the interpolating cubic remains monotonic. If the interpolating
        cubic is already monotonic, the Hyman filter leaves it unchanged
        preserving all its original features.

        In the case of $C^2$ interpolants the Hyman filter ensures local
        monotonicity at the expense of the second derivative of the interpolant
        which will no longer be continuous in the points where the filter has
        been applied.

        While some non-linear schemes (Modified Parabolic, Fritsch-Butland,
        Kruger) are guaranteed to be locally monotone in their original
        approximation, all other schemes must be filtered according to the
        Hyman criteria at the expense of their linearity.

        See R. L. Dougherty, A. Edelman, and J. M. Hyman,
        "Nonnegativity-, Monotonicity-, or Convexity-Preserving CubicSpline and
        Quintic Hermite Interpolation"
        Mathematics Of Computation, v. 52, n. 186, April 1989, pp. 471-494.

        \todo implement missing schemes (FourthOrder and ModifiedParabolic) and
              missing boundary conditions (Periodic and Lagrange).

        \test to be adapted from old ones.

    */
    class CubicInterpolation : public Interpolation {
      public:
        enum DerivativeApprox {
            /*! Spline approximation (non-local, non-monotone, linear[?]).
                Different boundary conditions can be used on the left and right
                boundaries: see BoundaryCondition.
            */
            Spline,

            //! Overshooting minimization 1st derivative
            SplineOM1,

            //! Overshooting minimization 2nd derivative
            SplineOM2,

            //! Fourth-order approximation (local, non-monotone, linear)
            FourthOrder,

            //! Parabolic approximation (local, non-monotone, linear)
            Parabolic,

            //! Fritsch-Butland approximation (local, monotone, non-linear)
            FritschButland,

            //! Akima approximation (local, non-monotone, non-linear)
            Akima,

            //! Kruger approximation (local, monotone, non-linear)
            Kruger
        };
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
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CubicInterpolation(const I1& xBegin,
                           const I1& xEnd,
                           const I2& yBegin,
                           CubicInterpolation::DerivativeApprox da,
                           bool monotonic,
                           CubicInterpolation::BoundaryCondition leftCond,
                           Real leftConditionValue,
                           CubicInterpolation::BoundaryCondition rightCond,
                           Real rightConditionValue) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::CubicInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                      da,
                                                      monotonic,
                                                      leftCond,
                                                      leftConditionValue,
                                                      rightCond,
                                                      rightConditionValue));
            impl_->update();
            coeffs_ =
                boost::dynamic_pointer_cast<detail::CoefficientHolder>(impl_);
        }
        const std::vector<Real>& primitiveConstants() const {
            return coeffs_->primitiveConst_;
        }
        const std::vector<Real>& aCoefficients() const { return coeffs_->a_; }
        const std::vector<Real>& bCoefficients() const { return coeffs_->b_; }
        const std::vector<Real>& cCoefficients() const { return coeffs_->c_; }
        const std::vector<bool>& monotonicityAdjustments() const {
            return coeffs_->monotonicityAdjustments_;
        }
      private:
        boost::shared_ptr<detail::CoefficientHolder> coeffs_;
    };


    // convenience classes

    class CubicNaturalSpline : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CubicNaturalSpline(const I1& xBegin,
                           const I1& xEnd,
                           const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             Spline, false,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class MonotonicCubicNaturalSpline : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MonotonicCubicNaturalSpline(const I1& xBegin,
                                    const I1& xEnd,
                                    const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             Spline, true,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class CubicSplineOvershootingMinimization1 : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CubicSplineOvershootingMinimization1 (const I1& xBegin,
                                           const I1& xEnd,
                                           const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             SplineOM1, false,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class CubicSplineOvershootingMinimization2 : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        CubicSplineOvershootingMinimization2 (const I1& xBegin,
                                           const I1& xEnd,
                                           const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             SplineOM2, false,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class AkimaCubicInterpolation : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
    template <class I1, class I2>
    AkimaCubicInterpolation(const I1& xBegin,
                const I1& xEnd,
                const I2& yBegin)
    : CubicInterpolation(xBegin, xEnd, yBegin,
                 Akima, false,
                 SecondDerivative, 0.0,
                 SecondDerivative, 0.0) {}
    };

    class KrugerCubic : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        KrugerCubic(const I1& xBegin,
                    const I1& xEnd,
                    const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             Kruger, false,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class FritschButlandCubic : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        FritschButlandCubic(const I1& xBegin,
                            const I1& xEnd,
                            const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             FritschButland, false,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class Parabolic : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        Parabolic(const I1& xBegin,
                  const I1& xEnd,
                  const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             CubicInterpolation::Parabolic, false,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    class MonotonicParabolic : public CubicInterpolation {
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        MonotonicParabolic(const I1& xBegin,
                           const I1& xEnd,
                           const I2& yBegin)
        : CubicInterpolation(xBegin, xEnd, yBegin,
                             Parabolic, true,
                             SecondDerivative, 0.0,
                             SecondDerivative, 0.0) {}
    };

    //! %Cubic interpolation factory and traits
    class Cubic {
      public:
        Cubic(CubicInterpolation::DerivativeApprox da
                  = CubicInterpolation::Kruger,
              bool monotonic = false,
              CubicInterpolation::BoundaryCondition leftCondition
                  = CubicInterpolation::SecondDerivative,
              Real leftConditionValue = 0.0,
              CubicInterpolation::BoundaryCondition rightCondition
                  = CubicInterpolation::SecondDerivative,
              Real rightConditionValue = 0.0)
        : da_(da), monotonic_(monotonic),
          leftType_(leftCondition), rightType_(rightCondition),
          leftValue_(leftConditionValue), rightValue_(rightConditionValue) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin,
                                  const I1& xEnd,
                                  const I2& yBegin) const {
            return CubicInterpolation(xBegin, xEnd, yBegin,
                                      da_, monotonic_,
                                      leftType_, leftValue_,
                                      rightType_, rightValue_);
        }
        static const bool global = true;
        static const Size requiredPoints = 2;
      private:
        CubicInterpolation::DerivativeApprox da_;
        bool monotonic_;
        CubicInterpolation::BoundaryCondition leftType_, rightType_;
        Real leftValue_, rightValue_;
    };


    namespace detail {

        template <class I1, class I2>
        class CubicInterpolationImpl : public CoefficientHolder,
                                    public Interpolation::templateImpl<I1,I2> {
          public:
            CubicInterpolationImpl(const I1& xBegin,
                                   const I1& xEnd,
                                   const I2& yBegin,
                                   CubicInterpolation::DerivativeApprox da,
                                   bool monotonic,
                                   CubicInterpolation::BoundaryCondition leftCondition,
                                   Real leftConditionValue,
                                   CubicInterpolation::BoundaryCondition rightCondition,
                                   Real rightConditionValue)
            : CoefficientHolder(xEnd-xBegin),
              Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              da_(da),
              monotonic_(monotonic),
              leftType_(leftCondition), rightType_(rightCondition),
              leftValue_(leftConditionValue),
              rightValue_(rightConditionValue) {}

            void update() {

                Array tmp(n_);
                std::vector<Real> dx(n_-1), S(n_-1);

                for (Size i=0; i<n_-1; ++i) {
                    dx[i] = this->xBegin_[i+1] - this->xBegin_[i];
                    S[i] = (this->yBegin_[i+1] - this->yBegin_[i])/dx[i];
                }

                // first derivative approximation
                if (da_==CubicInterpolation::Spline) {
                    TridiagonalOperator L(n_);
                    for (Size i=1; i<n_-1; ++i) {
                        L.setMidRow(i, dx[i], 2.0*(dx[i]+dx[i-1]), dx[i-1]);
                        tmp[i] = 3.0*(dx[i]*S[i-1] + dx[i-1]*S[i]);
                    }

                    // left boundary condition
                    switch (leftType_) {
                      case CubicInterpolation::NotAKnot:
                        // ignoring end condition value
                        L.setFirstRow(dx[1]*(dx[1]+dx[0]),
                                      (dx[0]+dx[1])*(dx[0]+dx[1]));
                        tmp[0] = S[0]*dx[1]*(2.0*dx[1]+3.0*dx[0]) +
                                 S[1]*dx[0]*dx[0];
                        break;
                      case CubicInterpolation::FirstDerivative:
                        L.setFirstRow(1.0, 0.0);
                        tmp[0] = leftValue_;
                        break;
                      case CubicInterpolation::SecondDerivative:
                        L.setFirstRow(2.0, 1.0);
                        tmp[0] = 3.0*S[0] - leftValue_*dx[0]/2.0;
                        break;
                      case CubicInterpolation::Periodic:
                      case CubicInterpolation::Lagrange:
                        // ignoring end condition value
                        QL_FAIL("this end condition is not implemented yet");
                      default:
                        QL_FAIL("unknown end condition");
                    }

                    // right boundary condition
                    switch (rightType_) {
                      case CubicInterpolation::NotAKnot:
                        // ignoring end condition value
                        L.setLastRow(-(dx[n_-2]+dx[n_-3])*(dx[n_-2]+dx[n_-3]),
                                     -dx[n_-3]*(dx[n_-3]+dx[n_-2]));
                        tmp[n_-1] = -S[n_-3]*dx[n_-2]*dx[n_-2] -
                                     S[n_-2]*dx[n_-3]*(3.0*dx[n_-2]+2.0*dx[n_-3]);
                        break;
                      case CubicInterpolation::FirstDerivative:
                        L.setLastRow(0.0, 1.0);
                        tmp[n_-1] = rightValue_;
                        break;
                      case CubicInterpolation::SecondDerivative:
                        L.setLastRow(1.0, 2.0);
                        tmp[n_-1] = 3.0*S[n_-2] + rightValue_*dx[n_-2]/2.0;
                        break;
                      case CubicInterpolation::Periodic:
                      case CubicInterpolation::Lagrange:
                        // ignoring end condition value
                        QL_FAIL("this end condition is not implemented yet");
                      default:
                        QL_FAIL("unknown end condition");
                    }

                    // solve the system
                    tmp = L.solveFor(tmp);
                } else if (da_==CubicInterpolation::SplineOM1) {
                    Matrix T_,S_,Up_,Us_,Z_,I_,V_,W_,Q_,J_;
                    T_=Matrix(n_-2,n_,0.0);
                    for (Size i=0; i<n_-2; ++i) {
                        T_[i][i]=dx[i]/6.0;
                        T_[i][i+1]=(dx[i+1]+dx[i])/3.0;
                        T_[i][i+2]=dx[i+1]/6.0;
                    }
                    S_=Matrix(n_-2,n_,0.0);
                    for (Size i=0; i<n_-2; ++i) {
                        S_[i][i]=1.0/dx[i];
                        S_[i][i+1]=-(1.0/dx[i+1]+1.0/dx[i]);
                        S_[i][i+2]=1.0/dx[i+1];
                    }
                    Up_=Matrix(n_,2,0.0);
                    Up_[0][0]=1;
                    Up_[n_-1][1]=1;
                    Us_=Matrix(n_,n_-2,0.0);
                    for (Size i=0; i<n_-2; ++i) {
                        Us_[i+1][i]=1;
                    }
                    Z_=Matrix(n_,n_-2);
                    Z_=Us_*inverse(T_*Us_);
                    I_=Matrix(n_,n_,0.0);
                    for (Size i=0; i<n_; ++i) {
                        I_[i][i]=1;
                    }
                    V_=Matrix(n_,2);
                    V_=(I_-Z_*T_)*Up_;
                    W_=Matrix(n_,n_);
                    W_=Z_*S_;
                    Q_=Matrix(n_,n_,0.0);
                    Q_[0][0]=1.0/(n_-1)*dx[0]*dx[0]*dx[0];
                    Q_[0][1]=7.0/8*1.0/(n_-1)*dx[0]*dx[0]*dx[0];
                    for (Size i=1; i<n_-1; ++i) {
                        Q_[i][i-1]=7.0/8*1.0/(n_-1)*dx[i-1]*dx[i-1]*dx[i-1];
                        Q_[i][i]=1.0/(n_-1)*dx[i]*dx[i]*dx[i]+1.0/(n_-1)*dx[i-1]*dx[i-1]*dx[i-1];
                        Q_[i][i+1]=7.0/8*1.0/(n_-1)*dx[i]*dx[i]*dx[i];
                    }
                    Q_[n_-1][n_-2]=7.0/8*1.0/(n_-1)*dx[n_-2]*dx[n_-2]*dx[n_-2];
                    Q_[n_-1][n_-1]=1.0/(n_-1)*dx[n_-2]*dx[n_-2]*dx[n_-2];
                    J_=Matrix(n_,n_);
                    J_=(I_-V_*inverse(transpose(V_)*Q_*V_)*transpose(V_)*Q_)*W_;
                    Array Y_(n_);
                    Array D_(n_);
                    for (Size i=0; i<n_; ++i) {
                        Y_[i]=this->yBegin_[i];
                    }
                    D_=J_*Y_;
                    for (Size i=0; i<n_-1; ++i) {
                        tmp[i]=(Y_[i+1]-Y_[i])/dx[i]-(2.0*D_[i]+D_[i+1])*dx[i]/6.0;
                    }
                    tmp[n_-1]=tmp[n_-2]+D_[n_-2]*dx[n_-2]+(D_[n_-1]-D_[n_-2])*dx[n_-2]/2.0;

                } else if (da_==CubicInterpolation::SplineOM2) {
                    Matrix T_,S_,Up_,Us_,Z_,I_,V_,W_,Q_,J_;
                    T_=Matrix(n_-2,n_,0.0);
                    for (Size i=0; i<n_-2; ++i) {
                        T_[i][i]=dx[i]/6.0;
                        T_[i][i+1]=(dx[i]+dx[i+1])/3.0;
                        T_[i][i+2]=dx[i+1]/6.0;
                    }
                    S_=Matrix(n_-2,n_,0.0);
                    for (Size i=0; i<n_-2; ++i) {
                        S_[i][i]=1.0/dx[i];
                        S_[i][i+1]=-(1.0/dx[i+1]+1.0/dx[i]);
                        S_[i][i+2]=1.0/dx[i+1];
                    }
                    Up_=Matrix(n_,2,0.0);
                    Up_[0][0]=1;
                    Up_[n_-1][1]=1;
                    Us_=Matrix(n_,n_-2,0.0);
                    for (Size i=0; i<n_-2; ++i) {
                        Us_[i+1][i]=1;
                    }
                    Z_=Matrix(n_,n_-2);
                    Z_=Us_*inverse(T_*Us_);
                    I_=Matrix(n_,n_,0.0);
                    for (Size i=0; i<n_; ++i) {
                        I_[i][i]=1;
                    }
                    V_=Matrix(n_,2);
                    V_=(I_-Z_*T_)*Up_;
                    W_=Matrix(n_,n_);
                    W_=Z_*S_;
                    Q_=Matrix(n_,n_,0.0);
                    Q_[0][0]=1.0/(n_-1)*dx[0];
                    Q_[0][1]=1.0/2*1.0/(n_-1)*dx[0];
                    for (Size i=1; i<n_-1; ++i) {
                        Q_[i][i-1]=1.0/2*1.0/(n_-1)*dx[i-1];
                        Q_[i][i]=1.0/(n_-1)*dx[i]+1.0/(n_-1)*dx[i-1];
                        Q_[i][i+1]=1.0/2*1.0/(n_-1)*dx[i];
                    }
                    Q_[n_-1][n_-2]=1.0/2*1.0/(n_-1)*dx[n_-2];
                    Q_[n_-1][n_-1]=1.0/(n_-1)*dx[n_-2];
                    J_=Matrix(n_,n_);
                    J_=(I_-V_*inverse(transpose(V_)*Q_*V_)*transpose(V_)*Q_)*W_;
                    Array Y_(n_);
                    Array D_(n_);
                    for (Size i=0; i<n_; ++i) {
                        Y_[i]=this->yBegin_[i];
                    }
                    D_=J_*Y_;
                    for (Size i=0; i<n_-1; ++i) {
                        tmp[i]=(Y_[i+1]-Y_[i])/dx[i]-(2.0*D_[i]+D_[i+1])*dx[i]/6.0;
                    }
                    tmp[n_-1]=tmp[n_-2]+D_[n_-2]*dx[n_-2]+(D_[n_-1]-D_[n_-2])*dx[n_-2]/2.0;
                } else { // local schemes
                    if (n_==2)
                        tmp[0] = tmp[1] = S[0];
                    else {
                        switch (da_) {
                            case CubicInterpolation::FourthOrder:
                                QL_FAIL("FourthOrder not implemented yet");
                                break;
                            case CubicInterpolation::Parabolic:
                                // intermediate points
                                for (Size i=1; i<n_-1; ++i) {
                                    tmp[i] = (dx[i-1]*S[i]+dx[i]*S[i-1])/(dx[i]+dx[i-1]);
                                }
                                // end points
                                tmp[0]    = ((2.0*dx[   0]+dx[   1])*S[   0] - dx[   0]*S[   1]) / (dx[   0]+dx[   1]);
                                tmp[n_-1] = ((2.0*dx[n_-2]+dx[n_-3])*S[n_-2] - dx[n_-2]*S[n_-3]) / (dx[n_-2]+dx[n_-3]);
                                break;
                            case CubicInterpolation::FritschButland:
                                // intermediate points
                                for (Size i=1; i<n_-1; ++i) {
                                    Real Smin = std::min(S[i-1], S[i]);
                                    Real Smax = std::max(S[i-1], S[i]);
                                    tmp[i] = 3.0*Smin*Smax/(Smax+2.0*Smin);
                                }
                                // end points
                                tmp[0]    = ((2.0*dx[   0]+dx[   1])*S[   0] - dx[   0]*S[   1]) / (dx[   0]+dx[   1]);
                                tmp[n_-1] = ((2.0*dx[n_-2]+dx[n_-3])*S[n_-2] - dx[n_-2]*S[n_-3]) / (dx[n_-2]+dx[n_-3]);
                                break;
                            case CubicInterpolation::Akima:
                                tmp[0] = (std::abs(S[1]-S[0])*2*S[0]*S[1]+std::abs(2*S[0]*S[1]-4*S[0]*S[0]*S[1])*S[0])/(std::abs(S[1]-S[0])+std::abs(2*S[0]*S[1]-4*S[0]*S[0]*S[1]));
                                tmp[1] = (std::abs(S[2]-S[1])*S[0]+std::abs(S[0]-2*S[0]*S[1])*S[1])/(std::abs(S[2]-S[1])+std::abs(S[0]-2*S[0]*S[1]));
                                for (Size i=2; i<n_-2; ++i) {
                                    if ((S[i-2]==S[i-1]) && (S[i]!=S[i+1])) {
                                        tmp[i] = S[i-1];
                                    } else if ((S[i-2]!=S[i-1]) && (S[i]==S[i+1])) {
                                        tmp[i] = S[i];
                                    } else if (S[i]==S[i-1]) {
                                        tmp[i] = S[i];
                                    } else if ((S[i-2]==S[i-1]) && (S[i-1]!=S[i]) && (S[i]==S[i+1])) {
                                        tmp[i] = (S[i-1]+S[i])/2.0;
                                    } else {
                                        tmp[i] = (std::abs(S[i+1]-S[i])*S[i-1]+std::abs(S[i-1]-S[i-2])*S[i])/(std::abs(S[i+1]-S[i])+std::abs(S[i-1]-S[i-2]));
                                    }
                                 }
                                 tmp[n_-2] = (std::abs(2*S[n_-2]*S[n_-3]-S[n_-2])*S[n_-3]+std::abs(S[n_-3]-S[n_-4])*S[n_-2])/(std::abs(2*S[n_-2]*S[n_-3]-S[n_-2])+std::abs(S[n_-3]-S[n_-4]));
                                 tmp[n_-1] = (std::abs(4*S[n_-2]*S[n_-2]*S[n_-3]-2*S[n_-2]*S[n_-3])*S[n_-2]+std::abs(S[n_-2]-S[n_-3])*2*S[n_-2]*S[n_-3])/(std::abs(4*S[n_-2]*S[n_-2]*S[n_-3]-2*S[n_-2]*S[n_-3])+std::abs(S[n_-2]-S[n_-3]));
                                 break;
                            case CubicInterpolation::Kruger:
                                // intermediate points
                                for (Size i=1; i<n_-1; ++i) {
                                    if (S[i-1]*S[i]<0.0)
                                        // slope changes sign at point
                                        tmp[i] = 0.0;
                                    else
                                        // slope will be between the slopes of the adjacent
                                        // straight lines and should approach zero if the
                                        // slope of either line approaches zero
                                        tmp[i] = 2.0/(1.0/S[i-1]+1.0/S[i]);
                                }
                                // end points
                                tmp[0] = (3.0*S[0]-tmp[1])/2.0;
                                tmp[n_-1] = (3.0*S[n_-2]-tmp[n_-2])/2.0;
                                break;
                            default:
                                QL_FAIL("unknown scheme");
                        }
                    }
                }

                std::fill(monotonicityAdjustments_.begin(),
                          monotonicityAdjustments_.end(), false);
                // Hyman monotonicity constrained filter
                if (monotonic_) {
                    Real correction;
                    Real pm, pu, pd, M;
                    for (Size i=0; i<n_; ++i) {
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
                                monotonicityAdjustments_[i] = true;
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
                                monotonicityAdjustments_[i] = true;
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
                                monotonicityAdjustments_[i] = true;
                            }
                        }
                    }
                }


                // cubic coefficients
                for (Size i=0; i<n_-1; ++i) {
                    a_[i] = tmp[i];
                    b_[i] = (3.0*S[i] - tmp[i+1] - 2.0*tmp[i])/dx[i];
                    c_[i] = (tmp[i+1] + tmp[i] - 2.0*S[i])/(dx[i]*dx[i]);
                }

                primitiveConst_[0] = 0.0;
                for (Size i=1; i<n_-1; ++i) {
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
            CubicInterpolation::DerivativeApprox da_;
            bool monotonic_;
            CubicInterpolation::BoundaryCondition leftType_, rightType_;
            Real leftValue_, rightValue_;
        };

    }

}

#endif
