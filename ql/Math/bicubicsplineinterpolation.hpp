
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file bicubicsplineinterpolation.hpp
    \brief bicubic spline interpolation between discrete points
*/

#ifndef quantlib_bicubicspline_interpolation_h
#define quantlib_bicubicspline_interpolation_h

#include <ql/Math/interpolation2D.hpp>
#include <ql/Math/cubicspline.hpp>

namespace QuantLib {

    //! bicubic spline interpolation between discrete points
    template <class RandomAccessIteratorX,
              class RandomAccessIteratorY,
              class MatricialData>
    class BicubicSplineInterpolation
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
        BicubicSplineInterpolation(const RandomAccessIteratorX& xBegin,
                                   const RandomAccessIteratorX& xEnd,
                                   const RandomAccessIteratorY& yBegin,
                                   const RandomAccessIteratorY& yEnd,
                                   const MatricialData& data);
        result_type operator()(const first_argument_type& x,
                               const second_argument_type& y,
                               bool allowExtrapolation = false) const;
      private:
        Size rows_;
        std::vector<CubicSplineInterpolation<
                            RandomAccessIteratorX,
                            typename MatricialData::const_row_iterator> >
                                                                     splines_;
    };


    // inline definitions
    template <class I1, class I2, class M>
    BicubicSplineInterpolation<I1,I2,M>::BicubicSplineInterpolation(
                                             const I1& xBegin, const I1& xEnd,
                                             const I2& yBegin, const I2& yEnd,
                                             const M& data)
    : Interpolation2D<I1,I2, M>(xBegin,xEnd,yBegin,yEnd,data),
      rows_(data_.rows()) {
        typedef typename M::const_row_iterator row_iterator;
        for (Size i = 0; i< rows_; i++)
            splines_.push_back(CubicSplineInterpolation<I1, row_iterator>(
           xBegin, xEnd, data_.row_begin(i),
           Null<double>(), 0.0, // something better needed here
           Null<double>(), 0.0, // something better needed here
           false));
    }

    template <class I1, class I2, class M>
    double BicubicSplineInterpolation<I1,I2,M>::operator()(
               const typename
               BicubicSplineInterpolation<I1,I2,M>::first_argument_type& x,
               const typename
               BicubicSplineInterpolation<I1,I2,M>::second_argument_type& y,
               bool allowExtrapolation) const {

        std::vector<result_type> newColumn(rows_);
        for (Size i = 0; i< rows_; i++) {
            newColumn[i]=splines_[i](x, allowExtrapolation);
        }

        CubicSplineInterpolation<I2,std::vector<result_type>::const_iterator>
            columnSpline(yBegin_, yEnd_, newColumn.begin(),
                Null<double>(), 0.0, // something better needed here
                Null<double>(), 0.0, // something better needed here
                false);

        return columnSpline(y, allowExtrapolation);
    }

}


#endif
