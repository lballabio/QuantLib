/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file constrainedcostfunction.hpp
    \brief constrained cost function
*/

#ifndef quantlib_math_constrainedcostfunction_h
#define quantlib_math_constrainedcostfunction_h

//#include <cstdlib>
//#include <iostream>
//#include <vector>
//
//#include <boost/function.hpp>
#include <ql/math/optimization/costfunction.hpp>


namespace QuantLib {

    class ConstrainedCostFunction : public CostFunction {
        public:
            ConstrainedCostFunction(const CostFunction& costFunction,
                const Array& x,
                const std::vector<bool>& argAreFixed);

            virtual Real value(const Array& x) const;
            virtual Disposable<Array> values(const Array& x) const;

            virtual Disposable<Array> project(const Array& x) const;
            virtual Disposable<Array> include(const Array& x) const;

        private:
            void resetNewArg(const Array& x) const; 

            Size numberOfFreeParameters_;
            const Array arg_;
            mutable Array newArg_;
            const std::vector<bool>& argAreFixed_;
            const CostFunction& costFunction_;
    };

}


#endif
