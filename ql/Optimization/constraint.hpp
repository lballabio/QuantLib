/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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

    \fullpath
    ql/Optimization/%constraint.hpp
*/

// $Id$

#ifndef quantlib_optimization_constraint_h
#define quantlib_optimization_constraint_h

#include "ql/array.hpp"

namespace QuantLib {

    namespace Optimization {


        //! Abstract constraint class
        class Constraint {
          public:

            //! Actual implementation class
            class ConstraintImpl {
              public:
                //! Tests if params satisfy the constraint
                virtual bool test(const Array& params) const = 0;
            };

            Constraint(const Handle<ConstraintImpl>& impl) : impl_(impl) {}
            bool test(const Array& params) const { return impl_->test(params); }
            double update(Array& params, const Array& direction, double beta) {

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

          private:
            Handle<ConstraintImpl> impl_;
        };

        //! No constraint 
        class NoConstraint : public Constraint {
          public:
            class NoConstraintImpl : public Constraint::ConstraintImpl {
              public:
                bool test(const Array& params) const {
                    return true;
                }
            };
            NoConstraint() 
            : Constraint(Handle<ConstraintImpl>(new NoConstraintImpl)) {}
        };

        //! Constraint imposing positivity to all parameters
        class PositiveConstraint : public Constraint {
          public:
            class PositiveConstraintImpl : public Constraint::ConstraintImpl {
              public:
                bool test(const Array& params) const {
                    for (Size i=0; i<params.size(); i++) {
                        if (params[i] <= 0.0)
                            return false;
                    }
                    return true;
                }
            };
            PositiveConstraint() 
            : Constraint(Handle<ConstraintImpl>(new PositiveConstraintImpl)) {}
        };

        //! Constraint imposing all parameters to be in [low,high]
        class BoundaryConstraint : public Constraint {
          public:
            class BoundaryConstraintImpl : public Constraint::ConstraintImpl {
              public:
                BoundaryConstraintImpl(double low, double high) 
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
            BoundaryConstraint(double low, double high) 
            : Constraint(Handle<ConstraintImpl>(
                new BoundaryConstraintImpl(low, high))) {}
        };

    }
}


#endif
