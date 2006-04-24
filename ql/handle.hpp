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

/*! \file handle.hpp
    \brief Globally accessible relinkable pointer
*/

#ifndef quantlib_handle_hpp
#define quantlib_handle_hpp

#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    //! Relinkable access to a shared pointer
    /*! \pre Class T must inherit from Observable */
    template <class T>
    class Link : public Observable, public Observer {
      public:
        /*! \warning see the documentation of the linkTo() method for
                     issues relatives to <tt>registerAsObserver</tt>.
        */
        explicit Link(const boost::shared_ptr<T>& h = boost::shared_ptr<T>(),
                      bool registerAsObserver = true);
        /*! \warning <tt>registerAsObserver</tt> is left as a backdoor
                     in case the programmer cannot guarantee that the
                     object pointed to will remain alive for the whole
                     lifetime of the handle---namely, it should be set
                     to <tt>false</tt> when the passed shared pointer
                     was created with <tt>owns = false</tt> (the
                     latter should only happen in a controlled
                     environment, so that the programmer is aware of
                     it). Failure to do so can very likely result in a
                     program crash.  If the programmer does want the
                     handle to register as observer of such a shared
                     pointer, it is his responsibility to ensure that
                     the handle gets destroyed before the pointed
                     object does.
        */
        void linkTo(const boost::shared_ptr<T>&,
                    bool registerAsObserver = true);
        //! Checks if the contained shared pointer points to anything
        bool empty() const { return !h_; }
        //! Returns the contained shared pointer
        const boost::shared_ptr<T>& currentLink() const { return h_; }
        //! Swaps two links
        void swap(Link<T>& other) {
            h_.swap(other.h_);
            std::swap(isObserver_, other.isObserver_);
        }
        //! Observer interface
        void update() { notifyObservers(); }
      private:
        boost::shared_ptr<T> h_;
        bool isObserver_;
    };

    /*! \relates Link */
    template <class T> void swap(Link<T>&, Link<T>&);


    //! Globally accessible relinkable pointer
    /*! An instance of this class can be relinked to another shared
        pointer: such change will be propagated to all the copies of
        the instance.

        \pre Class T must inherit from Observable
    */
    template <class T>
    class Handle {
      private:
        boost::shared_ptr<Link<T> > link_;
      public:
        /*! \warning see the documentation of the Link class for
                     issues relatives to <tt>registerAsObserver</tt>.
        */
        explicit Handle(const boost::shared_ptr<T>& h = boost::shared_ptr<T>(),
                        bool registerAsObserver = true);
        /*! \warning see the documentation of the Link class for
                     issues relatives to <tt>registerAsObserver</tt>.
        */
        void linkTo(const boost::shared_ptr<T>&,
                    bool registerAsObserver = true);
        //! dereferencing
        const boost::shared_ptr<T>& currentLink() const;
        const boost::shared_ptr<T>& operator->() const;
        //! checks if the contained shared pointer points to anything
        bool empty() const;
        //! allows registration as observable
        operator boost::shared_ptr<Observable>() const;
        //! swaps two handles
        void swap(Handle<T>& other) { link_.swap(other.link_); }
        //! equality test
        template <class U>
        bool operator==(const Handle<U>& other) {
            return link_ == other.link_;
        }
        //! disequality test
        template <class U>
        bool operator!=(const Handle<U>& other) {
            return link_ != other.link_;
        }
        //! strict weak ordering
        template <class U>
        bool operator<(const Handle<U>& other) {
            return link_ < other.link_;
        }
    };

    /*! \relates Handle */
    template <class T> void swap(Handle<T>&, Handle<T>&);


    // inline definitions

    template <class T>
    inline Link<T>::Link(const boost::shared_ptr<T>& h,
                         bool registerAsObserver)
    : isObserver_(false) {
        linkTo(h,registerAsObserver);
    }

    template <class T>
    inline void Link<T>::linkTo(const boost::shared_ptr<T>& h,
                                bool registerAsObserver) {
        if ((h != h_) || (isObserver_ != registerAsObserver)) {
            if (bool(h_) && isObserver_) {
                boost::shared_ptr<Observable> obs = h_;
                unregisterWith(obs);
            }
            h_ = h;
            isObserver_ = registerAsObserver;
            if (bool(h_) && isObserver_) {
                boost::shared_ptr<Observable> obs = h_;
                registerWith(obs);
            }
            notifyObservers();
        }
    }

    template <class T>
    inline void swap(Link<T>& l1, Link<T>& l2) {
        l1.swap(l2);
    }


    template <class T>
    inline Handle<T>::Handle(const boost::shared_ptr<T>& h,
                             bool registerAsObserver)
    : link_(new Link<T>(h,registerAsObserver)) {}

    template <class T>
    inline void Handle<T>::linkTo(const boost::shared_ptr<T>& h,
                                  bool registerAsObserver) {
        link_->linkTo(h,registerAsObserver);
    }

    template <class T>
    inline const boost::shared_ptr<T>& Handle<T>::currentLink() const {
        return link_->currentLink();
    }

    template <class T>
    inline const boost::shared_ptr<T>& Handle<T>::operator->() const {
        return link_->currentLink();
    }

    template <class T>
    inline bool Handle<T>::empty() const {
        return link_->empty();
    }

    template <class T>
    inline Handle<T>::operator boost::shared_ptr<Observable>() const {
        return link_;
    }

    template <class T>
    inline void swap(Handle<T>& h1, Handle<T>& h2) {
        h1.swap(h2);
    }

}


#endif
