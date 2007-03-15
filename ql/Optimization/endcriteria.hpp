/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
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

/*! \file criteria.hpp
    \brief Optimization criteria class
*/

#ifndef quantlib_optimization_criteria_hpp
#define quantlib_optimization_criteria_hpp

#include <ql/types.hpp>
#include <ql/Utilities/null.hpp>
#include <iostream>

namespace QuantLib {


    //! Criteria to end optimization process
    /*!   - stationary point
          - stationary gradient
          - maximum number of iterations
          ....
    */
    class EndCriteria {
      public:
        enum Type { None,
                    MaxIterations,
                    StationaryPoint,
                    StationaryGradient,
                    Unknown };

        //! initialization constructor
        EndCriteria(Size maxIteration,
                    Real functionEpsilon,
                    Real gradientEpsilon,
                    Size maxStationaryStateIterations);

        // Inspectors
        Size maxIterations() const;
        Real functionEpsilon() const;
        Real gradientEpsilon() const;
        Size maxStationaryStateIterations() const;

        /*! test if the number of iteration is not too big and if we don't
            raise a stationary point */
        bool operator()(const Size iteration,
                        Size& statState,
                        const bool positiveOptimization,
                        const Real fold,
                        const Real normgold,
                        const Real fnew,
                        const Real normgnew,
                        EndCriteria::Type& ecType) const;

        bool checkIterationNumber(const Size iteration,
                                  EndCriteria::Type& ecType) const;
        bool checkStationaryValue(const Real fold,
                                  const Real fnew,
                                  Size& statStateIterations,
                                  EndCriteria::Type& ecType) const;
        bool checkAccuracyValue(const Real f,
                                const bool positiveOptimization,
                                EndCriteria::Type& ecType) const;
        bool checkStationaryGradientNorm(const Real normDiff,
                                         EndCriteria::Type& ecType) const;
        bool checkAccuracyGradientNorm(const Real norm,
                                       EndCriteria::Type& ecType) const;

      protected:
        //! Maximum number of iterations
        Size maxIterations_;
        //! function and gradient epsilons
        Real functionEpsilon_, gradientEpsilon_;
        //! Maximun number of iterations in stationary state
        mutable Size maxStationaryStateIterations_;

    };

	std::ostream& operator<<(std::ostream& out,
                             EndCriteria::Type ecType);

    inline bool EndCriteria::checkIterationNumber(const Size iteration,
                                                  EndCriteria::Type& ecType) const{
        if (iteration < maxIterations_)
            return false;
        ecType = MaxIterations;
        return true;
    }

    inline bool EndCriteria::checkStationaryValue(const Real fold,
                                                  const Real fnew,
                                                  Size& statStateIterations,
                                                  EndCriteria::Type& ecType) const {
        if (std::fabs(fold - fnew) >= functionEpsilon_) {
            statStateIterations = 0;
            return false;
        }
        ++statStateIterations;
        if (statStateIterations <= maxStationaryStateIterations_)
            return false;
        ecType = StationaryPoint;
        return true;
    }

    inline bool EndCriteria::checkAccuracyValue(const Real f,
                                                const bool positiveOptimization,
                                                EndCriteria::Type& ecType) const {
        if (!positiveOptimization)
            return false;
        if (f >= functionEpsilon_)
            return false;
        ecType = StationaryPoint;
        return true;
    }

    inline bool EndCriteria::checkStationaryGradientNorm(const Real normDiff,
                                                         EndCriteria::Type& ecType) const {
        if (normDiff >= gradientEpsilon_)
            return false;
        ecType = StationaryGradient;
        return true;
    }

    inline bool EndCriteria::checkAccuracyGradientNorm(const Real norm,
                                                       EndCriteria::Type& ecType) const {
        if (norm >= gradientEpsilon_)
            return false;
        ecType = StationaryGradient;
        return true;
    }

    inline bool EndCriteria::operator()(const Size iteration,
                                        Size& statStateIterations,
                                        const bool positiveOptimization,
                                        const Real fold,
                                        const Real normgold,
                                        const Real fnew,
                                        const Real normgnew,
                                        EndCriteria::Type& ecType) const {
        return
            checkIterationNumber(iteration, ecType) ||
            checkStationaryValue(fold, fnew, statStateIterations, ecType) ||
            checkAccuracyValue(fnew, positiveOptimization, ecType) ||
            checkAccuracyValue(fold, positiveOptimization, ecType) ||
            checkAccuracyGradientNorm(normgnew, ecType) ||
            checkAccuracyGradientNorm(normgold, ecType);
    }

    // Inspectors
    inline Size EndCriteria::maxIterations() const {
        return maxIterations_;
    }

    inline Size EndCriteria::maxStationaryStateIterations() const {
        return maxStationaryStateIterations_;
    }

    inline Real EndCriteria::functionEpsilon() const {
        return functionEpsilon_;
    }

    inline Real EndCriteria::gradientEpsilon() const {
        return gradientEpsilon_;
    }
}

#endif
