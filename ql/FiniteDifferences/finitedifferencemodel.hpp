
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
#include <ql/FiniteDifferences/boundarycondition.hpp>
#include <ql/handle.hpp>
#include <ql/null.hpp>
#include <vector>

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
            typedef BoundaryCondition<operatorType> bcType;
            // constructor
            FiniteDifferenceModel(const operatorType& L,
                                  const std::vector<Handle<bcType> >& bcs) 
            : evolver_(L,bcs) {}
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
            Evolver evolver_;
        };

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
            FiniteDifferenceModel::arrayType& a,
            Time from, Time to, Size steps,
            Handle<StepCondition<arrayType> > condition) {
                Time dt = (from-to)/steps, t = from;
                evolver_.setStep(dt);
                for (Size i=0; i<steps; i++, t -= dt) {
                    evolver_.step(a,t);
                    if (!condition.isNull())
                        condition->applyTo(a,t);
            }
        }
    }
}


#endif
