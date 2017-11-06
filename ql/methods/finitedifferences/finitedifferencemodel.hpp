/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file finitedifferencemodel.hpp
    \brief generic finite difference model
*/

#ifndef quantlib_finite_difference_model_hpp
#define quantlib_finite_difference_model_hpp

#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/methods/finitedifferences/boundarycondition.hpp>
#include <ql/methods/finitedifferences/operatortraits.hpp>

namespace QuantLib {

    //! Generic finite difference model
    /*! \ingroup findiff */
    template<class Evolver>
    class FiniteDifferenceModel {
      public:
        typedef typename Evolver::traits traits;
        typedef typename traits::operator_type operator_type;
        typedef typename traits::array_type array_type;
        typedef typename traits::bc_set bc_set;
        typedef typename traits::condition_type condition_type;
        // constructors
        FiniteDifferenceModel(const operator_type& L,
                              const bc_set& bcs,
                              const std::vector<Time>& stoppingTimes =
                                                          std::vector<Time>())
            : evolver_(L,bcs), stoppingTimes_(stoppingTimes),
              last_dt_(0.0) {
            std::sort(stoppingTimes_.begin(), stoppingTimes_.end());
            std::vector<Time>::iterator last =
                std::unique(stoppingTimes_.begin(), stoppingTimes_.end());
            stoppingTimes_.erase(last, stoppingTimes_.end());
        }
        FiniteDifferenceModel(const Evolver& evolver,
                              const std::vector<Time>& stoppingTimes =
                                                          std::vector<Time>())
            : evolver_(evolver), stoppingTimes_(stoppingTimes),
              last_dt_(0.0) {
            std::sort(stoppingTimes_.begin(), stoppingTimes_.end());
            std::vector<Time>::iterator last =
                std::unique(stoppingTimes_.begin(), stoppingTimes_.end());
            stoppingTimes_.erase(last, stoppingTimes_.end());
        }
        // methods
        // array_type grid() const { return evolver.xGrid(); }
        const Evolver& evolver() const{ return evolver_; }
        /*! solves the problem between the given times.
            \warning being this a rollback, <tt>from</tt> must be a later
                     time than <tt>to</tt>.
        */
        void rollback(array_type& a,
                      Time from,
                      Time to,
                      Size steps) {
            rollbackImpl(a,from,to,steps,(const condition_type*) 0);
        }
        /*! solves the problem between the given times,
            applying a condition at every step.
            \warning being this a rollback, <tt>from</tt> must be a later
                     time than <tt>to</tt>.
        */
        void rollback(array_type& a,
                      Time from,
                      Time to,
                      Size steps,
                      const condition_type& condition) {
            rollbackImpl(a,from,to,steps,&condition);
        }

        array_type prevValue() const {
            return last_array_;
        }
        Time prevDt() const {
            return last_dt_;
        }
      private:
        /*! Save to calculate theta */
        array_type last_array_;
        Time last_dt_;

        void rollbackImpl(array_type& a,
                          Time from,
                          Time to,
                          Size steps,
                          const condition_type* condition) {

            QL_REQUIRE(from >= to,
                       "trying to roll back from " << from << " to " << to);

            Time dt = (from-to)/steps, t = from;
            evolver_.setStep(dt);

            if(!stoppingTimes_.empty() && stoppingTimes_.back() == from) {
                if (condition)
                    condition->applyTo(a,from);
            }
            for (Size i=0; i<steps; ++i, t -= dt) {
                Time now = t, next = t-dt;
                if (std::fabs(to-next) < std::sqrt(QL_EPSILON)) next = to;
                bool hit = false;
                for (Integer j = static_cast<Integer>(stoppingTimes_.size())-1; j >= 0 ; --j) {
                    if (next <= stoppingTimes_[j] && stoppingTimes_[j] < now) {
                        // a stopping time was hit
                        hit = true;
                        // save last array
                        last_array_ = a;
                        last_dt_ = now-stoppingTimes_[j];

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
        Evolver evolver_;
        std::vector<Time> stoppingTimes_;
    };

}


#endif
