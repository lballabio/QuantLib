/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.

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

#include <ql/processes/forwardmeasureprocess.hpp>

namespace QuantLib {

    // base class

    ForwardMeasureProcess::ForwardMeasureProcess(
                                const ext::shared_ptr<discretization>& disc)
    : StochasticProcess(disc) {}

    void ForwardMeasureProcess::setForwardMeasureTime(Time T) {
        T_ = T;
        notifyObservers();
    }

    Time ForwardMeasureProcess::getForwardMeasureTime() const {
        return T_;
    }

    // 1-D specialization

    ForwardMeasureProcess1D::ForwardMeasureProcess1D(
                                const ext::shared_ptr<discretization>& disc)
    : StochasticProcess1D(disc) {}

    void ForwardMeasureProcess1D::setForwardMeasureTime(Time T) {
        T_ = T;
        notifyObservers();
    }

    Time ForwardMeasureProcess1D::getForwardMeasureTime() const {
        return T_;
    }
}

