
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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
*/

#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include <ql/FiniteDifferences/stepcondition.hpp>
#include <ql/FiniteDifferences/boundarycondition.hpp>
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
            // constructors
            FiniteDifferenceModel(
                const operatorType& L,
                const std::vector<Handle<bcType> >& bcs,
                const std::vector<Time>& stoppingTimes=std::vector<Time>())
            : evolver_(L,bcs), stoppingTimes_(stoppingTimes) {}
            FiniteDifferenceModel(
                const Evolver& evolver,
                const std::vector<Time>& stoppingTimes=std::vector<Time>())
            : evolver_(evolver), stoppingTimes_(stoppingTimes) {}
            // methods
            // arrayType grid() const { return evolver.xGrid(); }
            /*! solves the problem between the given times, possibly
                applying a condition at every step.
                \warning being this a rollback, <tt>from</tt> must be a later
                time than <tt>to</tt>.
            */
            void rollback(arrayType& a,
                          Time from,
                          Time to,
                          Size steps,
                          const Handle<StepCondition<arrayType> >& condition =
                                     Handle<StepCondition<arrayType> >());
            const Evolver& evolver() const{return evolver_;};
          private:
            Evolver evolver_;
            std::vector<Time> stoppingTimes_;
        };

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
            typename FiniteDifferenceModel::arrayType& a,
            Time from, Time to, Size steps,
            const Handle<StepCondition<arrayType> >& condition) {
                Time dt = (from-to)/steps, t = from;
                evolver_.setStep(dt);

                for (Size i=0; i<steps; i++, t -= dt) {
                    Size j;
                    for(j=0; j < stoppingTimes_.size(); j++)
                        if(t-dt <= stoppingTimes_[j] && stoppingTimes_[j] < t)
                            break;
                    if(j == stoppingTimes_.size())
                    {   // No stopping time was hit
                        evolver_.step(a,t);
                        if (!condition.isNull())
                            condition->applyTo(a, t-dt);
                    }
                    else
                    {   // A stopping time was hit

                        // First baby step from t to stoppingTimes_[j]
                        evolver_.setStep(t-stoppingTimes_[j]);
                        evolver_.step(a,t);
                        if (!condition.isNull())
                            condition->applyTo(a,stoppingTimes_[j]);

                        // Second baby step from stoppingTimes_[j] to t-dt
                        evolver_.setStep(stoppingTimes_[j] - (t-dt));
                        evolver_.step(a,stoppingTimes_[j]);
                        if (!condition.isNull())
                            condition->applyTo(a,t-dt);

                        evolver_.setStep(dt);
                    }
            }
        }
    }
}


#endif
