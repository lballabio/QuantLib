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

#include <ql/methods/finitedifferences/stepconditions/fdmsnapshotcondition.hpp>

namespace QuantLib {

    FdmSnapshotCondition::FdmSnapshotCondition(Time t)
    : t_(t) {
    }


    void FdmSnapshotCondition::applyTo(Array& a, Time t) const {
        if (t == t_)
            values_ = a;
    }


    Time FdmSnapshotCondition::getTime() const {
        return t_;
    }


    const Array& FdmSnapshotCondition::getValues() const {
        return values_;
    }

}
