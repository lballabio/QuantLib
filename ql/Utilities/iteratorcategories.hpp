
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
/*! \file iteratorcategories.hpp
    \brief Lowest common denominator between two iterator categories

    \fullpath
    ql/Utilities/%iteratorcategories.hpp
*/

// $Id$

#ifndef quantlib_iterator_categories_h
#define quantlib_iterator_categories_h

#include <iterator>

namespace QuantLib {

    //! Classes and functions of general utility
    /*! See sect. \ref utilities */
    namespace Utilities {

        //! most generic of two given iterator categories
        /*! Specializations of this struct define a typedef iterator_category
            which corresponds to the most generic of the two input 
            categories, e.g., 
            lowest_category_iterator<std::random_access_iterator_tag,
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
