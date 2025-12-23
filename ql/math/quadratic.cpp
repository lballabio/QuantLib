/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

#include <ql/math/quadratic.hpp>

namespace QuantLib
{

    quadratic::quadratic(Real a, Real b, Real c) : a_(a), b_(b), c_(c) {}

    Real quadratic::turningPoint() const {
        return -b_/(2.0*a_);
    }

    Real quadratic::valueAtTurningPoint() const {
        return (*this)(turningPoint());
    }

    Real quadratic::operator()(Real x) const {
        return x*(x*a_+b_)+c_;
    }

    Real quadratic::discriminant() const {
        return b_*b_-4*a_*c_;
    }

    // return false if roots not real, and give turning point instead
    bool quadratic::roots(Real& x, Real& y) const {
        Real d = discriminant();
        if (d<0) {
            x = y = turningPoint();
            return false;
        }
        d = std::sqrt(d);
        x = (-b_ -  d)/(2*a_);
        y = (-b_ + d)/(2*a_);
        return true;

    }
}
