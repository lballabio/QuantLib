
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

#ifndef quantlib_cubic_spline_h
#define quantlib_cubic_spline_h

#include <ql/Math/interpolation.hpp>
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <vector>

namespace QuantLib {

    //! cubic spline interpolation between discrete points
    template <class RandomAccessIterator1, class RandomAccessIterator2>
    class CubicSplineInterpolation
        : public Interpolation<RandomAccessIterator1,RandomAccessIterator2> {
      public:
        typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIterator1>::value_type
                                                                argument_type;
        typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIterator2>::value_type
                                                                  result_type;
        CubicSplineInterpolation(const RandomAccessIterator1& xBegin,
                                 const RandomAccessIterator1& xEnd,
                                 const RandomAccessIterator2& yBegin,
                                 result_type y1a,
                                 result_type y2a,
                                 result_type y1b,
                                 result_type y2b,
                                 bool monotonicityConstraint);
        result_type operator()(const argument_type& x,
                               bool allowExtrapolation = false) const;
        result_type primitive(const argument_type& x,
                              bool allowExtrapolation = false) const;
        result_type derivative(const argument_type& x,
                               bool allowExtrapolation = false) const;
        result_type secondDerivative(const argument_type& x,
                                     bool allowExtrapolation = false) const;
        const std::vector<result_type>& aCoefficients() const { return a_; };
        const std::vector<result_type>& bCoefficients() const { return b_; };
        const std::vector<result_type>& cCoefficients() const { return c_; };
      private:
        // P[i](x) = y[i] +
        //           a[i]*(x-x[i]) +
        //           b[i]*(x-x[i])^2 +
        //           c[i]*(x-x[i])^3
        std::vector<result_type> primitiveConst_, a_, b_, c_;
        bool monotonicityCorrectionApplied_;
    };


    // template definitions

