/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file singleton.hpp
    \brief basic support for the singleton pattern
*/

#ifndef quantlib_singleton_hpp
#define quantlib_singleton_hpp

#include <ql/qldefines.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace QuantLib {

    //! Basic support for the singleton pattern.
    /*! The typical use of this class is:
        \code
        class Foo : public Singleton<Foo> {
            friend class Singleton<Foo>;
          private:
            Foo() {}
          public:
            ...
        };
        \endcode
        which, albeit sub-optimal, frees one from the concerns of
        creating and managing the unique instance and can serve later
        as a single implemementation point should synchronization
        features be added.

        \ingroup patterns
    */
    template <class T>
    class Singleton : private boost::noncopyable {
      public:
        //! access to the unique instance
        static T& instance();
      protected:
        Singleton() {}
    #ifdef QL_PATCH_MSVC6
      private:
        static boost::shared_ptr<T> instance_;
    #endif
    };


    // template definitions

    #ifdef QL_PATCH_MSVC6
    template <class T>
    boost::shared_ptr<T> Singleton<T>::instance_;
    #endif

    template <class T>
    T& Singleton<T>::instance() {
        #ifndef QL_PATCH_MSVC6
        static boost::shared_ptr<T> instance_(new T);
        #else
        if (!instance_)
            instance_ = boost::shared_ptr<T>(new T);
        #endif
        return *instance_;
    }

}


#endif
