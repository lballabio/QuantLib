/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file linesearch.hpp
    \brief Line search abstract class
*/

#ifndef quantlib_optimization_line_search_h_
#define quantlib_optimization_line_search_h_

#include <ql/Math/array.hpp>

namespace QuantLib {

    class Problem;
    class Constraint;

    //! Base class for line search
    class LineSearch {
      public:
        //! Default constructor
        LineSearch(Real = 0.0)
        : qt_(0.0), qpt_(0.0), succeed_(true) {}
        //! Destructor
        virtual ~LineSearch() {}

        //! return last x value
        const Array& lastX() { return xtd_; }
        //! return last cost function value
        Real lastFunctionValue() { return qt_; }
        //! return last gradient
        const Array& lastGradient() { return gradient_; }
        //! return square norm of last gradient
        Real lastGradientNorm2() { return qpt_;}

        bool succeed() { return succeed_; }

        //! Perform line search
        virtual Real operator()(const Problem& P,
                                Real t_ini) = 0;

        Real update(Array& params,
                    const Array& direction,
                    Real beta,
                    const Constraint& constraint);

        //! current value of the search direction
        const Array& searchDirection() const { return searchDirection_; }
        Array& searchDirection() { return searchDirection_; }
      protected:
        //! current values of the search direction
        Array searchDirection_;
        //! new x and its gradient
        Array xtd_, gradient_;
        //! cost function value and gradient norm corresponding to xtd_
        Real qt_, qpt_;
        //! flag to know if linesearch succeed
        bool succeed_;

    };
}

#endif
