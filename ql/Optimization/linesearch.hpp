
/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file linesearch.hpp
    \brief Line search abstract class
*/

#ifndef quantlib_optimization_line_search_h_
#define quantlib_optimization_line_search_h_

#include <ql/Optimization/problem.hpp>

namespace QuantLib {

    //! Base class for line search
    class LineSearch {
      public:
        //! Default constructor
        LineSearch(double eps = 1e-8)
        : qt_(0.0), qpt_(0.0), succeed_(true) {}
        //! Destructor
        virtual ~LineSearch() {}

        //! return last x value
        const Array& lastX() { return xtd_; }
        //! return last cost function value
        double lastFunctionValue() { return qt_; }
        //! return last gradient
        const Array& lastGradient() { return gradient_; }
        //! return square norm of last gradient
        double lastGradientNorm2() { return qpt_;}

        bool succeed() { return succeed_; }

        //! Perform line search
        virtual double operator()(const Problem& P, double t_ini) = 0;

        double update(Array& params, 
                      const Array& direction,
                      double beta,
                      const Constraint& constraint);

      protected:
        //! new x and its gradient
        Array xtd_, gradient_;
        //! cost function value and gradient norm corresponding to xtd_
        double qt_, qpt_;
        //! flag to know if linesearch succeed
        bool succeed_;

    };


    inline double LineSearch::update(Array& params, const Array& direction, 
                                     double beta,
                                     const Constraint& constraint) {

        double diff=beta;

        Array newParams = params + diff*direction;
        bool valid = constraint.test(newParams);
        int icount = 0;
        while (!valid) {
            if (icount > 200)
                QL_FAIL("Can't update linesearch");
            diff *= 0.5;
            icount ++;

            newParams = params + diff*direction;
            valid = constraint.test(newParams);
        }

        params += diff*direction;
        return diff;
    }

}


#endif
