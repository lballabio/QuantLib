
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file minimizer.hpp
    \brief Abstract 1-D solver class

    \fullpath
    ql/%minimizer.hpp
*/

// $Id$

#ifndef quantlib_minimizer_h
#define quantlib_minimizer_h

#include <vector>
#include "ql/constraint.hpp"
#include "ql/handle.hpp"

namespace QuantLib {

    //! Cost function for minimizers
    /*! This is the function that must be minimized
    */

    using std::vector;

    class CostFunction {
      public:
        CostFunction(unsigned dimensions) : dimensions_(dimensions) {};
        virtual ~CostFunction() {}
        //! returns \f$ f(x) \f$
        virtual double operator()(const vector<double>& x) const = 0;
        virtual void gradient(const vector<double>& x, vector<double>& grad) const {
            vector<double> y(x);

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
        virtual std::vector<double> solve(
          const Handle<CostFunction>& f, 
          const Handle<Constraint>& constraint) = 0;
      protected:
      private:
    };

}


#endif
