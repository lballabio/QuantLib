
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

/*! \file criteria.hpp
    \brief Optimization criteria class
*/

#ifndef quantlib_optimization_criteria_h
#define quantlib_optimization_criteria_h

namespace QuantLib {

    //! Criteria to end optimization process
    /*!   - stationary point
          - stationary gradient
          - maximum number of iterations
          ....
    */
    class EndCriteria {
      public:
        enum Type { maxIter, statPt, statGd };

        //! default constructor
        EndCriteria()
        : maxIteration_(100), functionEpsilon_(1e-8), 
          gradientEpsilon_(1e-8), maxIterStatPt_(10), 
          statState_(0), endCriteria_(0),
          positiveOptimization_ (false) {}

        //! initialization constructor
        EndCriteria(int maxIteration, double epsilon)
        : maxIteration_ (maxIteration), functionEpsilon_ (epsilon),
          gradientEpsilon_ (epsilon), maxIterStatPt_(maxIteration/10),
          statState_ (0), endCriteria_ (0), 
          positiveOptimization_ (false) {}

        void setPositiveOptimization() {
            positiveOptimization_ = true;
        }

        bool checkIterationNumber (int iteration) {
            bool test = (iteration >= maxIteration_);
            if (test)
                endCriteria_ = maxIter;
            return test;
        }

        bool checkStationaryValue(double fold, double fnew) {
            bool test = (QL_FABS(fold - fnew) < functionEpsilon_);
            if (test) {
                statState_++;
                if (statState_ > maxIterStatPt_) {
                    endCriteria_ = statPt;
                }
            } else {
                if (statState_ != 0)
                    statState_ = 0;
            }
            return (test && (statState_ > maxIterStatPt_));
        }

        bool checkAccuracyValue(double f) {
            bool test = (f < functionEpsilon_ && positiveOptimization_);
            if (test) {
                endCriteria_ = statPt;
            }
            return test;
        }

        bool checkStationaryGradientNorm (double normDiff) {
            bool test = (normDiff < gradientEpsilon_);
            if (test)
                endCriteria_ = statGd;
            return test;
        }

        bool checkAccuracyGradientNorm (double norm) {
            bool test = (norm < gradientEpsilon_);
            if (test)
                endCriteria_ = statGd;
            return test;
        }

        //! test if the number of iteration is not too big and if we don't
        //  raise a stationary point
        bool operator()(int iteration,
                        double fold,
                        double normgold,
                        double fnew,
                        double normgnew,
                        double normdiff) {
            return 
                checkIterationNumber(iteration) ||
                checkStationaryValue(fold, fnew) ||
                checkAccuracyValue(fnew) ||
                checkAccuracyValue(fold) ||
                checkAccuracyGradientNorm(normgnew) ||
                checkAccuracyGradientNorm(normgold);
        }

        //! return the end criteria type
        int criteria() const {
            return endCriteria_;
        }

      protected:
        //! Maximum number of iterations
        int maxIteration_;
        //! function and gradient epsilons
        double functionEpsilon_, gradientEpsilon_;
        //! Maximun number of iterations in stationary state
        int maxIterStatPt_, statState_;
        int endCriteria_;
        bool positiveOptimization_;
    };

}


#endif
