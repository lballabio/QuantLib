
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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
*/

#ifndef quantlib_bilinear_interpolation_h
#define quantlib_bilinear_interpolation_h

#include <ql/Math/interpolation2D.hpp>

namespace QuantLib {

    namespace Math {

        //! bilinear interpolation between discrete points
        template <class RandomAccessIteratorX, 
                  class RandomAccessIteratorY,
                  class MatricialData>
        class BilinearInterpolation
        : public Interpolation2D<RandomAccessIteratorX,
                                 RandomAccessIteratorY,
                                 MatricialData> {
          public:
            typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIteratorX>::value_type
                first_argument_type;
            typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIteratorY>::value_type
                second_argument_type;
            typedef double result_type;
            BilinearInterpolation(
                const RandomAccessIteratorX& xBegin,
                const RandomAccessIteratorX& xEnd,
                const RandomAccessIteratorY& yBegin,
                const RandomAccessIteratorY& yEnd,
                const MatricialData& data)
            : Interpolation2D<RandomAccessIteratorX,
                              RandomAccessIteratorY,
                              MatricialData>(
                 xBegin,xEnd,yBegin,yEnd,data) {}
            result_type operator()(
                const first_argument_type& x,
                const second_argument_type& y,
                bool allowExtrapolation = false) const;
        };


        // template definitions

        template <class I1, class I2, class M>
        double BilinearInterpolation<I1,I2,M>::operator()(
            const typename 
            BilinearInterpolation<I1,I2,M>::first_argument_type& x,
            const typename 
            BilinearInterpolation<I1,I2,M>::second_argument_type& y,
            bool allowExtrapolation) const {

                locate(x, y);
                if (isOutOfRange_) {
                    QL_REQUIRE(allowExtrapolation,
                        "BilinearInterpolation::operator() : "
                        "extrapolation not allowed");
                }

                double z1=data_[yPos_-yBegin_]   [xPos_-xBegin_];
                double z2=data_[yPos_-yBegin_]   [xPos_-xBegin_+1];
                double z3=data_[yPos_-yBegin_+1] [xPos_-xBegin_];
                double z4=data_[yPos_-yBegin_+1] [xPos_-xBegin_+1];

                double t=(x-*xPos_)/(*(xPos_+1)-*xPos_);
                double u=(y-*yPos_)/(*(yPos_+1)-*yPos_);

                return (1.0-t) * (1.0-u) * z1+
                          t    * (1.0-u) * z2+
                       (1.0-t) *    u    * z3+
                          t    *    u    * z4;
        }

    }

}


#endif
