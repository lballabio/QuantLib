
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file constraint.hpp
    \brief Abstract constraint class
*/

// $Id$

#ifndef quantlib_optimization_constraint_h
#define quantlib_optimization_constraint_h

#include <ql/array.hpp>
#include <ql/Patterns/bridge.hpp>

namespace QuantLib {

    namespace Optimization {

        //! Base class for Constraint implementations
        class ConstraintImpl {
          public:
            virtual ~ConstraintImpl() {}
            //! Tests if params satisfy the constraint
            virtual bool test(const Array& params) const = 0;
        };

        //! Base constraint class
        class Constraint : public Patterns::Bridge<Constraint,ConstraintImpl> {
          public:
            bool test(const Array& p) const { return impl_->test(p); }
            double update(Array& p, const Array& direction, double beta);
          protected:
            Constraint(const Handle<ConstraintImpl>& impl);
        };

        //! No constraint 
        class NoConstraint : public Constraint {
          private:
            class Impl : public Constraint::Impl {
              public:
                bool test(const Array& params) const {
                    return true;
                }
            };
          public:
            NoConstraint() 
            : Constraint(Handle<Constraint::Impl>(new NoConstraint::Impl)) {}
        };

        //! Constraint imposing positivity to all arguments
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
            : Constraint(Handle<Constraint::Impl>(
                 new PositiveConstraint::Impl)) {}
        };

        //! Constraint imposing all arguments to be in [low,high]
        class BoundaryConstraint : public Constraint {
          private:
            class Impl : public Constraint::Impl {
              public:
                Impl(double low, double high) 
                : low_(low), high_(high) {}
                bool test(const Array& params) const {
                    for (Size i=0; i<params.size(); i++) {
                        if ((params[i] < low_) || (params[i] > high_))
                            return false;
                    }
                    return true; 
                }
              private:
                double low_, high_;
            };
          public:
            BoundaryConstraint(double low, double high) 
            : Constraint(Handle<Constraint::Impl>(
                new BoundaryConstraint::Impl(low, high))) {}
        };

        // inline definitions

        inline Constraint::Constraint(const Handle<Constraint::Impl>& impl)
        : Patterns::Bridge<Constraint,ConstraintImpl>(impl) {}

        inline double Constraint::update(
            Array& params, const Array& direction, double beta) {

            double diff=beta;
            Array newParams = params + diff*direction;
            bool valid = test(newParams);
            int icount = 0;
            while (!valid) {
                if (icount > 200)
                    throw Error("Can't update parameter vector");
                diff *= 0.5;
                icount ++;
        
                newParams = params + diff*direction;
                valid = test(newParams);
            }

            params += diff*direction;
            return diff;
        }

    }
}


#endif
