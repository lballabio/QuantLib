
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file constraint.hpp
    \brief Abstract constraint class
*/

#ifndef quantlib_optimization_constraint_h
#define quantlib_optimization_constraint_h

#include <ql/Math/array.hpp>
#include <ql/Patterns/bridge.hpp>

namespace QuantLib {

    //! Base class for constraint implementations
    class ConstraintImpl {
      public:
        virtual ~ConstraintImpl() {}
        //! Tests if params satisfy the constraint
        virtual bool test(const Array& params) const = 0;
    };

    //! Base constraint class
    class Constraint : public Bridge<Constraint,ConstraintImpl> {
      public:
        bool test(const Array& p) const { return impl_->test(p); }
        Real update(Array& p, const Array& direction, Real beta);
        Constraint(const boost::shared_ptr<ConstraintImpl>& impl =
                                      boost::shared_ptr<ConstraintImpl>());
    };

    //! No constraint
    class NoConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            bool test(const Array&) const {
                return true;
            }
        };
      public:
        NoConstraint()
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                                   new NoConstraint::Impl)) {}
    };

    //! %Constraint imposing positivity to all arguments
    class PositiveConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            bool test(const Array& params) const {
                for (Size i=0; i<params.size(); i++) {
                    if (params[i] <= 0.0)
                        return false;
                }
                return true;
            }
        };
      public:
        PositiveConstraint()
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                             new PositiveConstraint::Impl)) {}
    };

    //! %Constraint imposing all arguments to be in [low,high]
    class BoundaryConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            Impl(Real low, Real high)
            : low_(low), high_(high) {}
            bool test(const Array& params) const {
                for (Size i=0; i<params.size(); i++) {
                    if ((params[i] < low_) || (params[i] > high_))
                        return false;
                }
                return true;
            }
          private:
            Real low_, high_;
        };
      public:
        BoundaryConstraint(Real low, Real high)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                  new BoundaryConstraint::Impl(low, high))) {}
    };

    //! %Constraint enforcing both given sub-constraints
    class CompositeConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
          public:
            Impl(const Constraint& c1, const Constraint& c2)
            : c1_(c1), c2_(c2) {}
            bool test(const Array& params) const {
                return c1_.test(params) && c2_.test(params);
            }
          private:
            Constraint c1_, c2_;
        };
      public:
        CompositeConstraint(const Constraint& c1, const Constraint& c2)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                     new CompositeConstraint::Impl(c1,c2))) {}
    };


    // inline definitions

    inline Constraint::Constraint(
                              const boost::shared_ptr<ConstraintImpl>& impl)
    : Bridge<Constraint,ConstraintImpl>(impl) {}

    inline Real Constraint::update(Array& params, const Array& direction,
                                   Real beta) {

        Real diff=beta;
        Array newParams = params + diff*direction;
        bool valid = test(newParams);
        Integer icount = 0;
        while (!valid) {
            if (icount > 200)
                QL_FAIL("can't update parameter vector");
            diff *= 0.5;
            icount ++;

            newParams = params + diff*direction;
            valid = test(newParams);
        }

        params += diff*direction;
        return diff;
    }

}


#endif
