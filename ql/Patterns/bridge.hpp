
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file bridge.hpp
    \brief bridge pattern (a.k.a. handle-body idiom)

    \fullpath
    ql/Patterns/%bridge.hpp
*/

// $Id$

#ifndef quantlib_bridge_h
#define quantlib_bridge_h

#include <ql/handle.hpp>

namespace QuantLib {

    namespace Patterns {

        /*! The Bridge pattern made explicit. 
            One will typically use it as in:
            \code
            class FooImpl;
            class Foo : public Bridge<Foo,FooImpl> {
                ...
            };
            \endcode
        */
        template <class T, class T_impl>
        class Bridge {
          public:
            typedef T_impl Impl;
          protected:
            Bridge(const Handle<Impl>& impl) : impl_(impl) {}
            Handle<Impl> impl_;
        };
                        
    }

}


#endif
