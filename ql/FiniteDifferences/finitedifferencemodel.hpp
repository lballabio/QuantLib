
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

/*! \file finitedifferencemodel.hpp
    \brief generic finite difference model

    \fullpath
    ql/FiniteDifferences/%finitedifferencemodel.hpp
*/

// $Id$

#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include "ql/FiniteDifferences/stepcondition.hpp"
#include "ql/handle.hpp"
#include "ql/null.hpp"

namespace QuantLib {

    //! Finite difference framework
    /*! See sect. \ref findiff */
    namespace FiniteDifferences {

        //! Generic finite difference model
        /*! See sect. \ref findiff */
        template<class Evolver>
        class FiniteDifferenceModel {
          public:
            typedef typename Evolver::arrayType arrayType;
            typedef typename Evolver::operatorType operatorType;
            // constructor
            FiniteDifferenceModel(const operatorType& L) : evolver(L) {}
            // methods
            // arrayType grid() const { return evolver.xGrid(); }
            /*! solves the problem between the given times, possibly 
                applying a condition at every step.
                \warning being this a rollback, <tt>from</tt> must be a later 
                time than <tt>to</tt>.
            */
            void rollback(arrayType& a, Time from, Time to, 
              unsigned int steps, 
              Handle<StepCondition<arrayType> > condition =
                Handle<StepCondition<arrayType> >());
          private:
            Evolver evolver;
        };

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
            FiniteDifferenceModel::arrayType& a,
            Time from, Time to, unsigned int steps,
            Handle<StepCondition<arrayType> > condition) {
                Time dt = (from-to)/steps, t = from;
                evolver.setStep(dt);
                for (unsigned int i=0; i<steps; i++, t -= dt) {
                    evolver.step(a,t);
                    if (!condition.isNull())
                        condition->applyTo(a,t);
            }
        }
    }
}


#endif
