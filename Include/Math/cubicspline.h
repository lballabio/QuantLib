
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file cubicspline.h
    \brief cubic spline interpolation between discrete points

    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/02/16 15:32:22  lballabio
    Used QL_ITERATOR_TRAITS macro

    Revision 1.2  2001/01/17 14:37:55  nando
    tabs removed

    Revision 1.1  2001/01/15 13:36:01  lballabio
    Added interpolation classes

*/

#ifndef quantlib_cubic_spline_h
#define quantlib_cubic_spline_h

#include "qldefines.h"
#include "qlerrors.h"
#include "interpolation.h"

namespace QuantLib {

    namespace Math {

        //! cubic spline interpolation between discrete points
        template <class RandomAccessIterator1, class RandomAccessIterator2>
        class CubicSpline
        : public Interpolation<RandomAccessIterator1,RandomAccessIterator2> {
          public:
            /*  these typedefs are repeated because Borland C++ won't inherit
                them from Interpolation - they shouldn't hurt, though.
            */
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
          private:
            std::vector<result_type> d2y_;
        };


        // template definitions

        template <class I1, class I2>
        CubicSpline<I1,I2>::CubicSpline(const I1& xBegin, const I1& xEnd,
            const I2& yBegin)
        : Interpolation<I1,I2>(xBegin,xEnd,yBegin), d2y_(xEnd-xBegin) {
            int n = xEnd_-xBegin_;
            #ifdef QL_DEBUG
                QL_REQUIRE(n >= 4,
                    "not enough points for cubic spline interpolation");
            #endif
            // calculate cubic spline coefficients
            std::vector<result_type> u(n-1);
            d2y_[0] = u[0] = result_type();
            I1 xi = xBegin_+1;
            I2 yi = yBegin_+1;
            int i;
            for (i=1; i<n-1; i++,xi++,yi++) {
                double sig = double(*xi-*(xi-1))/double(*(xi+1)-*(xi-1));
                result_type p = sig*d2y_[i-1]+2.0;
                d2y_[i] = (sig-1)/p;
                u[i] = (*(yi+1)-*yi)/double(*(xi+1)-*xi) -
                    (*yi-*(yi-1))/double(*xi-*(xi-1));
                u[i] = (6*u[i]/double(*(xi+1)-*(xi-1))-sig*u[i-1])/p;
            }

            d2y_[n-1] = result_type();
            for (i=n-2; i>=0; i--)
                d2y_[i] = d2y_[i]*d2y_[i+1] + u[i];
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
                I2 j = yBegin_+(i-xBegin_);
                std::vector<double>::const_iterator k =
                    d2y_.begin()+(i-xBegin_);
                double h = double(*(i+1)-*i);
                double a = double(*(i+1)-x)/h;
                double b = 1.0-a;
                return a*(*j) + b*(*(j+1)) +
                    ((a*a*a-a)*(*k)+(b*b*b-b)*(*(k+1)))*(h*h)/6.0;
        }

    }

}


#endif
