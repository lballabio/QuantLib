
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

/*! \file interpolation.hpp
    \brief abstract base classes for interpolations

    $Source$
    $Log$
    Revision 1.4  2001/05/24 13:57:51  nando
    smoothing #include xx.hpp and cutting old Log messages

    Revision 1.3  2001/05/24 12:52:02  nando
    smoothing #include xx.hpp

*/

#ifndef quantlib_interpolation_h
#define quantlib_interpolation_h

#include "ql/qlerrors.hpp"

namespace QuantLib {

    namespace Math {

        //! abstract base class for 1-D interpolations
        /*! Classes derived from this class will override operator() to provide
            interpolated values from two sequences of equal length, representing
            discretized values of a variable and a function of the former,
            respectively.
        */
        template <class RandomAccessIterator1, class RandomAccessIterator2>
        class Interpolation {
          public:
            /*  these typedefs are here because Borland C++ won't inherit
                them from unary_function - they shouldn't hurt, though.
            */
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIterator1>::value_type
                argument_type;
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIterator2>::value_type
                result_type;
            Interpolation(const RandomAccessIterator1& xBegin,
                const RandomAccessIterator1& xEnd,
                const RandomAccessIterator2& yBegin);
            /*! This operator must be overridden to provide an implementation of
                the actual interpolation.

                \pre The sequence of values for x must have been sorted for the
                    result to make sense.
            */
            virtual result_type operator()(const argument_type& x) const = 0;
          protected:
            RandomAccessIterator1 xBegin_, xEnd_;
            RandomAccessIterator2 yBegin_;
        };

        //! Helper function to find the values between which to interpolate
        /*! It returns either an iterator <tt>i</tt> such that
            <tt>*i <= x <= *(i+1)</tt> or <tt>xEnd</tt> if no such iterator
            exists.

            \pre The sequence of values must have been sorted for the
                result to make sense.
        */
        template <class RandomAccessIterator>
        RandomAccessIterator Location(
            const RandomAccessIterator& begin,
            const RandomAccessIterator& end,
            const typename
            QL_ITERATOR_TRAITS<RandomAccessIterator>::value_type& x);


        // inline definitions

        template <class I1, class I2>
        inline Interpolation<I1,I2>::Interpolation(const I1& xBegin,
            const I1& xEnd, const I2& yBegin)
        : xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin) {
            #ifdef QL_DEBUG
                QL_REQUIRE(xEnd_-xBegin_ >= 2,
                    "not enough points to interpolate");
            #endif
        }

        // template definitions

        template <class I>
        I Location(const I& begin, const I& end,
            const typename QL_ITERATOR_TRAITS<I>::value_type& x) {
            I i = begin, j = end-1;
               if (x < *i || x > *j)
                return end;
            while (j-i > 1) {
                I k = i+(j-i)/2;
                if (x == *k)
                    return k;
                else if (x < *k)
                    j = k;
                else
                    i = k;
            }
            return i;
        }

    }

}


#endif
