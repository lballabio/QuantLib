
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
        CostFunction(unsigned dimensions) : dimensions_(dimensions) {};
        virtual ~CostFunction() {}
        //! returns \f$ f(x) \f$
        virtual double operator()(const Array& x) const = 0;
        virtual void gradient(const Array& x, Array& grad) const {
            Array y(x);

            for (unsigned i=0; i<dimensions_; i++) {
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
        unsigned dimensions() const { return dimensions_; }
      private:
        const unsigned dimensions_;
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
