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
    \brief Constrained cost function
*/

#ifndef quantlib_math_projectedcostfunction_h
#define quantlib_math_projectedcostfunction_h

#include <ql/math/optimization/costfunction.hpp>


namespace QuantLib {
    /*! This class allows to restrict the number of parameters used 
        by a cost function. This should be used if one wants to calibrate
        with respect to a subset of parameters (the other being fixed)
    */
    class ProjectedCostFunction : public CostFunction {
        public:
            ProjectedCostFunction(const CostFunction& costFunction,
                const Array& x,
                const std::vector<bool>& parametersFreedoms);

            //! \name CostFunction interface
            //@{
            virtual Real value(const Array& freeParameters) const;
            virtual Disposable<Array> values(const Array& freeParameters) const;
            //@}

            //! returns the subset of free parameters corresponding 
            // to set of parameters x
            virtual Disposable<Array> project(const Array& parameters) const;

            //! returns set of parameters x corresponding to the subset of 
            // free parameters
            virtual Disposable<Array> include(
                                        const Array& projectedParameters) const;

        private:
            void mapFreeParamters(const Array& x) const; 
            Size numberOfFreeParameters_;
            const Array fixedParameters_;
            mutable Array actualParameters_;
            const std::vector<bool>& parametersFreedoms_;
            const CostFunction& costFunction_;
    };

}


#endif
