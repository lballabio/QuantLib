
/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cubicspline.hpp
    \brief cubic spline interpolation between discrete points

    \fullpath
    ql/Math/%cubicspline.hpp
*/

// $Id$

#ifndef quantlib_cubic_spline_h
#define quantlib_cubic_spline_h

#include <ql/Math/interpolation.hpp>
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>
#include <vector>

namespace QuantLib {

    namespace Math {

        //! cubic spline interpolation between discrete points
        template <class RandomAccessIterator1, class RandomAccessIterator2>
        class CubicSpline
        : public Interpolation<RandomAccessIterator1,RandomAccessIterator2> {
          public:
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIterator1>::value_type
                argument_type;
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIterator2>::value_type
                result_type;
            CubicSpline(const RandomAccessIterator1& xBegin,
                const RandomAccessIterator1& xEnd,
                const RandomAccessIterator2& yBegin);
            result_type operator()(const argument_type& x,
                bool allowExtrapolation = false) const;
	    virtual ~CubicSpline() {}
          private:
            // P[i](x) = y[i] +
            //           a[i]*(x-x[i]) +
            //           b[i]*(x-x[i])^2 +
            //           c[i]*(x-x[i])^3
            std::vector<result_type> a_, b_, c_;
            // Numerical Recipes approch
            std::vector<result_type> d2y_;
            bool isNRapproach_;
        };


        // template definitions

        template <class I1, class I2>
        CubicSpline<I1,I2>::CubicSpline(const I1& xBegin, const I1& xEnd,
            const I2& yBegin)
        : Interpolation<I1,I2>(xBegin,xEnd,yBegin), a_(xEnd-xBegin-1),
          b_(xEnd-xBegin-1), c_(xEnd-xBegin-1), d2y_(xEnd-xBegin),
          isNRapproach_(true) {

            if (isNRapproach_) {
                // calculate cubic spline coefficients
                std::vector<result_type> u(n_-1);
                d2y_[0] = u[0] = result_type();
                I1 xi = xBegin_+1;
                I2 yi = yBegin_+1;
                int i;
                for (i=1; i<int(n_-1); i++,xi++,yi++) {
                    double sig = double(*xi-*(xi-1))/double(*(xi+1)-*(xi-1));
                    result_type p = sig*d2y_[i-1]+2.0;
                    d2y_[i] = (sig-1)/p;
                    u[i] = (*(yi+1)-*yi)/double(*(xi+1)-*xi) -
                        (*yi-*(yi-1))/double(*xi-*(xi-1));
                    u[i] = (6*u[i]/double(*(xi+1)-*(xi-1))-sig*u[i-1])/p;
                }

                d2y_[n_-1] = result_type();
                for (i=n_-2; i>=0; i--)
                    d2y_[i] = d2y_[i]*d2y_[i+1] + u[i];
            } else {
                QL_REQUIRE(n_ >= 4,
                    "CubicSpline::CubicSpline : "
                    "not enough points for cubic spline interpolation");
                FiniteDifferences::TridiagonalOperator L(n_);
                Array tmp(n_);

                argument_type dx01  = xBegin_[1] - xBegin_[0],
                              dx12  = xBegin_[2] - xBegin_[1],
                              dx02  = xBegin_[2] - xBegin_[0];
                result_type   dy01  = yBegin_[1] - yBegin_[0],
                              dy12  = yBegin_[2] - yBegin_[1];

                L.setFirstRow(dx01,dx02);
                tmp[0] = ((dx01+2.0*dx02)*dx12*dy01/dx01 +
                           dx01*dx01*dy12/dx12)/dx02;

                Size i;
                for (i=1; i<n_-1; i++) {
                    argument_type dxp = xBegin_[i+1] - xBegin_[i],
                                  dxm = xBegin_[i]   - xBegin_[i-1];
                    result_type   dyp = yBegin_[i+1] - yBegin_[i],
                                  dym = yBegin_[i]   - yBegin_[i-1];
                    L.setMidRow(i, dxp, 2.0*(dxp+dxm), dxm);
                    tmp[i] = 3.0*(dxp*dym/dxm + dxm*dyp/dxp);
                }

                argument_type dxN32 = xBegin_[n_-2] - xBegin_[n_-3],
                              dxN21 = xBegin_[n_-1] - xBegin_[n_-2],
                              dxN31 = xBegin_[n_-1] - xBegin_[n_-3];
                result_type   dyN32 = yBegin_[n_-2] - yBegin_[n_-3],
                              dyN21 = yBegin_[n_-1] - yBegin_[n_-2];

                L.setLastRow(dxN31,dxN21);
                tmp[n_-1] = ((dxN21+2.0*dxN31)*dxN32*dyN21/dxN21 +
                             dxN21*dxN21*dyN32/dxN32)/dxN31;

                tmp = L.solveFor(tmp);

                for (i=0; i<n_-1; i++) {
                    argument_type dx  = xBegin_[i+1]-xBegin_[i];
                    result_type   dy  = yBegin_[i+1]-yBegin_[i];

                    a_[i] = tmp[i];
                    c_[i] = (tmp[i] + tmp[i+1] - 2.0*dy/dx)/(dx*dx);
                    b_[i] = (dy/dx - tmp[i])/dx - c_[i]*dx;
                }
            }
        }

        template <class I1, class I2>
        typename CubicSpline<I1,I2>::result_type
        CubicSpline<I1,I2>::operator()(
            const CubicSpline<I1,I2>::argument_type& x,
            bool allowExtrapolation) const {
                locate(x);
                if (isOutOfRange_) {
                    QL_REQUIRE(allowExtrapolation,
                        "CubicSpline::operator() : "
                        "extrapolation not allowed");
                }

                if (isNRapproach_) {
                    I2 j = yBegin_+(position_-xBegin_);
                    std::vector<double>::const_iterator k =
                        d2y_.begin()+(position_-xBegin_);
                    double h = double(*(position_+1)-*position_);
                    double a = double(*(position_+1)-x)/h;
                    double b = 1.0-a;
                    return a*(*j) + b*(*(j+1)) +
                        ((a*a*a-a)*(*k)+(b*b*b-b)*(*(k+1)))*(h*h)/6.0;
                } else {
                    Size j = position_-xBegin_;
                    argument_type dx = x-xBegin_[j];
                    return yBegin_[j] + dx*(a_[j] + dx*(b_[j] + dx*c_[j]));
                }
        }

    }

}


#endif
