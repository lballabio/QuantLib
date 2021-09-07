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

#include <ql/qldefines.hpp>

#ifdef QL_ENABLE_SESSIONS
#    include <boost/thread/lock_types.hpp>
#    include <boost/thread/shared_mutex.hpp>
#else
#    ifdef QL_ENABLE_SINGLETON_THREAD_SAFE_INIT
#        include <boost/atomic.hpp>
#        include <boost/thread/mutex.hpp>
#        if !defined(BOOST_ATOMIC_ADDRESS_LOCK_FREE)
#            ifdef BOOST_MSVC
#                pragma message("Thread-safe singleton initialization may degrade performances.")
#            else
#                warning Thread-safe singleton initialization may degrade performances.
#            endif
#        endif
#    endif
#endif

#include <ql/shared_ptr.hpp>
#include <ql/types.hpp>
#include <boost/noncopyable.hpp>
#include <map>

namespace QuantLib {

// This allows to define a different type if needed, while keeping
// backwards compatibility with the current implementation.
// For instance, one might create a file threadkey.hpp with:
//
// #include <pthread.h>
// #define QL_THREAD_KEY pthread_t
//
// and then compile QuantLib with the option -DQL_INCLUDE_FIRST=threadkey.hpp
// to have that file included by qldefines.hpp and thus this one.
#if defined(QL_THREAD_KEY)
    typedef QL_THREAD_KEY ThreadKey;
#else
    typedef Integer ThreadKey;
#endif

#if defined(QL_ENABLE_SESSIONS)
    // definition must be provided by the user
    ThreadKey sessionId();
#endif

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

        Global can be used to distinguish Singletons that are local to a session
        (Global = false) or that are global across all sessions (B = true).
        This is only relevant if QL_ENABLE_SESSIONS is enabled.

        \ingroup patterns
    */
    template <class T, class Global = std::integral_constant<bool, false> >
    class Singleton : private boost::noncopyable {
      private:
#ifdef QL_ENABLE_SESSIONS
        // construct on first use to avoid static initialization order fiasko
        static std::map<ThreadKey, ext::shared_ptr<T> >& m_instances() {
            static std::map<ThreadKey, ext::shared_ptr<T> > instances;
            return instances;
        }
        static boost::shared_mutex& m_mutex() {
            static boost::shared_mutex mutex;
            return mutex;
        }
#else
#    ifdef QL_ENABLE_SINGLETON_THREAD_SAFE_INIT
        static boost::atomic<T*>& m_instance() {
            static boost::atomic<T*> instance;
            return instance;
        }
        static boost::mutex& m_mutex() {
            static boost::mutex mutex;
            return mutex;
        }
#    else
        static ext::shared_ptr<T>& m_instance() {
            static ext::shared_ptr<T> instance;
            return instance;
        }
#    endif
#endif

      public:
        //! access to the unique instance
        static T& instance();

      protected:
        Singleton() = default;
    };

    // template definitions

    template <class T, class Global>
    T& Singleton<T, Global>::instance() {

#ifdef QL_ENABLE_SESSIONS
        ThreadKey id = sessionId();
        {
            boost::shared_lock<boost::shared_mutex> shared_lock(m_mutex());
            auto instance = Global() ? m_instances().begin() : m_instances().find(id);
            if (instance != m_instances().end())
                return *instance->second;
        }
        {
            boost::unique_lock<boost::shared_mutex> uniqueLock(m_mutex());
            auto instance = Global() ? m_instances().begin() : m_instances().find(id);
            if (instance != m_instances().end())
                return *instance->second;
            auto tmp = ext::shared_ptr<T>(new T);
            m_instances()[id] = tmp;
            return *tmp;
        }
#else
#    ifdef QL_ENABLE_SINGLETON_THREAD_SAFE_INIT
        // thread safe double checked locking pattern with atomic memory calls
        T* instance = m_instance().load(boost::memory_order_consume);
        if (!instance) {
            boost::mutex::scoped_lock guard(m_mutex());
            instance = m_instance().load(boost::memory_order_consume);
            if (!instance) {
                instance = new T();
                m_instance().store(instance, boost::memory_order_release);
            }
        }
        return *instance;
#    else
        if (m_instance() == nullptr)
            m_instance() = ext::shared_ptr<T>(new T);
        return *m_instance();
#    endif
#endif
    }
}

#endif
