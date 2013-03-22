/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file FdmAffineModelswapinnervalue.cpp
*/

#include <ql/models/shortrate/twofactormodels/g2.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/methods/finitedifferences/utilities/fdmaffinemodelswapinnervalue.hpp>

namespace QuantLib {

    template <>
    Disposable<Array> FdmAffineModelSwapInnerValue<HullWhite>::getState(
        const boost::shared_ptr<HullWhite>& model, Time t,
        const FdmLinearOpIterator& iter) const {

        Array retVal(1, model->dynamics()->shortRate(t,
                                    mesher_->location(iter, direction_)));
        return retVal;
    }

    template <>
    Disposable<Array> FdmAffineModelSwapInnerValue<G2>::getState(
        const boost::shared_ptr<G2>&, Time,
        const FdmLinearOpIterator& iter) const {

        Array retVal(2);
        retVal[0] = mesher_->location(iter, direction_);
        retVal[1] = mesher_->location(iter, direction_+1);

        return retVal;
    }

}
