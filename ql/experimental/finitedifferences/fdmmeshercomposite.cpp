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

#include <ql/experimental/finitedifferences/fdmmeshercomposite.hpp>

namespace QuantLib {

    Real FdmMesherComposite::dplus(const FdmLinearOpIterator& iter,
                                   Size direction) const {
        return mesher_[direction]->dplus(iter.coordinates()[direction]);
    }

    Real FdmMesherComposite::dminus(const FdmLinearOpIterator& iter,
                                    Size direction) const {
        return mesher_[direction]->dminus(iter.coordinates()[direction]);
    }

    Real FdmMesherComposite::location(const FdmLinearOpIterator& iter,
                                      Size direction) const {
        return mesher_[direction]->location(iter.coordinates()[direction]);
    }

    Disposable<Array> FdmMesherComposite::locations(Size direction) const {
        Array retVal(layout_->size());

        const FdmLinearOpIterator endIter = layout_->end();
        for (FdmLinearOpIterator iter = layout_->begin();
             iter != endIter; ++iter) {
            retVal[iter.index()] =
                mesher_[direction]->locations()[iter.coordinates()[direction]];
        }

        return retVal;
    }
}
