
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.3  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file finitedifferencemodel.hpp
    \brief generic finite difference model
*/

#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include "qldefines.hpp"
#include "FiniteDifferences/stepcondition.hpp"
#include "handle.hpp"
#include "null.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        /*  Evolvers do not need to inherit from any base class.
            However, they must implement the following interface:

            class Evolver {
              public:
                typedef ... arrayType;
                typedef ... operatorType;
                // constructors
                Evolver(const operatorType& D);
                // member functions
                void step(arrayType& a, Time t) const;
                void setStep(Time dt);
            };

        */

        template<class Evolver>
        class FiniteDifferenceModel {
          public:
            typedef typename Evolver::arrayType arrayType;
            typedef typename Evolver::operatorType operatorType;
            // constructor
            FiniteDifferenceModel(const operatorType& D) : evolver(D) {}
            // methods
            // arrayType grid() const { return evolver.xGrid(); }
            void rollback(arrayType& a, Time from, Time to, int steps,
              Handle<StepCondition<arrayType> > condition =
                Handle<StepCondition<arrayType> >());
          private:
            Evolver evolver;
        };

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
            FiniteDifferenceModel::arrayType& a, Time from, Time to, int steps,
            Handle<StepCondition<arrayType> > condition)
        {
            // WARNING: it is a rollback: 'from' must be a later time than 'to'!
            Time dt = (from-to)/steps, t = from;
            evolver.setStep(dt);
            for (int i=0; i<steps; i++, t -= dt) {
                evolver.step(a,t);
                if (!condition.isNull())
                    condition->applyTo(a,t);
            }
        }
    }
}


#endif
