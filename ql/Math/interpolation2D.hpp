
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

/*! \file interpolation2D.hpp
    \brief abstract base classes for 2-D interpolations
*/

#ifndef quantlib_interpolation2D_h
#define quantlib_interpolation2D_h

#include <ql/errors.hpp>

namespace QuantLib {

    namespace Math {

        //! abstract base class for 2-D interpolations
        /*! Classes derived from this class will override operator() to
            provide interpolated values from two sequences of length N and M,
            representing the discretized values of the x,y variables,
            and a NxM matrix representing the function tabulated z values.

            \todo Bicubic interpolation and bicubic spline */
        template <class RandomAccessIteratorX, 
                  class RandomAccessIteratorY,
                  class MatricialData>
        class Interpolation2D {
          public:
            typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIteratorX>::value_type
                first_argument_type;
            typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIteratorY>::value_type
                second_argument_type;
            // I could think of no way of extracting type info from all
            // the matricial types that could be passed here.
            // We'll just cast to double for the time being
            typedef double result_type;
            Interpolation2D(
                const RandomAccessIteratorX& xBegin,
                const RandomAccessIteratorX& xEnd,
                const RandomAccessIteratorY& yBegin,
                const RandomAccessIteratorY& yEnd,
                const MatricialData& data);
            virtual ~Interpolation2D() {}
            /*! This operator must be overridden to provide an implementation
                of the actual interpolation.

                \pre The sequence of values for x must have been sorted for
                the result to make sense.
            */
            virtual double operator()(const first_argument_type& x,
                const second_argument_type& y,
                bool allowExtrapolation = false) const = 0;
          protected:
            void locate(const first_argument_type& x,
                        const second_argument_type& y) const;
            mutable bool isOutOfRange_;
            mutable RandomAccessIteratorX xPos_;
            mutable RandomAccessIteratorY yPos_;
            RandomAccessIteratorX xBegin_, xEnd_;
            RandomAccessIteratorY yBegin_, yEnd_;
            // the iterators above already introduce lifetime issues.
            // There would be no added advantage in copying the data.
            const MatricialData& data_;
        };


        // inline definitions

        template <class I1, class I2, class M>
        inline Interpolation2D<I1,I2,M>::Interpolation2D(
            const I1& xBegin, const I1& xEnd, 
            const I2& yBegin, const I2& yEnd, const M& data)
        : isOutOfRange_(false), xPos_(xBegin), yPos_(yBegin), xBegin_(xBegin),
          xEnd_(xEnd), yBegin_(yBegin), yEnd_(yEnd), data_(data) {
            int i;

            QL_REQUIRE(xEnd_-xBegin_ >= 2,
                "not enough columns to interpolate");
            I1 xi = xBegin_+1;
            for (i=1; i<xEnd_-xBegin_; i++, xi++) {
                QL_REQUIRE(double(*xi-*(xi-1)) > 0.0,
                    "Interpolation::Interpolation : "
                    "x[i] not sorted");
            }

            QL_REQUIRE(yEnd_-yBegin_ >= 2,
                "not enough rows to interpolate");
            I2 yi = yBegin_+1;
            for (i=1; i<yEnd_-yBegin_; i++, yi++) {
                QL_REQUIRE(double(*yi-*(yi-1)) > 0.0,
                    "Interpolation::Interpolation : "
                    "y[i] not sorted");
            }
        }

        template <class I1, class I2, class M>
        inline void Interpolation2D<I1,I2,M>::locate(const first_argument_type& x,
            const second_argument_type& y) const {

                // column
                if (x < *xBegin_) {
                    isOutOfRange_ = true;
                    xPos_ = xBegin_;
                } else if (x > *(xEnd_-1)) {
                    isOutOfRange_ = true;
                    xPos_ = xEnd_-2;
                } else {
                    isOutOfRange_ = false;
                    xPos_ = std::upper_bound(xBegin_,xEnd_-1,x)-1;
                }

                // row
                if (y < *yBegin_) {
                    isOutOfRange_ = true;
                    yPos_ = yBegin_;
                } else if (y > *(yEnd_-1)) {
                    isOutOfRange_ = true;
                    yPos_ = yEnd_-2;
                } else {
                    isOutOfRange_ = false;
                    yPos_ = std::upper_bound(yBegin_,yEnd_-1,y)-1;
                }


        }


    }

}


#endif
