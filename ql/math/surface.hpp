/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

/*! \file surface.hpp
    \brief Surface
*/

#ifndef quantlib_surface_hpp
#define quantlib_surface_hpp

#include <ql/math/domain.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>

namespace QuantLib {

// we know Domain is deprecated, we're deprecating Surface too.
#if defined(BOOST_MSVC)
#pragma warning( push )
#pragma warning( disable : 4996 )
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

    //! %Surface abstract class
    /*! \deprecated */
    class QL_DEPRECATED Surface
        : public std::binary_function<Real, Real, Real> {
      public:
        virtual Real operator()(Real x, Real y) const = 0;
        virtual boost::shared_ptr<Domain> domain() const = 0;
        virtual ~Surface() {};
    };

    class QL_DEPRECATED TestSurface : public Surface {
      public:
        Real operator()(Real x, Real y) const;
        boost::shared_ptr<Domain> domain() const;
    };

#if defined(BOOST_MSVC)
#pragma warning( pop )
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

}


#endif
