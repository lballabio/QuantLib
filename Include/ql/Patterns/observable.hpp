
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file observable.hpp
    \brief observer/observable pattern

    $Source$
    $Log$
    Revision 1.2  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.1  2001/04/09 14:05:48  nando
    all the *.hpp moved below the Include/ql level

*/

#ifndef quantlib_observable_h
#define quantlib_observable_h

#include "ql/qldefines.hpp"
#include <set>

namespace QuantLib {

    namespace Patterns {

        /*  These classes are a simplified version of the ones implemented in
            Thinking in C++ which in turn mirror the Java Observer and
            Observable interface
        */

        class Observer {
          public:
            virtual ~Observer() {}
            virtual void update() = 0;
        };

        class Observable {
          public:
            virtual ~Observable() {}
            virtual void registerObserver(Observer*);
            void registerObservers(std::set<Observer*>&);
            virtual void unregisterObserver(Observer*);
            void unregisterObservers(std::set<Observer*>&);
            virtual void unregisterAll();
            virtual void notifyObservers();
            std::set<Observer*> observers() const;
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

        inline void Observable::notifyObservers() {
            for (std::set<Observer*>::iterator i = theObservers.begin();
                i != theObservers.end(); ++i)
                    (*i)->update();
        }

        inline std::set<Observer*> Observable::observers() const {
            return theObservers;
        }

    }

}


#endif
