
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file iteratorcategories.hpp
    \brief Lowest common denominator between two iterator categories
*/

#ifndef quantlib_iterator_categories_h
#define quantlib_iterator_categories_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! most generic of two given iterator categories
    /*! Specializations of this struct define a typedef
        iterator_category which corresponds to the most generic of the
        two input categories, e.g.,
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


#endif
