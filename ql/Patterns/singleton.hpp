
/*
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file singleton.hpp
    \brief basic support for the singleton pattern
*/

#ifndef quantlib_singleton_hpp
#define quantlib_singleton_hpp

#include <ql/qldefines.hpp>
#include <boost/shared_ptr.hpp>

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

        If the derived class provides an initialize() method, it will
        be called after creation of the unique instance.

        \ingroup patterns
    */
    template <class T>
    class Singleton {
      public:
        //! access to the unique instance
        static T& instance();
      protected:
        Singleton() {}
        void initialize() {}
      private:
        Singleton(const Singleton&) {}
        void operator=(const Singleton&) {}
    };


    // template definitions

    template <class T>
    T& Singleton<T>::instance() {
        static boost::shared_ptr<T> instance_;
        if (!instance_) {
            instance_ = boost::shared_ptr<T>(new T);
            instance_->initialize();
        }
        return *instance_;
    }
        
}


#endif
