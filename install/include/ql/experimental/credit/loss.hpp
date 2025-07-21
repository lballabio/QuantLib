/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file loss.hpp
  \brief Pair of loss time and amount, sortable by loss time
*/

#ifndef quantlib_loss_hpp
#define quantlib_loss_hpp

#include <ql/types.hpp>

namespace QuantLib {

class Loss {
    public:
        Loss(Real t = 0.0, Real a = 0.0) : time(t), amount(a) {};
        Real time, amount;
    };

    inline bool operator<(const Loss& l1, const Loss& l2) {
        return (l1.time < l2.time);
    }
    inline bool operator>(const Loss& l1, const Loss& l2) {
        return (l1.time > l2.time);
    }
    inline bool operator==(const Loss& l1, const Loss& l2) {
        return (l1.time == l2.time);
    }
    inline bool operator!=(const Loss& l1, const Loss& l2) {
        return (l1.time != l2.time);
    }

}

#endif
