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

#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>

namespace QuantLib {

    Size FdmLinearOpLayout::neighbourhood(const FdmLinearOpIterator& iterator,
                                          Size i, Integer offset) const {
        Size myIndex = iterator.index()
            - iterator.coordinates()[i]*spacing_[i];

        Integer coorOffset = Integer(iterator.coordinates()[i])+offset;
        if (coorOffset < 0) {
            coorOffset=-coorOffset;
        }
        else if (Size(coorOffset) >= dim_[i]) {
            coorOffset = 2*(dim_[i]-1) - coorOffset;
        }
        return myIndex + coorOffset*spacing_[i];
    }

    Size FdmLinearOpLayout::neighbourhood(const FdmLinearOpIterator& iterator,
                                          Size i1, Integer offset1,
                                          Size i2, Integer offset2) const {

        Size myIndex = iterator.index()
            - iterator.coordinates()[i1]*spacing_[i1]
            - iterator.coordinates()[i2]*spacing_[i2];

        Integer coorOffset1 = Integer(iterator.coordinates()[i1])+offset1;
        if (coorOffset1 < 0) {
            coorOffset1=-coorOffset1;
        }
        else if (Size(coorOffset1) >= dim_[i1]) {
            coorOffset1 = 2*(dim_[i1]-1) - coorOffset1;
        }

        Integer coorOffset2 = Integer(iterator.coordinates()[i2])+offset2;
        if (coorOffset2 < 0) {
            coorOffset2=-coorOffset2;
        }
        else if (Size(coorOffset2) >= dim_[i2]) {
            coorOffset2 = 2*(dim_[i2]-1) - coorOffset2;
        }

        return myIndex + coorOffset1*spacing_[i1]+coorOffset2*spacing_[i2];
    }

    // smart but sometimes too slow
    FdmLinearOpIterator FdmLinearOpLayout::iter_neighbourhood(
        const FdmLinearOpIterator& iterator, Size i, Integer offset) const {

        std::vector<Size> coordinates = iterator.coordinates();

        Integer coorOffset = Integer(coordinates[i])+offset;
        if (coorOffset < 0) {
            coorOffset=-coorOffset;
        }
        else if (Size(coorOffset) >= dim_[i]) {
            coorOffset = 2*(dim_[i]-1) - coorOffset;
        }
        coordinates[i] = Size(coorOffset);

        return FdmLinearOpIterator(dim_, coordinates,
                                   index(coordinates));
    }

}
