
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
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

namespace QuantLib {

    //! Generic finite difference model
    /*! See sect. \ref findiff

        \ingroup findiff
    */
    template<class Evolver>
    class FiniteDifferenceModel {
      public:
        typedef typename Evolver::arrayType arrayType;
        typedef typename Evolver::operatorType operatorType;
        typedef BoundaryCondition<operatorType> bcType;
        typedef StepCondition<arrayType> conditionType;
        // constructors
        FiniteDifferenceModel(
                           const operatorType& L,
                           const std::vector<boost::shared_ptr<bcType> >& bcs,
                           const std::vector<Time>& stoppingTimes =
                                                          std::vector<Time>())
        : evolver_(L,bcs), stoppingTimes_(stoppingTimes) {
            std::sort(stoppingTimes_.begin(), stoppingTimes_.end());
            std::vector<Time>::iterator last =
                std::unique(stoppingTimes_.begin(), stoppingTimes_.end());
            stoppingTimes_.erase(last, stoppingTimes_.end());
        }
        FiniteDifferenceModel(const Evolver& evolver,
                              const std::vector<Time>& stoppingTimes =
                                                          std::vector<Time>())
        : evolver_(evolver), stoppingTimes_(stoppingTimes) {
            std::sort(stoppingTimes_.begin(), stoppingTimes_.end());
            std::vector<Time>::iterator last =
                std::unique(stoppingTimes_.begin(), stoppingTimes_.end());
            stoppingTimes_.erase(last, stoppingTimes_.end());
        }
        // methods
        // arrayType grid() const { return evolver.xGrid(); }
        const Evolver& evolver() const{ return evolver_; }
        /*! solves the problem between the given times, possibly
          applying a condition at every step.
          \warning being this a rollback, <tt>from</tt> must be a later
          time than <tt>to</tt>.
        */
        void rollback(arrayType& a,
                      Time from,
                      Time to,
                      Size steps,
                      const boost::shared_ptr<conditionType>& condition =
                                         boost::shared_ptr<conditionType>()) {

            Time dt = (from-to)/steps, t = from;
            evolver_.setStep(dt);

            for (Size i=0; i<steps; i++, t -= dt) {
                Time now = t, next = t-dt;
                bool hit = false;
                for (Integer j = stoppingTimes_.size()-1; j >= 0 ; j--) {
                    if (next <= stoppingTimes_[j] && stoppingTimes_[j] < now) {
                        // a stopping time was hit
                        hit = true;

                        // perform a small step to stoppingTimes_[j]...
                        evolver_.setStep(now-stoppingTimes_[j]);
                        evolver_.step(a,now);
                        if (condition)
                            condition->applyTo(a,stoppingTimes_[j]);
                        // ...and continue the cycle
                        now = stoppingTimes_[j];
                    }
                }
                // if we did hit...
                if (hit) {
                    // ...we might have to make a small step to
                    // complete the big one...
                    if (now > next) {
                        evolver_.setStep(now - next);
                        evolver_.step(a,now);
                        if (condition)
                            condition->applyTo(a,next);
                    }
                    // ...and in any case, we have to reset the
                    // evolver to the default step.
                    evolver_.setStep(dt);
                } else {
                    // if we didn't, the evolver is already set to the
                    // default step, which is ok for us.
                    evolver_.step(a,now);
                    if (condition)
                        condition->applyTo(a, next);
                }
            }
        }
      private:
        Evolver evolver_;
        std::vector<Time> stoppingTimes_;
    };

}


#endif
