

/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file minimizer.hpp
    \brief Abstract 1-D solver class

    \fullpath
    ql/%minimizer.hpp
*/

// $Id$

#ifndef quantlib_minimizer_h
#define quantlib_minimizer_h

#include "ql/constraint.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    //! Cost function for minimizers
    /*! This is the function that must be minimized
    */

    class CostFunction {
      public:
        CostFunction(Size dimensions) : dimensions_(dimensions) {};
        virtual ~CostFunction() {}
        //! returns \f$ f(x) \f$
        virtual double operator()(const Array& x) const = 0;
        virtual void gradient(const Array& x, Array& grad) const {
            Array y(x);

            for (Size i=0; i<dimensions_; i++) {
                double off = 1e-6;
                y[i] -= off;
                double newValue = (*this)(y);
                y[i] += 2.0*off;
                double diffValue = (*this)(y);
                diffValue -= newValue;
                grad[i] = diffValue/(2.0*off);
                y[i] = x[i];
            }
        }
        Size dimensions() const { return dimensions_; }
      private:
        const Size dimensions_;
    };

    //! Abstract base class for minimizers
    class Minimizer {
      public:
        Minimizer() {}
        virtual Array solve(
          const Handle<CostFunction>& f,
          const Handle<Constraint>& constraint) = 0;
      protected:
      private:
    };

}


#endif
