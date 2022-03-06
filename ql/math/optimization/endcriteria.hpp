/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

/*! \file endcriteria.hpp
    \brief Optimization criteria class
*/

#ifndef quantlib_optimization_criteria_hpp
#define quantlib_optimization_criteria_hpp

#include <ql/utilities/null.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Criteria to end optimization process:
    /*! - maximum number of iterations AND minimum number of iterations around stationary point
        - x (independent variable) stationary point
        - y=f(x) (dependent variable) stationary point
        - stationary gradient
    */
    class EndCriteria {
      public:
        enum Type {None,
                   MaxIterations,
                   StationaryPoint,
                   StationaryFunctionValue,
                   StationaryFunctionAccuracy,
                   ZeroGradientNorm,
                   Unknown};

        //! Initialization constructor
        EndCriteria(Size maxIterations,
                    Size maxStationaryStateIterations,
                    Real rootEpsilon,
                    Real functionEpsilon,
                    Real gradientNormEpsilon);

        // Inspectors
        Size maxIterations() const;
        Size maxStationaryStateIterations() const;
        Real rootEpsilon() const;
        Real functionEpsilon() const;
        Real gradientNormEpsilon() const;

        /*! Test if the number of iterations is not too big 
            and if a minimum point is not reached */
        bool operator()(Size iteration,
                        Size& statState,
                        bool positiveOptimization,
                        Real fold,
                        Real normgold,
                        Real fnew,
                        Real normgnew,
                        EndCriteria::Type& ecType) const;

        /*! Test if the number of iteration is below MaxIterations */
        bool checkMaxIterations(Size iteration, EndCriteria::Type& ecType) const;
        /*! Test if the root variation is below rootEpsilon */
        bool checkStationaryPoint(Real xOld,
                                  Real xNew,
                                  Size& statStateIterations,
                                  EndCriteria::Type& ecType) const;
        /*! Test if the function variation is below functionEpsilon */
        bool checkStationaryFunctionValue(Real fxOld,
                                          Real fxNew,
                                          Size& statStateIterations,
                                          EndCriteria::Type& ecType) const;
        /*! Test if the function value is below functionEpsilon */
        bool checkStationaryFunctionAccuracy(Real f,
                                             bool positiveOptimization,
                                             EndCriteria::Type& ecType) const;
        /*! Test if the gradient norm variation is below gradientNormEpsilon */
        //bool checkZerGradientNormValue(const Real gNormOld, 
        //                               const Real gNormNew,
        //                               EndCriteria::Type& ecType) const;
        /*! Test if the gradient norm value is below gradientNormEpsilon */
        bool checkZeroGradientNorm(Real gNorm, EndCriteria::Type& ecType) const;

      protected:
        //! Maximum number of iterations
        Size maxIterations_;
        //! Maximun number of iterations in stationary state
        mutable Size maxStationaryStateIterations_;
        //! root, function and gradient epsilons
        Real rootEpsilon_, functionEpsilon_, gradientNormEpsilon_;

    };

    std::ostream& operator<<(std::ostream& out, EndCriteria::Type ecType);

}

#endif


#ifndef id_d412a10634e3fb60ee66ddae5d07c8e6
#define id_d412a10634e3fb60ee66ddae5d07c8e6
inline bool test_d412a10634e3fb60ee66ddae5d07c8e6(const int* i) {
    return i != nullptr;
}
#endif
