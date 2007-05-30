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


namespace QuantLib {

    //! Parameterized cost function
    /*! This class creates a proxy cost function which can depend
        on any arbitrary subset of parameters (the other being fixed)
    */
    class ProjectedCostFunction : public CostFunction {
        public:
            ProjectedCostFunction(const CostFunction& costFunction,
                                 const Array& parametersValues,
                                 const std::vector<bool>& parametersFreedoms);

            //! \name CostFunction interface
            //@{
            virtual Real value(const Array& freeParameters) const;
            virtual Disposable<Array>
                                   values(const Array& freeParameters) const;
            //@}

            //! returns the subset of free parameters corresponding
            // to set of parameters
            virtual Disposable<Array> project(const Array& parameters) const;

            //! returns whole set of parameters corresponding to the set
            // of projected parameters
            virtual Disposable<Array>
                             include(const Array& projectedParameters) const;

        private:
            void mapFreeParameters(const Array& parametersValues) const;
            Size numberOfFreeParameters_;
            const Array fixedParameters_;
            mutable Array actualParameters_;
            const std::vector<bool>& parametersFreedoms_;
            const CostFunction& costFunction_;
    };

}


#endif
