
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file observable.hpp
    \fullpath Include/ql/Patterns/%observable.hpp
    \brief observer/observable pattern

*/

// $Id$
// $Log$
// Revision 1.11  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.10  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.9  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/06/21 14:30:43  lballabio
// Observability is back
//
// Revision 1.4  2001/06/20 11:52:30  lballabio
// Some observability is back
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_observable_h
#define quantlib_observable_h

#include "ql/qldefines.hpp"
#include <set>

namespace QuantLib {

    //! Implementations of design patterns
    namespace Patterns {

        /*  These classes are a simplified version of the ones implemented in
            Thinking in C++ which in turn mirror the Java Observer and
            Observable interface
        */

        //! Object that gets notified when a given observable changes
        class Observer {
          public:
            virtual ~Observer() {}
            /*! This method must be implemented in derived classes. An 
                instance of %Observer does not call this method directly: 
                instead, it will be called by the observables the instance 
                registered with when they need to notify any changes.
            */
            virtual void update() = 0;
        };

        //! Object that notifies its changes to a set of observables
        class Observable {
          public:
            virtual ~Observable() {}
            /*! \name Observer management
                \warning It is responsibility of the programmer to make sure 
                that the registered observers unregister themselves before 
                going out of scope.
            */
            //@{
            virtual void registerObserver(Observer*);
            void registerObservers(std::set<Observer*>&);
            virtual void unregisterObserver(Observer*);
            void unregisterObservers(std::set<Observer*>&);
            virtual void unregisterAll();
            std::set<Observer*> observers() const;
            /*! In derived classes, this method should be called at the end 
                of non-const methods or when the programmer desires to notify
                any changes.
            */
            virtual void notifyObservers();
            //@}
          private:
            std::set<Observer*> theObservers;
        };


        // inline definitions

        inline void Observable::registerObserver(Observer* o) {
            theObservers.insert(o);
        }

        inline void Observable::registerObservers(std::set<Observer*>& o) {
            for (std::set<Observer*>::iterator i = o.begin(); i != o.end(); ++i)
                registerObserver(*i);
        }

        inline void Observable::unregisterObserver(Observer* o) {
            theObservers.erase(o);
        }

        inline void Observable::unregisterObservers(std::set<Observer*>& o) {
            for (std::set<Observer*>::iterator i = o.begin(); i != o.end(); ++i)
                unregisterObserver(*i);
        }

        inline void Observable::unregisterAll() {
            theObservers.clear();
        }

        inline std::set<Observer*> Observable::observers() const {
            return theObservers;
        }

        inline void Observable::notifyObservers() {
            for (std::set<Observer*>::iterator i = theObservers.begin();
                i != theObservers.end(); ++i)
                    (*i)->update();
        }

    }

}


#endif
