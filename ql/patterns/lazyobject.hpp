/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file lazyobject.hpp
    \brief framework for calculation on demand and result caching
*/

#ifndef quantlib_lazy_object_h
#define quantlib_lazy_object_h

#include <ql/patterns/observable.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    //! Framework for calculation on demand and result caching.
    /*! \ingroup patterns */
    class LazyObject : public virtual Observable,
                       public virtual Observer {
      public:
        LazyObject();
        ~LazyObject() override = default;
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        /*! \name Calculations
            These methods do not modify the structure of the object
            and are therefore declared as <tt>const</tt>. Data members
            which will be calculated on demand need to be declared as
            mutable.
        */
        //@{
        /*! This method force the recalculation of any results which
            would otherwise be cached. It is not declared as
            <tt>const</tt> since it needs to call the
            non-<tt>const</tt> <i><b>notifyObservers</b></i> method.

            \note Explicit invocation of this method is <b>not</b>
                  necessary if the object registered itself as
                  observer with the structures on which such results
                  depend.  It is strongly advised to follow this
                  policy when possible.
        */
        void recalculate();
        /*! This method constrains the object to return the presently
            cached results on successive invocations, even if
            arguments upon which they depend should change.
        */
        void freeze();
        /*! This method reverts the effect of the <i><b>freeze</b></i>
            method, thus re-enabling recalculations.
        */
        void unfreeze();

      protected:
        /*! This method performs all needed calculations by calling
            the <i><b>performCalculations</b></i> method.

            \warning Objects cache the results of the previous
                     calculation. Such results will be returned upon
                     later invocations of
                     <i><b>calculate</b></i>. When the results depend
                     on arguments which could change between
                     invocations, the lazy object must register itself
                     as observer of such objects for the calculations
                     to be performed again when they change.

            \warning Should this method be redefined in derived
                     classes, LazyObject::calculate() should be called
                     in the overriding method.
        */
        virtual void calculate() const;
        /*! This method must implement any calculations which must be
            (re)done in order to calculate the desired results.
        */
        virtual void performCalculations() const = 0;
        //@}

      public:
        //! \name Notification settings
        //@{
        /*! This method causes the object to forward the first notification received,
            and discard the others until recalculated; the rationale is that observers
            were already notified, and don't need further notifications until they
            recalculate, at which point this object would be recalculated too.
            After recalculation, this object would again forward the first notification
            received.

            Although not always correct, this behavior is a lot faster
            and thus is the current default.  The default can be
            changed at compile time, or at at run time by calling
            `LazyObjectSettings::instance().alwaysForwardNotifications()`;
            the run-time change won't affect lazy objects already created.
        */
        void forwardFirstNotificationOnly();

        /*! This method causes the object to forward all notifications received.

            Although safer, this behavior is a lot slower and thus
            usually not the default.  The default can be changed at
            compile time, or at run-time by calling
            `LazyObjectSettings::instance().alwaysForwardNotifications()`;
            the run-time change won't affect lazy objects already
            created.
        */
        void alwaysForwardNotifications();
        //@}

      protected:
        mutable bool calculated_ = false, frozen_ = false, alwaysForward_;
       private:
        bool updating_ = false;
        class UpdateChecker {  // NOLINT(cppcoreguidelines-special-member-functions)
            LazyObject* subject_;
          public:
            explicit UpdateChecker(LazyObject* subject) : subject_(subject) {
                subject_->updating_ = true;
            }
            ~UpdateChecker() {
                subject_->updating_ = false;
            }
        };
    };

    //! Per-session settings for the LazyObject class
    class LazyObjectSettings : public Singleton<LazyObjectSettings> {
        friend class Singleton<LazyObjectSettings>;
      private:
        LazyObjectSettings() = default;

      public:
        /*! by default, lazy objects created after calling this method
            will only forward the first notification after successful
            recalculation; see
            LazyObject::forwardFirstNotificationOnly for details.
        */
        void forwardFirstNotificationOnly() {
            forwardsAllNotifications_ = false;
        }

        /*! by default, lazy objects created after calling this method
            will always forward notifications; see
            LazyObject::alwaysForwardNotifications for details.
        */
        void alwaysForwardNotifications() {
            forwardsAllNotifications_ = true;
        }

        //! returns the current default
        bool forwardsAllNotifications() const {
            return forwardsAllNotifications_;
        }

      private:
        bool forwardsAllNotifications_ = false;
    };

    // inline definitions

    inline LazyObject::LazyObject()
    : alwaysForward_(LazyObjectSettings::instance().forwardsAllNotifications()) {}

    inline void LazyObject::update() {
        if (updating_) {
            #ifdef QL_THROW_IN_CYCLES
            QL_FAIL("recursive notification loop detected; you probably created an object cycle");
            #else
            return;
            #endif
        }

        // This sets updating to true (so the above check breaks the
        // infinite loop if we enter this method recursively) and will
        // set it back to false when we exit this scope, either
        // successfully or because of an exception.
        UpdateChecker checker(this);

        // forwards notifications only the first time
        if (calculated_ || alwaysForward_) {
            // set to false early
            // 1) to prevent infinite recursion
            // 2) otherways non-lazy observers would be served obsolete
            //    data because of calculated_ being still true
            calculated_ = false;
            // observers don't expect notifications from frozen objects
            if (!frozen_)
                notifyObservers();
                // exiting notifyObservers() calculated_ could be
                // already true because of non-lazy observers
        }
    }

    inline void LazyObject::recalculate() {
        bool wasFrozen = frozen_;
        calculated_ = frozen_ = false;
        try {
            calculate();
        } catch (...) {
            frozen_ = wasFrozen;
            notifyObservers();
            throw;
        }
        frozen_ = wasFrozen;
        notifyObservers();
    }

    inline void LazyObject::freeze() {
        frozen_ = true;
    }

    inline void LazyObject::unfreeze() {
        // send notifications, just in case we lost any,
        // but only once, i.e. if it was frozen
        if (frozen_) {
            frozen_ = false;
            notifyObservers();
        }
    }

    inline void LazyObject::forwardFirstNotificationOnly() {
        alwaysForward_ = false;
    }

    inline void LazyObject::alwaysForwardNotifications() {
        alwaysForward_ = true;
    }

    inline void LazyObject::calculate() const {
        if (!calculated_ && !frozen_) {
            calculated_ = true;   // prevent infinite recursion in
                                  // case of bootstrapping
            try {
                performCalculations();
            } catch (...) {
                calculated_ = false;
                throw;
            }
        }
    }
}

#endif
