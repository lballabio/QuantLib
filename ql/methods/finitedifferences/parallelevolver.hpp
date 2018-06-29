/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file parallelevolver.hpp
    \brief Parallel evolver for multiple arrays

    This class takes the evolver class and creates a new class which evolves
    each of the evolvers in parallel.  Part of what this does is to take the 
    types for each evolver class and then wrapper them so that they create
    new types which are sets of the old types.

    This class is intended to be run in situations where there are parallel
    differential equations such as with some convertible bond models.
*/

#ifndef quantlib_system_evolver_hpp
#define quantlib_system_evolver_hpp

#include <ql/methods/finitedifferences/finitedifferencemodel.hpp>
#include <ql/methods/finitedifferences/stepcondition.hpp>
#include <ql/numericalmethod.hpp>
#include <vector>

namespace QuantLib {

    //! Parallel evolver for multiple arrays
    /*! \ingroup findiff */

    template <typename array_type>
    class StepConditionSet {
        typedef ext::shared_ptr<StepCondition<array_type> > itemType;
        std::vector<itemType> stepConditions_;
      public:
        void applyTo(std::vector<array_type>& a, Time t) const {
            //#pragma omp parallel for
            for (Size i=0; i < stepConditions_.size(); i++) {
                stepConditions_[i]->applyTo(a[i], t);
            }
        }
        void push_back(const itemType& a) {
            stepConditions_.push_back(a);
        }
    };

    template <typename bc_set>
    class BoundaryConditionSet {
        std::vector<bc_set> bcSet_;
      public:
        void push_back(const bc_set& a) {
            bcSet_.push_back(a);
        }
        const bc_set& operator[](Size i) const {
            return bcSet_[i];
        }
    };

    template <typename traits>
    class ParallelEvolverTraits {
      public:
        typedef std::vector<typename traits::array_type> array_type;
        typedef std::vector<typename traits::operator_type> operator_type;
        typedef std::vector<typename traits::bc_type> bc_type;
        typedef BoundaryConditionSet<typename traits::bc_set> bc_set;
        typedef StepConditionSet<typename traits::array_type> condition_type;
    };

    template <class Evolver>
    class ParallelEvolver  {
      public:
        // typedefs
        typedef ParallelEvolverTraits<typename Evolver::traits> traits;
        typedef typename traits::operator_type operator_type;
        typedef typename traits::array_type array_type;
        typedef typename traits::bc_set bc_set;
        // constructors
        ParallelEvolver(const operator_type& L,
                        const bc_set& bcs) {
            evolvers_.reserve(L.size());
            for (Size i=0; i < L.size(); i++) {
                evolvers_.push_back(ext::shared_ptr<Evolver>(new
                    Evolver(L[i], bcs[i])));
            }
        }
        void step(array_type& a,
                  Time t) {
            //#pragma omp parallel for
            for (Size i=0; i < evolvers_.size(); i++) {
                evolvers_[i]->step(a[i], t);
            }
        }
        void setStep(Time dt) {
            for (Size i=0; i < evolvers_.size(); i++) {
                evolvers_[i]->setStep(dt);
            }
        }
      private:
        std::vector<ext::shared_ptr<Evolver> > evolvers_;
    };

}


#endif
