/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdmdirichletboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmindicesonboundary.hpp>

namespace QuantLib {

    FdmDirichletBoundary::FdmDirichletBoundary(
                            const ext::shared_ptr<FdmMesher>& mesher,
                            Real valueOnBoundary, Size direction,
                            FdmDirichletBoundary::Side side)
    : side_(side),
      valueOnBoundary_(valueOnBoundary),
      indices_(FdmIndicesOnBoundary(mesher->layout(),
                                    direction, side).getIndices()) {

        if (side_ == Lower) {
            xExtreme_ = mesher->locations(direction)[0];
        }
        else if (side_ == Upper) {
            xExtreme_ = mesher
                ->locations(direction)[mesher->layout()->dim()[direction]-1];
        }
        else {
            QL_FAIL("internal error");
        }
    }

    void FdmDirichletBoundary::applyBeforeApplying(operator_type&) const {
    }

    void FdmDirichletBoundary::applyAfterApplying(Array& x) const {
        for (unsigned long indice : indices_) {
            x[indice] = valueOnBoundary_;
        }
    }
    
    void FdmDirichletBoundary::applyBeforeSolving(operator_type&,
                                                  array_type&) const {
    }

    void FdmDirichletBoundary::applyAfterSolving(Array& rhs) const {
        this->applyAfterApplying(rhs);
    }
    
    Real FdmDirichletBoundary::applyAfterApplying(Real x, Real value) const {
        return (   (side_ == Lower && x < xExtreme_) 
                || (side_ == Upper && x > xExtreme_)) ? valueOnBoundary_ : value;
    }
}
