
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

/*! \file criteria.hpp
    \brief Optimization criteria class

    \fullpath
    ql/Optimization/%criteria.hpp
*/

#ifndef quantlib_optimization_criteria_h
#define quantlib_optimization_criteria_h

/*!
  class to gather criteria to end optimization process :
  - stationnary point
  - stationnary gradient
  - maximum number of iterations
  ....
*/

namespace QuantLib {

    namespace Optimization {

        class OptimizationEndCriteria {
          public:
            enum EndCriteria { maxIter = 1, statPt = 2, statGd = 3 };

            //! default constructor
            inline OptimizationEndCriteria ();
            //! initialization constructor
            inline OptimizationEndCriteria (int maxIteration, double epsilon);
            //! copy constructor
            inline OptimizationEndCriteria (const OptimizationEndCriteria& oec);
            //! destructor
            inline ~OptimizationEndCriteria () {}

            //! assignement operator
            inline OptimizationEndCriteria& operator=(
                const OptimizationEndCriteria& oec);

            inline void setPositiveOptimization() {
                positiveOptimization_ = true;
            }

            inline bool checkIterationNumber (int iteration) {
                bool test = (iteration >= maxIteration_);
                if (test)
                    endCriteria_ = maxIter;
                return test;
            }
            inline bool checkStationnaryValue(double fold, double fnew) {
                bool test = (QL_FABS (fold - fnew) < functionEpsilon_);
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

            inline bool checkAccuracyValue(double f) {
                bool test = ((f < functionEpsilon_) && positiveOptimization_);
                if (test) {
                    endCriteria_ = statPt;
                }
                return test;
            }

            inline bool checkStationnaryGradientNorm (double normDiff) {
                bool test = (normDiff < gradientEpsilon_);
                if (test)
                    endCriteria_ = statGd;
                return test;
            }
            inline bool checkAccuracyGradientNorm (double norm) {
                bool test = (norm < gradientEpsilon_);
                if (test)
                    endCriteria_ = statGd;
                return test;
            }

            //! test if the number of iteration is not too big and if we don't 
            //  raise a stationnary point
            inline bool operator()(int iteration,
                                   double fold, 
                                   double normgold,
                                   double fnew, 
                                   double normgnew, 
                                   double normdiff);

            //! return the end criteria type
            inline int criteria () const {
                return endCriteria_;
            }

          protected:
            //! Maximum number of iterations
            int maxIteration_;
            //! function and gradient epsilons 
            double functionEpsilon_, gradientEpsilon_;
            //! Maximun number of iterations in stationnary state
            int maxIterStatPt_, statState_;
            int endCriteria_;
            bool positiveOptimization_;
        };

        inline OptimizationEndCriteria::OptimizationEndCriteria ()
        : maxIteration_(100), functionEpsilon_(1e-8), gradientEpsilon_(1e-8),
          maxIterStatPt_(10), statState_(0), endCriteria_(0),
          positiveOptimization_ (false) {}

        inline OptimizationEndCriteria::OptimizationEndCriteria(
            int maxIteration, double epsilon)
        : maxIteration_ (maxIteration), functionEpsilon_ (epsilon), 
          gradientEpsilon_ (epsilon), maxIterStatPt_(maxIteration/10), 
          statState_ (0), endCriteria_ (0), positiveOptimization_ (false) {}

        inline OptimizationEndCriteria::OptimizationEndCriteria(
            const OptimizationEndCriteria & oec)
        : maxIteration_ (oec.maxIteration_), 
          functionEpsilon_(oec.functionEpsilon_), 
          gradientEpsilon_(oec.gradientEpsilon_), 
          maxIterStatPt_(oec.maxIterStatPt_),
          statState_(oec.statState_), endCriteria_(oec.endCriteria_),
          positiveOptimization_(oec.positiveOptimization_) {
        }

        inline OptimizationEndCriteria& OptimizationEndCriteria::operator=(
            const OptimizationEndCriteria & oec) {
            maxIteration_ = oec.maxIteration_;
            functionEpsilon_ = oec.functionEpsilon_;
            gradientEpsilon_ = oec.gradientEpsilon_;
            maxIterStatPt_ = oec.maxIterStatPt_;
            statState_ = oec.statState_;
            endCriteria_ = oec.endCriteria_;
            positiveOptimization_ = oec.positiveOptimization_;
            return (*this);
        }

        inline bool OptimizationEndCriteria::operator()(
            int iteration, double fold, double normgold,
            double fnew, double normgnew, double normdiff) {
            return (
                checkIterationNumber(iteration) ||
                checkStationnaryValue(fold, fnew) || 
                checkAccuracyValue(fnew) || 
                checkAccuracyValue(fold) || 
                checkAccuracyGradientNorm(normgnew) || 
                checkAccuracyGradientNorm(normgold));
        }

    }

}

#endif
