
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

/*! \file iteratorcategories.h
    \brief Lowest common denominator between two iterator categories

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/02/16 15:11:50  lballabio
    Hidden a few classes from Doxygen

    Revision 1.1  2001/02/14 18:43:07  lballabio
    Added coupling iterators

*/

#ifndef quantlib_iterator_categories_h
#define quantlib_iterator_categories_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

        /*! Specializations of this struct define a typedef iterator_category 
            which corresponds to the more generic of the two input categories, 
            e.g., lowest_category_iterator<std::random_access_iterator_tag,
            std::forward_iterator_tag>::iterator_category corresponds to 
            std::forward_iterator_tag.
        */
        template <class Category1, class Category2>
        struct lowest_category_iterator {};
        
        #if !defined(__DOXYGEN__)
        template <>
        struct lowest_category_iterator<
            std::random_access_iterator_tag,
            std::random_access_iterator_tag> {
                typedef std::random_access_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::random_access_iterator_tag,
            std::bidirectional_iterator_tag> {
                typedef std::bidirectional_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::random_access_iterator_tag,
            std::forward_iterator_tag> {
                typedef std::forward_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::random_access_iterator_tag,
            std::input_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::random_access_iterator_tag,
            std::output_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };


        template <>
        struct lowest_category_iterator<
            std::bidirectional_iterator_tag,
            std::random_access_iterator_tag> {
                typedef std::bidirectional_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::bidirectional_iterator_tag,
            std::bidirectional_iterator_tag> {
                typedef std::bidirectional_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::bidirectional_iterator_tag,
            std::forward_iterator_tag> {
                typedef std::forward_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::bidirectional_iterator_tag,
            std::input_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::bidirectional_iterator_tag,
            std::output_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };


        template <>
        struct lowest_category_iterator<
            std::forward_iterator_tag,
            std::random_access_iterator_tag> {
                typedef std::forward_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::forward_iterator_tag,
            std::bidirectional_iterator_tag> {
                typedef std::forward_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::forward_iterator_tag,
            std::forward_iterator_tag> {
                typedef std::forward_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::forward_iterator_tag,
            std::input_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::forward_iterator_tag,
            std::output_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };


        template <>
        struct lowest_category_iterator<
            std::input_iterator_tag,
            std::random_access_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::input_iterator_tag,
            std::bidirectional_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::input_iterator_tag,
            std::forward_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::input_iterator_tag,
            std::input_iterator_tag> {
                typedef std::input_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::input_iterator_tag,
            std::output_iterator_tag> {
                typedef void iterator_category;
        };


        template <>
        struct lowest_category_iterator<
            std::output_iterator_tag,
            std::random_access_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::output_iterator_tag,
            std::bidirectional_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::output_iterator_tag,
            std::forward_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::output_iterator_tag,
            std::input_iterator_tag> {
                typedef void iterator_category;
        };

        template <>
        struct lowest_category_iterator<
            std::output_iterator_tag,
            std::output_iterator_tag> {
                typedef std::output_iterator_tag iterator_category;
        };
        #endif

    }

}


#endif