    template <class I1, class I2>
    CubicSplineInterpolation<I1,I2>::CubicSplineInterpolation(
        const I1& xBegin, const I1& xEnd, const I2& yBegin,
        result_type y1a, result_type y2a, result_type y1b, result_type y2b,
        bool monotonicityConstraint)
    : Interpolation<I1,I2>(xBegin,xEnd,yBegin), primitiveConst_(xEnd-xBegin-1),
      a_(xEnd-xBegin-1), b_(xEnd-xBegin-1), c_(xEnd-xBegin-1),
      monotonicityCorrectionApplied_(false) {

        TridiagonalOperator L(n_);
        Array tmp(n_);
        std::vector<double> dx(n_-1), S(n_-1);

        Size i=0;
        dx[i]= xBegin_[i+1] - xBegin_[i];
        S[i] =(yBegin_[i+1] - yBegin_[i])/dx[i];
        for (i=1; i<n_-1; i++) {
            dx[i]= xBegin_[i+1] - xBegin_[i];
            S[i] =(yBegin_[i+1] - yBegin_[i])/dx[i];

            L.setMidRow(i, dx[i], 2.0*(dx[i]+dx[i-1]), dx[i-1]);
            tmp[i] = 3.0*(dx[i]*S[i-1] + dx[i-1]*S[i]);
        }

        /**** BOUNDARY CONDITIONS ****/

        // left condition
        if (y1a!=Null<double>()) {
            // first derivative value
            L.setFirstRow(1.0, 0.0);
            tmp[0] = y1a;
        } else if (y2a!=Null<double>()) {
            // 2nd derivative value
            L.setFirstRow(2.0, 1.0);
            tmp[0] = 3.0*S[0] - y2a*dx[0]/2.0;
        } else {
            // not-a-knot
            L.setFirstRow(dx[1]*(dx[1]+dx[0]), (dx[0]+dx[1])*(dx[0]+dx[1]));
            tmp[0] = S[0]*dx[1]*(2.0*dx[1]+3.0*dx[0]) + S[1]*dx[0]*dx[0];
        }


        // right condition
        if (y1b!=Null<double>()) {
            // first derivative value
            L.setLastRow(0.0, 1.0);
            tmp[n_-1] = y1b;
        } else if (y2b!=Null<double>()) {
            // 2nd derivative value
            L.setLastRow(1.0, 2.0);
            tmp[n_-1] = 3.0*S[n_-2] - y2b*dx[n_-2]/2.0;
        } else {
            // not-a-knot
            L.setLastRow(-(dx[n_-2]+dx[n_-3])*(dx[n_-2]+dx[n_-3]),
                         -dx[n_-3]*(dx[n_-3]+dx[n_-2]));
            tmp[n_-1] = -S[n_-3]*dx[n_-2]*dx[n_-2] -
                S[n_-2]*dx[n_-3]*(3.0*dx[n_-2]+2.0*dx[n_-3]);
        }


        // solve the system
        tmp = L.solveFor(tmp);

        if (monotonicityConstraint) {
            double constraint, correction;
            for (i=0; i<n_; i++) {
                if (i==0) {
                    // locally monotone constrain
                    constraint = 3.0 * S[0];
                    if (constraint>=0.0) {
                        correction = QL_MIN(QL_MAX(0.0, tmp[i]), constraint);
                        if (correction!=tmp[i]) {
                            tmp[i] = correction;
                            monotonicityCorrectionApplied_ = true;
                        }
                    } else {
                        correction = QL_MAX(QL_MIN(0.0, tmp[i]), constraint);
                        if (correction!=tmp[i]) {
                            tmp[i] = correction;
                            monotonicityCorrectionApplied_ = true;
                        }
                    }
                } else if (i==n_-1) {
                    // locally monotone constrain
                    constraint = 3.0 * S[n_-2];
                    if (constraint>=0.0) {
                        correction = QL_MIN(QL_MAX(0.0, tmp[i]), constraint);
                        if (correction!=tmp[i]) {
                            tmp[i] = correction;
                            monotonicityCorrectionApplied_ = true;
                        }
                    } else {
                        correction = QL_MAX(QL_MIN(0.0, tmp[i]), constraint);
                        if (correction!=tmp[i]) {
                            tmp[i] = correction;
                            monotonicityCorrectionApplied_ = true;
                        }
                    }
                } else {
                    // locally non-monotone (generalized) constrain
                    constraint = 3.0 * QL_MIN(QL_FABS(S[i-1]),
                                              QL_FABS(S[i]));
                    if (tmp[i]>=0.0) {
                        correction = QL_MIN(QL_MAX(0.0, tmp[i]), constraint);
                        if (correction!=tmp[i]) {
                            tmp[i] = correction;
                            monotonicityCorrectionApplied_ = true;
                        }
                    } else {
                        correction = QL_MAX(QL_MIN(0.0, tmp[i]),-constraint);
                        if (correction!=tmp[i]) {
                            tmp[i] = correction;
                            monotonicityCorrectionApplied_ = true;
                        }
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
                              + dx[i-1] *(yBegin_[i-1]
                              + dx[i-1] *(a_[i-1]/2.0
                              + dx[i-1] *(b_[i-1]/3.0
                              + dx[i-1] * c_[i-1]/4.0)));
        }
    }

    template <class I1, class I2>
    typename CubicSplineInterpolation<I1,I2>::result_type
    CubicSplineInterpolation<I1,I2>::operator()(
             const typename CubicSplineInterpolation<I1,I2>::argument_type& x,
             bool allowExtrapolation) const {

        locate(x);
        if (isOutOfRange_) {
            QL_REQUIRE(allowExtrapolation,
                       "CubicSplineInterpolation::operator() : "
                       "\ninterpolation range is ["
                       + DoubleFormatter::toString(xBegin_[0]) +
                       ", "
                       + DoubleFormatter::toString(xBegin_[n_-1]) +
                       "]: extrapolation at "
                       + DoubleFormatter::toString(x) +
                       " not allowed");
        }

        Size j = position_-xBegin_;
        argument_type dx = x-xBegin_[j];
        return yBegin_[j] + dx*(a_[j] + dx*(b_[j] + dx*c_[j]));
    }

    template <class I1, class I2>
    typename CubicSplineInterpolation<I1,I2>::result_type
    CubicSplineInterpolation<I1,I2>::primitive(
             const typename CubicSplineInterpolation<I1,I2>::argument_type& x,
             bool allowExtrapolation) const {

        locate(x);
        if (isOutOfRange_) {
            QL_REQUIRE(allowExtrapolation,
                       "CubicSplineInterpolation::primitive : "
                       "\ninterpolation range is ["
                       + DoubleFormatter::toString(xBegin_[0]) +
                       ", "
                       + DoubleFormatter::toString(xBegin_[n_-1]) +
                       "]: extrapolation at "
                       + DoubleFormatter::toString(x) +
                       " not allowed");
        }

        Size j = position_-xBegin_;
        argument_type dx = x-xBegin_[j];
        return primitiveConst_[j] + dx*(yBegin_[j] + dx*(a_[j]/2.0 
                                  + dx*(b_[j]/3.0 + dx*c_[j]/4.0)));
    }

    template <class I1, class I2>
    typename CubicSplineInterpolation<I1,I2>::result_type
    CubicSplineInterpolation<I1,I2>::derivative(
             const typename CubicSplineInterpolation<I1,I2>::argument_type& x,
             bool allowExtrapolation) const {

        locate(x);
        if (isOutOfRange_) {
            QL_REQUIRE(allowExtrapolation,
                       "CubicSplineInterpolation::derivative() : "
                       "\ninterpolation range is ["
                       + DoubleFormatter::toString(xBegin_[0]) +
                       ", "
                       + DoubleFormatter::toString(xBegin_[n_-1]) +
                       "]: extrapolation at "
                       + DoubleFormatter::toString(x) +
                       " not allowed");
        }

        Size j = position_-xBegin_;
        argument_type dx = x-xBegin_[j];
        return a_[j] + (2.0*b_[j] + 3.0*c_[j]*dx)*dx;
    }

    template <class I1, class I2>
    typename CubicSplineInterpolation<I1,I2>::result_type
    CubicSplineInterpolation<I1,I2>::secondDerivative(
             const typename CubicSplineInterpolation<I1,I2>::argument_type& x,
             bool allowExtrapolation) const {

        locate(x);
        if (isOutOfRange_) {
            QL_REQUIRE(allowExtrapolation,
                       "CubicSplineInterpolation::secondDerivative() : "
                       "\ninterpolation range is ["
                       + DoubleFormatter::toString(xBegin_[0]) +
                       ", "
                       + DoubleFormatter::toString(xBegin_[n_-1]) +
                       "]: extrapolation at "
                       + DoubleFormatter::toString(x) +
                       " not allowed");
        }

        Size j = position_-xBegin_;
        argument_type dx = x-xBegin_[j];
        return 2.0*b_[j] + 6.0*c_[j]*dx;
    }

}


#endif
