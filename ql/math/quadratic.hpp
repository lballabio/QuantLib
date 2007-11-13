/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

/*! \file quadratic.hpp
    \brief quadratic formula
*/

#ifndef quantlib_quadratic_hpp
#define quantlib_quadratic_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib
{
    class quadratic
    {
      public:
        quadratic(Real a, Real b, Real c);
        Real turningPoint() const;
        Real valueAtTurningPoint() const;
        Real operator()(Real x) const;
        Real discriminant() const;
        // return false if roots not real, and give turning point instead
        bool roots(Real& x, Real& y) const;
      private:
        Real a_, b_, c_;

    };
}

#endif
