/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file handle.hpp
    \brief Globally accessible relinkable pointer
*/

#ifndef quantlib_handle_hpp
#define quantlib_handle_hpp

#include <ql/patterns/observable.hpp>

namespace QuantLib {

    //! Shared handle to an observable
    /*! All copies of an instance of this class refer to the same
        observable by means of a relinkable smart pointer. When such
        pointer is relinked to another observable, the change will be
        propagated to all the copies.

        \pre Class T must inherit from Observable
    */
    template <class T>
    class Handle {
      protected:
        class Link : public Observable, public Observer {
          public:
            Link(const ext::shared_ptr<T>& h,
                 bool registerAsObserver);
            Link(ext::shared_ptr<T>&& h,
                 bool registerAsObserver);
            void linkTo(ext::shared_ptr<T>,
                        bool registerAsObserver);
            bool empty() const { return !h_; }
            const ext::shared_ptr<T>& currentLink() const { return h_; }
            void update() override { notifyObservers(); }

          private:
            ext::shared_ptr<T> h_;
            bool isObserver_ = false;
        };
        ext::shared_ptr<Link> link_;
      public:
        /*! \name Constructors

            \warning <tt>registerAsObserver</tt> is left as a backdoor
                     in case the programmer cannot guarantee that the
                     object pointed to will remain alive for the whole
                     lifetime of the handle---namely, it should be set
                     to <tt>false</tt> when the passed shared pointer
                     does not own the pointee (this should only happen
                     in a controlled environment, so that the
                     programmer is aware of it). Failure to do so can
                     very likely result in a program crash.  If the
                     programmer does want the handle to register as
                     observer of such a shared pointer, it is his
                     responsibility to ensure that the handle gets
                     destroyed before the pointed object does.
        */
        //@{
        Handle()
        : Handle(ext::shared_ptr<T>()) {}
        explicit Handle(const ext::shared_ptr<T>& p,
                        bool registerAsObserver = true)
        : link_(new Link(p, registerAsObserver)) {}
        explicit Handle(ext::shared_ptr<T>&& p,
                        bool registerAsObserver = true)
        : link_(new Link(std::move(p), registerAsObserver)) {}
        //@}
        //! dereferencing
        const ext::shared_ptr<T>& currentLink() const;
        const ext::shared_ptr<T>& operator->() const;
        const ext::shared_ptr<T>& operator*() const;
        //! checks if the contained shared pointer points to anything
        bool empty() const;
        //! allows registration as observable
        operator ext::shared_ptr<Observable>() const;
        //! equality test
        template <class U>
        bool operator==(const Handle<U>& other) const { return link_==other.link_; }
        //! disequality test
        template <class U>
        bool operator!=(const Handle<U>& other) const { return link_!=other.link_; }
        //! strict weak ordering
        template <class U>
        bool operator<(const Handle<U>& other) const { return link_ < other.link_; }
    };

    //! Relinkable handle to an observable
    /*! An instance of this class can be relinked so that it points to
        another observable. The change will be propagated to all
        handles that were created as copies of such instance.

        \pre Class T must inherit from Observable

        \warning see the Handle documentation for issues
                 relatives to <tt>registerAsObserver</tt>.
    */
    template <class T>
    class RelinkableHandle : public Handle<T> {
      public:
        RelinkableHandle()
        : RelinkableHandle(ext::shared_ptr<T>()) {}
        explicit RelinkableHandle(
                       const ext::shared_ptr<T>& p,
                       bool registerAsObserver = true);
        explicit RelinkableHandle(
                       ext::shared_ptr<T>&& p,
                       bool registerAsObserver = true);
        void linkTo(const ext::shared_ptr<T>& h,
                    bool registerAsObserver = true);
        void linkTo(ext::shared_ptr<T>&& h,
                    bool registerAsObserver = true);
        void reset();
    };


    // inline definitions

    template <class T>
    inline Handle<T>::Link::Link(const ext::shared_ptr<T>& h, bool registerAsObserver) {
        linkTo(h, registerAsObserver);
    }

    template <class T>
    inline Handle<T>::Link::Link(ext::shared_ptr<T>&& h, bool registerAsObserver) {
        linkTo(std::move(h), registerAsObserver);
    }

    template <class T>
    inline void Handle<T>::Link::linkTo(ext::shared_ptr<T> h,
                                        bool registerAsObserver) {
        if ((h != h_) || (isObserver_ != registerAsObserver)) {
            if (h_ && isObserver_)
                unregisterWith(h_);
            h_ = std::move(h);
            isObserver_ = registerAsObserver;
            if (h_ && isObserver_)
                registerWith(h_);
            notifyObservers();
        }
    }


    template <class T>
    inline const ext::shared_ptr<T>& Handle<T>::currentLink() const {
        QL_REQUIRE(!empty(), "empty Handle cannot be dereferenced");
        return link_->currentLink();
    }

    template <class T>
    inline const ext::shared_ptr<T>& Handle<T>::operator->() const {
        QL_REQUIRE(!empty(), "empty Handle cannot be dereferenced");
        return link_->currentLink();
    }

    template <class T>
    inline const ext::shared_ptr<T>& Handle<T>::operator*() const {
        QL_REQUIRE(!empty(), "empty Handle cannot be dereferenced");
        return link_->currentLink();
    }

    template <class T>
    inline bool Handle<T>::empty() const {
        return link_->empty();
    }

    template <class T>
    inline Handle<T>::operator ext::shared_ptr<Observable>() const {
        return link_;
    }


    template <class T>
    inline RelinkableHandle<T>::RelinkableHandle(const ext::shared_ptr<T>& p,
                                                 bool registerAsObserver)
    : Handle<T>(p,registerAsObserver) {}

    template <class T>
    inline RelinkableHandle<T>::RelinkableHandle(ext::shared_ptr<T>&& p,
                                                 bool registerAsObserver)
    : Handle<T>(std::move(p), registerAsObserver) {}

    template <class T>
    inline void RelinkableHandle<T>::linkTo(const ext::shared_ptr<T>& h,
                                            bool registerAsObserver) {
        this->link_->linkTo(h, registerAsObserver);
    }

    template <class T>
    inline void RelinkableHandle<T>::linkTo(ext::shared_ptr<T>&& h,
                                            bool registerAsObserver) {
        this->link_->linkTo(std::move(h), registerAsObserver);
    }

    template <class T>
    inline void RelinkableHandle<T>::reset() {
        this->link_->linkTo(nullptr, true);
    }

}

#endif
