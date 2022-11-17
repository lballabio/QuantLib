/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl

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

/*! \file singleton.hpp
    \brief basic support for the singleton pattern
*/

#ifndef quantlib_singleton_hpp
#define quantlib_singleton_hpp

#include <ql/types.hpp>
#include <type_traits>

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

        which, albeit sub-optimal, frees one from the concerns of creating and managing the unique instance
        and can serve later as a single implemementation point should synchronization features be added.

        Global can be used to distinguish Singletons that are local to a session (Global = false) or that are global
        across all sessions (B = true).  This is only relevant if QL_ENABLE_SESSIONS is enabled.

        Notice that the creation and retrieval of (local or global) singleton instances through instance() is thread
        safe, but obviously subsequent operations on the singleton have to be synchronized within the singleton
        implementation itself.

        \ingroup patterns
    */
    template <class T, class Global = std::integral_constant<bool, false> >
    class Singleton {
      public:
        // disable copy/move
        Singleton(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton& operator=(Singleton&&) = delete;

        //! access to the unique instance
        static T& instance();

      protected:
        Singleton() = default;
    };

    // template definitions

#ifdef QL_ENABLE_SESSIONS

#if (defined(__GNUC__) && !defined(__clang__)) && (((__GNUC__ == 8) && (__GNUC_MINOR__ < 4)) || (__GNUC__ < 8))
#pragma message("Singleton::instance() is always compiled with `-O0` for versions of GCC below 8.4 when sessions are enabled.")
#pragma message("This is to work around the following compiler bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91757")
#pragma message("If possible, please update your compiler to a more recent version.")
#pragma GCC push_options
#pragma GCC optimize("-O0")
#endif

    template <class T, class Global>
    T& Singleton<T, Global>::instance() {
        if(Global()) {
            static T global_instance;
            return global_instance;
        } else {
            thread_local static T local_instance;
            return local_instance;
        }
    }

#if (defined(__GNUC__) && !defined(__clang__)) && (((__GNUC__ == 8) && (__GNUC_MINOR__ < 4)) || (__GNUC__ < 8))
#pragma GCC pop_options
#endif

#else

    template <class T, class Global>
    T& Singleton<T, Global>::instance() {
        static T instance;
        return instance;
    }

#endif

    // backwards compatibility

#if defined(QL_THREAD_KEY)
    /*! \deprecated This typedef is obsolete. Do not use it.
                    Deprecated in version 1.29.
    */
    QL_DEPRECATED
    typedef QL_THREAD_KEY ThreadKey;
#else
    /*! \deprecated This typedef is obsolete. Do not use it.
                    Deprecated in version 1.29.
    */
    QL_DEPRECATED
    typedef Integer ThreadKey;
#endif

}

#endif
