

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
/*! \file boundarycondition.hpp
    \brief boundary conditions for differential operators

    \fullpath
    ql/FiniteDifferences/%boundarycondition.hpp
*/

// $Id$

#ifndef quantlib_boundary_condition_h
#define quantlib_boundary_condition_h

#include <ql/null.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! Boundary condition for finite difference problems
        /*! Three possibilities are given for setting boundary conditions,
            namely, no boundary condition, Dirichlet boundary condition
            (i.e., constant value), and Neumann boundary condition (i.e.,
            constant derivative).

            \warning For Neumann conditions. the value passed must not be the
            value of the derivative. Instead, it must be comprehensive of the
            grid step between the first two points--i.e., it must be the
            difference between f[0] and f[1].
        */
        class BoundaryCondition {
          public:
            // enumeration
            enum Type { None, Neumann, Dirichlet };
            // constructors
            BoundaryCondition(Type type = None, double value = Null<double>())
            : type_(type), value_(value) {
                if (type_ != None)
                    QL_REQUIRE(value != Null<double>(),
                        "A value must be supplied for "
                        "this type of boundary condition");
            }
            // access methods
            Type type() const { return type_; }
            double value() const { return value_; }
          private:
            Type type_;
            double value_;
        };

    }

}


#endif
