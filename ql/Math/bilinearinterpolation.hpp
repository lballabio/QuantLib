
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file bilinearinterpolation.hpp
    \brief bilinear interpolation between discrete points

    \fullpath
    ql/Math/%bilinearinterpolation.hpp
*/

// $Id$

#ifndef quantlib_bilinear_interpolation_h
#define quantlib_bilinear_interpolation_h

#include <ql/Math/interpolation2D.hpp>
#include <ql/Math/matrix.hpp>
#include <algorithm>

namespace QuantLib {

    namespace Math {

        //! bilinear interpolation between discrete points
        /*! \todo Bicubic interpolation and bicubic spline

            \warning not working yet !!!!!
        */
        template <class RandomAccessIteratorX, class RandomAccessIteratorY>
        class BilinearInterpolation
        : public Interpolation2D<RandomAccessIteratorX,RandomAccessIteratorY> {
          public:
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIteratorX>::value_type
                x_type;
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIteratorY>::value_type
                y_type;
             BilinearInterpolation(const RandomAccessIteratorX& xBegin,
                const RandomAccessIteratorY& yBegin,
                const Matrix& dataMatrix)
             : Interpolation2D<RandomAccessIteratorX,RandomAccessIteratorY>(
                 xBegin,yBegin,dataMatrix) {}
            double operator()(const x_type& x,
                const y_type& y) const;
        };


        // inline definitions

        template <class I1, class I2>
        inline double BilinearInterpolation<I1,I2>::operator()(
            const BilinearInterpolation<I1,I2>::x_type& x,
            const BilinearInterpolation<I1,I2>::y_type& y) const {
                I1 i; // column
                if (x < *xBegin_)
                    i = xBegin_;
                else if (x > *(xBegin_+dataMatrix_.columns()-1))
                    i = xBegin_+dataMatrix_.columns()-2;
                else
                    i = std::upper_bound(xBegin_,xBegin_+dataMatrix_.columns()-1,x)-1;
                I2 j; // row
                if (y < *yBegin_)
                    j = yBegin_;
                else if (y > *(yBegin_+dataMatrix_.rows()-1))
                    j = yBegin_+dataMatrix_.rows()-2;
                else
                    j = std::upper_bound(yBegin_,yBegin_+dataMatrix_.rows()-1,y)-1;



                double z1=dataMatrix_[j-yBegin_]   [i-xBegin_];
                double z2=dataMatrix_[j-yBegin_+1] [i-xBegin_];
                double z3=dataMatrix_[j-yBegin_+1] [i-xBegin_+1];
                double z4=dataMatrix_[j-yBegin_]   [i-xBegin_+1];


                double t=(x-*i)/(*(i+1)-*i);
                double u=(y-*j)/(*(j+1)-*j);

                return (1.0-t)*(1.0-u)*z1+
                    t*(1.0-u)         *z2+
                    t*u               *z3+
                    (1.0-t)*u         *z4;
        }

    }

}


#endif
