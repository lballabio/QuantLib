
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file interpolation.hpp
    \brief abstract base classes for interpolations

    \fullpath
    ql/Math/%interpolation.hpp
*/

// $Id$

#ifndef quantlib_interpolation_h
#define quantlib_interpolation_h

#include <ql/errors.hpp>

namespace QuantLib {

    namespace Math {

        //! abstract base class for 1-D interpolations
        /*! Classes derived from this class will override operator() to
            provide interpolated values from two sequences of equal length,
            representing discretized values of a variable and a function of
            the former, respectively.
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
            /*! This operator must be overridden to provide an implementation
                of the actual interpolation.

                \pre The sequence of values for x must have been sorted for
                the result to make sense.
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
