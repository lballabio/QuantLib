/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2013 Peter Caspers

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

/*! \file projectedcostfunction.hpp
    \brief Cost function utility
*/

#ifndef quantlib_math_projectedcostfunction_h
#define quantlib_math_projectedcostfunction_h

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/projection.hpp>

namespace QuantLib {

    //! Parameterized cost function
    /*! This class creates a proxy cost function which can depend
        on any arbitrary subset of parameters (the other being fixed)
    */

    class ProjectedCostFunction : public CostFunction, public Projection {
        public:
            ProjectedCostFunction(const CostFunction& costFunction,
                                 const Array& parameterValues,
                                 const std::vector<bool>& fixParameters);

            ProjectedCostFunction(const CostFunction& costFunction,
                                  const Projection& projection);

            //! \name CostFunction interface
            //@{
            Real value(const Array& freeParameters) const override;
            Disposable<Array> values(const Array& freeParameters) const override;
            //@}

        private:
            const CostFunction& costFunction_;
    };

}


#endif


#ifndef id_563aaecea977a6fa33ede373a7cd6498
#define id_563aaecea977a6fa33ede373a7cd6498
inline bool test_563aaecea977a6fa33ede373a7cd6498(const int* i) {
    return i != nullptr;
}
#endif
