
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file finitedifferencemodel.hpp
    \brief generic finite difference model

    \fullpath
    ql/FiniteDifferences/%finitedifferencemodel.hpp
*/

// $Id$

#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include <ql/FiniteDifferences/stepcondition.hpp>
#include <ql/handle.hpp>
#include <ql/null.hpp>

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
              Size steps,
              Handle<StepCondition<arrayType> > condition =
                Handle<StepCondition<arrayType> >());
          private:
            Evolver evolver;
        };

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
            FiniteDifferenceModel::arrayType& a,
            Time from, Time to, Size steps,
            Handle<StepCondition<arrayType> > condition) {
                Time dt = (from-to)/steps, t = from;
                evolver.setStep(dt);
                for (Size i=0; i<steps; i++, t -= dt) {
                    evolver.step(a,t);
                    if (!condition.isNull())
                        condition->applyTo(a,t);
            }
        }
    }
}


#endif
