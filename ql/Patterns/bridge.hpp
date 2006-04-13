/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bridge.hpp
    \brief bridge pattern (a.k.a. handle-body idiom)
*/

#ifndef quantlib_bridge_hpp
#define quantlib_bridge_hpp

#include <ql/qldefines.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    //! The %Bridge pattern made explicit.
    /*! The typical use of this class is:
        \code
        class FooImpl;
        class Foo : public Bridge<Foo,FooImpl> {
            ...
        };
        \endcode
        which makes it possible to pass instances of class Foo
        by value while retaining polymorphic behavior.

        \ingroup patterns
    */
    template <class T, class T_impl>
    class Bridge {
      public:
        typedef T_impl Impl;
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated renamed to empty() */
        bool isNull() const { return empty(); }
        #endif
        bool empty() const { return !impl_; }
      protected:
        Bridge(const boost::shared_ptr<Impl>& impl = boost::shared_ptr<Impl>())
        : impl_(impl) {}
        boost::shared_ptr<Impl> impl_;
    };

}


#endif
