
/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file parallelevolver.hpp
    \brief Parallel evolver for multiple arrays
*/

#ifndef quantlib_system_evolver_hpp
#define quantlib_system_evolver_hpp

#include <ql/FiniteDifferences/finitedifferencemodel.hpp>
#include <ql/FiniteDifferences/stepcondition.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/numericalmethod.hpp>
#include <vector>

namespace QuantLib {

    //! Parallel evolver for multiple arrays
    /*! \ingroup findiff */

    template <typename array_type>
    class StepConditionSet {
        typedef boost::shared_ptr<StepCondition<array_type> > itemType;
        std::vector<itemType> stepConditions_;
      public:
        void applyTo(std::vector<array_type>& a, Time t) const {
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
            for (Size i=0; i < L.size(); i++) {
                evolvers_.push_back(boost::shared_ptr<Evolver>(
                                                  new Evolver(L[i], bcs[i])));
            }
        }
        void step(array_type& a,
                  Time t) {
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
        std::vector<boost::shared_ptr<Evolver> > evolvers_;
    };

};


#endif
