
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
            result_type operator()(const argument_type& x) const;
	    virtual ~CubicSpline() {}
          private:
            // P[i](x) = y[i] + 
            //           a[i]*(x-x[i]) + 
            //           b[i]*(x-x[i])^2 + 
            //           c[i]*(x-x[i])^3
            std::vector<result_type> a_, b_, c_;
        };


        // template definitions

        template <class I1, class I2>
        CubicSpline<I1,I2>::CubicSpline(const I1& xBegin, const I1& xEnd,
            const I2& yBegin)
        : Interpolation<I1,I2>(xBegin,xEnd,yBegin), a_(xEnd-xBegin-1),
          b_(xEnd-xBegin-1), c_(xEnd-xBegin-1) {
            size_t n = xEnd_-xBegin_;
            #ifdef QL_DEBUG
                QL_REQUIRE(n >= 4,
                    "not enough points for cubic spline interpolation");
            #endif
            FiniteDifferences::TridiagonalOperator L(n);
            Array tmp(n);

            argument_type dx01  = xBegin_[1] - xBegin_[0],
                          dx12  = xBegin_[2] - xBegin_[1],
                          dx02  = xBegin_[2] - xBegin_[0];
            result_type   dy01  = yBegin_[1] - yBegin_[0],
                          dy12  = yBegin_[2] - yBegin_[1];
                          
            L.setFirstRow(dx01,dx02);
            tmp[0] = ((dx01+2.0*dx02)*dx12*dy01/dx01 + 
                       dx01*dx01*dy12/dx12)/dx02;
            
            size_t i;
            for (i=1; i<n-1; i++) {
                argument_type dxp = xBegin_[i+1] - xBegin_[i],
                              dxm = xBegin_[i]   - xBegin_[i-1];
                result_type   dyp = yBegin_[i+1] - yBegin_[i],
                              dym = yBegin_[i]   - yBegin_[i-1];
                L.setMidRow(i, dxp, 2.0*(dxp+dxm), dxm);
                tmp[i] = 3.0*(dxp*dym/dxm + dxm*dyp/dxp);
            }
            
            argument_type dxN32 = xBegin_[n-2] - xBegin_[n-3],
                          dxN21 = xBegin_[n-1] - xBegin_[n-2],
                          dxN31 = xBegin_[n-1] - xBegin_[n-3];
            result_type   dyN32 = yBegin_[n-2] - yBegin_[n-3],
                          dyN21 = yBegin_[n-1] - yBegin_[n-2];
            
            L.setLastRow(dxN31,dxN21);
            tmp[n-1] = ((dxN21+2.0*dxN31)*dxN32*dyN21/dxN21 + 
                         dxN21*dxN21*dyN32/dxN32)/dxN31;
            
            tmp = L.solveFor(tmp);
            
            for (i=0; i<n-1; i++) {
                argument_type dx  = xBegin_[i+1]-xBegin_[i];
                result_type   dy  = yBegin_[i+1]-yBegin_[i];

                a_[i] = tmp[i];
                c_[i] = (tmp[i] + tmp[i+1] - 2.0*dy/dx)/(dx*dx);
                b_[i] = (dy/dx - tmp[i])/dx - c_[i]*dx;
            }
        }

        template <class I1, class I2>
        CubicSpline<I1,I2>::result_type
        CubicSpline<I1,I2>::operator()(
            const CubicSpline<I1,I2>::argument_type& x) const {
                I1 i;
                if (x < *xBegin_)
                    i = xBegin_;
                else if (x > *(xEnd_-1))
                    i = xEnd_-2;
                else
                    i = Location(xBegin_,xEnd_,x);

                size_t j = i-xBegin_;
                argument_type dx = x-xBegin_[j];
                return yBegin_[j] + dx*(a_[j] + dx*(b_[j] + dx*c_[j]));
        }

    }

}


#endif
