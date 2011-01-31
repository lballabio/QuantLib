/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file composite.hpp
    \brief composite pattern
*/

#ifndef quantlib_composite_hpp
#define quantlib_composite_hpp

#include <ql/qldefines.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

namespace QuantLib {

    //! %Composite pattern.
    /*! The typical use of this class is:
        \code
        class CompositeFoo : public Composite<Foo> {
            ...
        };
        \endcode
        which causes CompositeFoo to inherit from Foo and provides it
        with methods for adding components. Of course, any abstract
        Foo interface must still be implemented.

        \ingroup patterns
    */
    template <class T>
    class Composite : public T {
      protected:
        std::list<boost::shared_ptr<T> > components_;
        void add(const boost::shared_ptr<T>& c) { components_.push_back(c); }
        typedef typename std::list<boost::shared_ptr<T> >::iterator iterator;
        typedef typename std::list<boost::shared_ptr<T> >::const_iterator
                                                              const_iterator;
    };

}


#endif
