/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file constraint.hpp
    \brief Abstract constraint class
*/

#ifndef quantlib_optimization_constraint_h
#define quantlib_optimization_constraint_h

#include <ql/math/array.hpp>

namespace QuantLib {

    //! Base constraint class
    class Constraint {
      protected:
        //! Base class for constraint implementations
        class Impl {
          public:
            virtual ~Impl() {}
            //! Tests if params satisfy the constraint
            virtual bool test(const Array& params) const = 0;
        };
        boost::shared_ptr<Impl> impl_;
      public:
        bool empty() const { return !impl_; }
        bool test(const Array& p) const { return impl_->test(p); }
        Real update(Array& p,
                    const Array& direction,
                    Real beta);
        Constraint(const boost::shared_ptr<Impl>& impl =
                                                   boost::shared_ptr<Impl>());
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
                for (Size i=0; i<params.size(); ++i) {
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
            Impl(const Constraint& c1,
                 const Constraint& c2)
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

}

#endif
