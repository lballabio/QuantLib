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

#include <ql/methods/finitedifferences/boundarycondition.hpp>

namespace QuantLib {

    NeumannBC::NeumannBC(Real value, NeumannBC::Side side)
    : value_(value), side_(side) {}

    void NeumannBC::applyBeforeApplying(TridiagonalOperator& L) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(-1.0,1.0);
            break;
          case Upper:
            L.setLastRow(-1.0,1.0);
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void NeumannBC::applyAfterApplying(Array& u) const {
        switch (side_) {
          case Lower:
            u[0] = u[1] - value_;
            break;
          case Upper:
            u[u.size()-1] = u[u.size()-2] + value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void NeumannBC::applyBeforeSolving(TridiagonalOperator& L,
                                       Array& rhs) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(-1.0,1.0);
            rhs[0] = value_;
            break;
          case Upper:
            L.setLastRow(-1.0,1.0);
            rhs[rhs.size()-1] = value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void NeumannBC::applyAfterSolving(Array&) const {}



    DirichletBC::DirichletBC(Real value, DirichletBC::Side side)
    : value_(value), side_(side) {}

    void DirichletBC::applyBeforeApplying(TridiagonalOperator& L) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(1.0,0.0);
            break;
          case Upper:
            L.setLastRow(0.0,1.0);
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void DirichletBC::applyAfterApplying(Array& u) const {
        switch (side_) {
          case Lower:
            u[0] = value_;
            break;
          case Upper:
            u[u.size()-1] = value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void DirichletBC::applyBeforeSolving(TridiagonalOperator& L,
                                         Array& rhs) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(1.0,0.0);
            rhs[0] = value_;
            break;
          case Upper:
            L.setLastRow(0.0,1.0);
            rhs[rhs.size()-1] = value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void DirichletBC::applyAfterSolving(Array&) const {}

}
