/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file lexicographicalview.hpp
    \brief Lexicographical 2-D view of a contiguous set of data.
*/

#ifndef quantlib_lexicographical_view_hpp
#define quantlib_lexicographical_view_hpp

#include <ql/utilities/steppingiterator.hpp>
#include <boost/iterator/reverse_iterator.hpp>

namespace QuantLib {

    /*! \deprecated To be removed as unused.
                    Copy it in your codebase if you need it.
                    Deprecated in version 1.26.
    */
    template <class RandomAccessIterator>
    class QL_DEPRECATED LexicographicalView {
      public:
        //! attaches the view with the given dimension to a sequence
        LexicographicalView(const RandomAccessIterator& begin,
                            const RandomAccessIterator& end, Size xSize);
        //! iterates over \f$ v_{ij} \f$ with \f$ j \f$ fixed.
        typedef RandomAccessIterator x_iterator;
        //! iterates backwards over \f$ v_{ij} \f$ with \f$ j \f$ fixed.
        typedef boost::reverse_iterator<RandomAccessIterator>
                                                         reverse_x_iterator;
        //! iterates over \f$ v_{ij} \f$ with \f$ i \f$ fixed.
        typedef step_iterator<RandomAccessIterator> y_iterator;
        //! iterates backwards over \f$ v_{ij} \f$ with \f$ i \f$ fixed.
        typedef boost::reverse_iterator<y_iterator> reverse_y_iterator;

        //! \name Element access
        //@{
        y_iterator       operator[](Size i);
        //@}

        //! \name Iterator access
        //@{
        x_iterator               xbegin (Size j);
        x_iterator               xend   (Size j);
        reverse_x_iterator       rxbegin(Size j);
        reverse_x_iterator       rxend  (Size j);
        y_iterator               ybegin (Size i);
        y_iterator               yend   (Size i);
        reverse_y_iterator       rybegin(Size i);
        reverse_y_iterator       ryend  (Size i);
        //@}

        //! \name Inspectors
        //@{
        //! dimension of the array along x
        Size xSize() const;
        //! dimension of the array along y
        Size ySize() const;
        //@}
      private:
        RandomAccessIterator begin_, end_;
        Size xSize_, ySize_;
    };


    // inline definitions

    QL_DEPRECATED_DISABLE_WARNING

    template <class RandomAccessIterator>
    inline
    LexicographicalView<RandomAccessIterator>::LexicographicalView(
                                            const RandomAccessIterator& begin,
                                            const RandomAccessIterator& end,
                                            Size xSize)
    : begin_(begin), end_(end), xSize_(xSize),
      ySize_((end-begin)/xSize) {
        QL_REQUIRE((end_-begin_) % xSize_ == 0,
                   "The x size of the view is not an exact divisor"
                   "of the size of the underlying sequence");
    }

    template <class RandomAccessIterator>
    inline typename LexicographicalView<RandomAccessIterator>::x_iterator
    LexicographicalView<RandomAccessIterator>::xbegin(Size j) {
        return begin_+j*xSize_;
    }

    template <class RandomAccessIterator>
    inline typename LexicographicalView<RandomAccessIterator>::x_iterator
    LexicographicalView<RandomAccessIterator>::xend(Size j) {
        return begin_+(j+1)*xSize_;
    }

    template <class RandomAccessIterator>
    inline
    typename LexicographicalView<RandomAccessIterator>::reverse_x_iterator
    LexicographicalView<RandomAccessIterator>::rxbegin(Size j) {
        return reverse_x_iterator(xend(j));
    }

    template <class RandomAccessIterator>
    inline
    typename LexicographicalView<RandomAccessIterator>::reverse_x_iterator
    LexicographicalView<RandomAccessIterator>::rxend(Size j) {
        return reverse_x_iterator(xbegin(j));
    }

    template <class RandomAccessIterator>
    inline typename LexicographicalView<RandomAccessIterator>::y_iterator
    LexicographicalView<RandomAccessIterator>::ybegin(Size i) {
        return y_iterator(begin_+i,xSize_);
    }

    template <class RandomAccessIterator>
    inline typename LexicographicalView<RandomAccessIterator>::y_iterator
    LexicographicalView<RandomAccessIterator>::yend(Size i) {
        return y_iterator(begin_+i,xSize_)+ySize_;
    }

    template <class RandomAccessIterator>
    inline
    typename LexicographicalView<RandomAccessIterator>::reverse_y_iterator
    LexicographicalView<RandomAccessIterator>::rybegin(Size i) {
        return reverse_y_iterator(yend(i));
    }

    template <class RandomAccessIterator>
    inline
    typename LexicographicalView<RandomAccessIterator>::reverse_y_iterator
    LexicographicalView<RandomAccessIterator>::ryend(Size i) {
        return reverse_y_iterator(ybegin(i));
    }

    template <class RandomAccessIterator>
    inline typename LexicographicalView<RandomAccessIterator>::y_iterator
    LexicographicalView<RandomAccessIterator>::operator[](Size i) {
        return y_iterator(begin_+i,xSize_);
    }

    template <class RandomAccessIterator>
    inline Size LexicographicalView<RandomAccessIterator>::xSize() const {
        return xSize_;
    }

    template <class RandomAccessIterator>
    inline Size LexicographicalView<RandomAccessIterator>::ySize() const {
        return ySize_;
    }

    QL_DEPRECATED_ENABLE_WARNING
}


#endif
