
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file lexicographicalview.hpp
    \brief Lexicographical 2-D view of a contiguous set of data.

    $Id$
*/

// $Source$
// $Log$
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_lexicographical_view_h
#define quantlib_lexicographical_view_h

#include "ql/Utilities/steppingiterator.hpp"

namespace QuantLib {

    namespace Math {

        //! Lexicographical 2-D view of a contiguous set of data.
        /*! This view can be used to easily store a discretized 2-D function
            in an array to be used in a finite differences calculation.
        */
        template <class RandomAccessIterator>
        class LexicographicalView {
          public:
            //! attaches the view with the given dimension to a sequence
            LexicographicalView(const RandomAccessIterator& begin,
                const RandomAccessIterator& end, int xSize);
            //! iterates over \f$ v_{ij} \f$ with \f$ j \f$ fixed.
            typedef RandomAccessIterator x_iterator;
            //! iterates backwards over \f$ v_{ij} \f$ with \f$ j \f$ fixed.
            typedef QL_REVERSE_ITERATOR(RandomAccessIterator,
                typename QL_ITERATOR_TRAITS<RandomAccessIterator>::value_type)
                    reverse_x_iterator;
            //! iterates over \f$ v_{ij} \f$ with \f$ i \f$ fixed.
            typedef Utilities::stepping_iterator<RandomAccessIterator>
                y_iterator;
            //! iterates backwards over \f$ v_{ij} \f$ with \f$ i \f$ fixed.
            typedef QL_REVERSE_ITERATOR(y_iterator,
                typename QL_ITERATOR_TRAITS<RandomAccessIterator>::value_type)
                    reverse_y_iterator;

            //! \name Element access
            //@{
            y_iterator       operator[](int i);
            //@}

            //! \name Iterator access
            //@{
            x_iterator               xbegin (int j);
            x_iterator               xend   (int j);
            reverse_x_iterator       rxbegin(int j);
            reverse_x_iterator       rxend  (int j);
            y_iterator               ybegin (int i);
            y_iterator               yend   (int i);
            reverse_y_iterator       rybegin(int i);
            reverse_y_iterator       ryend  (int i);
            //@}

            //! \name Inspectors
            //@{
            //! dimension of the array along x
            int xSize() const;
            //! dimension of the array along y
            int ySize() const;
            //@}
          private:
            RandomAccessIterator begin_, end_;
            int xSize_, ySize_;
        };


        // inline definitions

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::LexicographicalView(
            const RandomAccessIterator& begin, const RandomAccessIterator& end,
            int xSize)
        : begin_(begin), end_(end), xSize_(xSize), ySize_((end-begin)/xSize) {
            #ifdef QL_DEBUG
                QL_REQUIRE((end_-begin_) % xSize_ == 0,
                    "The x size of the view is not an exact divisor"
                    "of the size of the underlying sequence");
            #endif
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::x_iterator
        LexicographicalView<RandomAccessIterator>::xbegin(int j) {
            return begin_+j*xSize_;
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::x_iterator
        LexicographicalView<RandomAccessIterator>::xend(int j) {
            return begin_+(j+1)*xSize_;
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::reverse_x_iterator
        LexicographicalView<RandomAccessIterator>::rxbegin(int j) {
            return reverse_x_iterator(xend(j));
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::reverse_x_iterator
        LexicographicalView<RandomAccessIterator>::rxend(int j) {
            return reverse_x_iterator(xbegin(j));
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::y_iterator
        LexicographicalView<RandomAccessIterator>::ybegin(int i) {
            return y_iterator(begin_+i,xSize_);
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::y_iterator
        LexicographicalView<RandomAccessIterator>::yend(int i) {
            return y_iterator(begin__+i,xSize_)+ySize_;
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::reverse_y_iterator
        LexicographicalView<RandomAccessIterator>::rybegin(int i) {
            return reverse_y_iterator(yend(i));
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::reverse_y_iterator
        LexicographicalView<RandomAccessIterator>::ryend(int i) {
            return reverse_y_iterator(ybegin(i));
        }

        template <class RandomAccessIterator>
        inline LexicographicalView<RandomAccessIterator>::y_iterator
        LexicographicalView<RandomAccessIterator>::operator[](int i) {
            return y_iterator(begin_+i,xSize_);
        }

        template <class RandomAccessIterator>
        inline int LexicographicalView<RandomAccessIterator>::xSize() const {
            return xSize_;
        }

        template <class RandomAccessIterator>
        inline int LexicographicalView<RandomAccessIterator>::ySize() const {
            return ySize_;
        }

    }

}


#endif
