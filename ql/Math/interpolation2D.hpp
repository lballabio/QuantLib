

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
/*! \file interpolation2D.hpp
    \brief abstract base classes for 2-D interpolations

    \fullpath
    ql/Math/%interpolation2D.hpp
*/

// $Id$

#ifndef quantlib_interpolation2D_h
#define quantlib_interpolation2D_h

#include <ql/Math/matrix.hpp>

namespace QuantLib {

    namespace Math {

        //! abstract base class for 2-D interpolations
        /*! Classes derived from this class will override operator() to
            provide interpolated values from two sequences of length N and M,
            representing the discretized values of the x,y variables,
            and a NxM matrix representing the function tabulated z values.
        */
        template <class RandomAccessIteratorX, class RandomAccessIteratorY>
        class Interpolation2D {
          public:
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIteratorX>::value_type
                x_type;
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIteratorY>::value_type
                y_type;
            Interpolation2D(const RandomAccessIteratorX& xBegin,
                const RandomAccessIteratorY& yBegin,
                const Matrix& dataMatrix);
            virtual ~Interpolation2D() {}
            /*! This operator must be overridden to provide an implementation
                of the actual interpolation.

                \pre The sequence of values for x must have been sorted for
                the result to make sense.
            */
            virtual double operator()(const x_type& x,
                const y_type& y) const = 0;
          protected:
            RandomAccessIteratorX xBegin_;
            RandomAccessIteratorY yBegin_;
            Matrix dataMatrix_;
        };

        // inline definitions

        template <class I1, class I2>
        inline Interpolation2D<I1,I2>::Interpolation2D(const I1& xBegin,
            const I2& yBegin, const Matrix& dataMatrix)
        : xBegin_(xBegin), yBegin_(yBegin), dataMatrix_(dataMatrix) {
            #ifdef QL_DEBUG
                QL_REQUIRE(dataMatrix_.cols() >= 2,
                    "not enough cols to interpolate");
                QL_REQUIRE(dataMatrix_.rows() >= 2,
                    "not enough rows to interpolate");
            #endif
        }



    }

}


#endif
