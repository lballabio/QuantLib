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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmdirichletboundary.hpp>
#include <ql/experimental/finitedifferences/fdmlinearoplayout.hpp>

namespace QuantLib {

    FdmDirichletBoundary::FdmDirichletBoundary(
                            const boost::shared_ptr<FdmMesher>& mesher,
                            Real valueOnBoundary, Size direction,
                            FdmDirichletBoundary::Side side)
    : side_(side),
      valueOnBoundary_(valueOnBoundary) {
                                
        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher->layout();
                                
        std::vector<Size> newDim(layout->dim());
        newDim[direction] = 1;
        const Size hyperSize = std::accumulate(newDim.begin(), newDim.end(),
                                               Size(1), std::multiplies<Size>());
        indicies_.resize(hyperSize);

        Size i=0;
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
            ++iter) {
            if (   (side == Lower && iter.coordinates()[direction] == 0)
                || (side == Upper && iter.coordinates()[direction] 
                                            == layout->dim()[direction]-1)) {

                QL_REQUIRE(hyperSize > i, "index missmatch");
                indicies_[i++] = iter.index();
            }
        }
        
        if (side_ == Lower) {
            xExtreme_ = mesher->locations(direction)[0];
        }
        else if (side_ == Upper) {
            xExtreme_ 
                = mesher->locations(direction)[layout->dim()[direction]-1];
        }
    }

    void FdmDirichletBoundary::applyAfterApplying(Array& rhs) const {
        for (std::vector<Size>::const_iterator iter = indicies_.begin();
             iter != indicies_.end(); ++iter) {
            rhs[*iter] = valueOnBoundary_;
        }
    }
    
    void FdmDirichletBoundary::applyAfterSolving(Array& rhs) const {
        this->applyAfterApplying(rhs);
    }
    
    Real FdmDirichletBoundary::applyAfterApplying(Real x, Real value) const {
        return (   (side_ == Lower && x < xExtreme_) 
                || (side_ == Upper && x > xExtreme_)) ? valueOnBoundary_ : value;
    }
}
