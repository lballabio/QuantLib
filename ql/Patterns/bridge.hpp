
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

/*! \file bridge.hpp
    \brief bridge pattern (a.k.a. handle-body idiom)
*/

#ifndef quantlib_bridge_h
#define quantlib_bridge_h

#include <ql/handle.hpp>

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
    */
    template <class T, class T_impl>
    class Bridge {
      public:
        typedef T_impl Impl;
        bool isNull() const {
            return !impl_;
        }
      protected:
        Bridge(const boost::shared_ptr<Impl>& impl = boost::shared_ptr<Impl>())
        : impl_(impl) {}
        boost::shared_ptr<Impl> impl_;
    };

}


#endif
