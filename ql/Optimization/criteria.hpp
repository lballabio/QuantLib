// Emacs will be in -*- Mode: c++ -*-
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@free.fr>
//  http://acm.emath.fr/~dicesare
//
//********************************************************
//
//  Optimization end criteria
//
//********************************************************
#ifndef quantlib_optimization_criteria_h
#define quantlib_optimization_criteria_h

#include <iostream>
#include <iomanip>

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
            enum EndCriteria
            { maxIter = 1, statPt = 2, statGd = 3 };
          protected:
            //! Maximum number of iterations
            int maxIteration_;
            //! function and gradient epsilons 
            double functionEpsilon_, gradientEpsilon_;
            //! Maximun number of iterations in stationnary state
            int maxIterStatPt_, statState_;
            int endCriteria_;
            bool positiveOptimization_;
          public:
            //! default constructor
              inline OptimizationEndCriteria ();
            //! initialization constructor
            inline OptimizationEndCriteria (int maxIteration, double epsilon);
            //! copy constructor
            inline OptimizationEndCriteria (const OptimizationEndCriteria & oec);
            //! destructor
              inline ~ OptimizationEndCriteria ()
            {
            }

            //! assignement operator
            inline OptimizationEndCriteria & operator =
            (const OptimizationEndCriteria & oec);

            inline void setPositiveOptimization ()
            {
            positiveOptimization_ = true;
#ifdef DISPLAY_OPTI
            std::
                cout << "--> Assuming positivity of the optimization problem" << std::endl;
#endif
            }

            inline bool checkIterationNumber (int iteration)
            {
            //cout<<"checkIterationNumber"<<endl;
            bool test = (iteration >= maxIteration_);
            if (test)
              {
                  endCriteria_ = maxIter;
#ifdef DISPLAY_OPTI
                  std::cout << "maxIteration" << std::endl;
#endif
              }
            return test;
            }
            inline bool checkStationnaryValue (double fold, double fnew)
            {
            //cout<<"checkStationnaryValue"<<endl;
            bool test = (QL_FABS (fold - fnew) < functionEpsilon_);
            if (test)
              {
                  statState_++;
                  if (statState_ > maxIterStatPt_)
                {
                    endCriteria_ = statPt;
#ifdef DISPLAY_OPTI
                    std::
                    
                    cout <<
                    "stationnary point, number of stationnary evaluation : "
                    << statState_ << std::endl;
#endif
                }
              }
            else
              {
                  if (statState_ != 0)
                  statState_ = 0;
              }
            return (test && (statState_ > maxIterStatPt_));
            }
            inline bool checkAccuracyValue (double f)
            {
            //cout <<"checkAccuracyValue"<<endl;
            bool test = ((f < functionEpsilon_) && positiveOptimization_);
            if (test)
              {
                  endCriteria_ = statPt;
#ifdef DISPLAY_OPTI
                  std::
                  
                  cout << "Function accuracy satisfied : f = " << f <<
                  ", functionEpsilon_ = " << functionEpsilon_ << std::endl;
#endif
              }
            return test;
            }
            inline bool checkStationnaryGradientNorm (double normDiff)
            {
            //cout << "checkStationnaryGradientNorm" << endl;
            bool test = (normDiff < gradientEpsilon_);
            if (test)
              {
                  endCriteria_ = statGd;
#ifdef DISPLAY_OPTI
                  std::cout << "stationnary gradient" << std::endl;
#endif
              }
            return test;
            }
            inline bool checkAccuracyGradientNorm (double norm)
            {
            //cout <<"checkAccuracyGradientNorm"<<endl;
            bool test = (norm < gradientEpsilon_);
            if (test)
              {
                  endCriteria_ = statGd;
#ifdef DISPLAY_OPTI
                  std::cout << "new local minima" << std::endl;
#endif
              }
            return test;
            }

            //! test if the number of iteration is not too big and if we not raise a stationnary point
            inline
            bool operator ()(int iteration,
                     double fold, double normgold,
                     double fnew, double normgnew, double normdiff);

            //! return the end criteria type : enum EndCriteria { maxIter=1, statPt=2, statGd=3};
            inline int criteria () const
            {
            return endCriteria_;
            }
        };

        inline OptimizationEndCriteria::OptimizationEndCriteria (): maxIteration_ (100),
          functionEpsilon_ (1e-8), gradientEpsilon_ (1e-8),
          maxIterStatPt_ (10), statState_ (0), endCriteria_ (0),
          positiveOptimization_ (false)
        {
            //std::cout <<"OptimizationEndCriteria) maxIteration_ : "<<maxIteration_<<std::endl;
        }

        inline
            OptimizationEndCriteria::OptimizationEndCriteria (int maxIteration,
        double epsilon)
        : maxIteration_ (maxIteration), functionEpsilon_ (epsilon), 
          gradientEpsilon_ (epsilon), maxIterStatPt_(maxIteration/10), 
          statState_ (0), endCriteria_ (0), positiveOptimization_ (false)
        {
            //std::cout <<"OptimizationEndCriteria(,) maxIteration_ : "<<maxIteration_<<std::endl;
        }

        inline
            OptimizationEndCriteria::
        OptimizationEndCriteria (const OptimizationEndCriteria & oec):
        maxIteration_ (oec.maxIteration_), functionEpsilon_ (oec.functionEpsilon_),
        gradientEpsilon_ (oec.gradientEpsilon_), maxIterStatPt_ (oec.maxIterStatPt_),
        statState_ (oec.statState_), endCriteria_ (oec.endCriteria_),
        positiveOptimization_ (oec.positiveOptimization_)
        {
            //std::cout <<"OptimizationEndCriteria(const) maxIteration_ : "<<maxIteration_<<std::endl;
        }

        inline OptimizationEndCriteria &
            OptimizationEndCriteria::operator = (const OptimizationEndCriteria & oec) {
            maxIteration_ = oec.maxIteration_;
            functionEpsilon_ = oec.functionEpsilon_;
            gradientEpsilon_ = oec.gradientEpsilon_;
            maxIterStatPt_ = oec.maxIterStatPt_;
            statState_ = oec.statState_;
            endCriteria_ = oec.endCriteria_;
            positiveOptimization_ = oec.positiveOptimization_;

            //std::cout <<"OptimizationEndCriteria =(const) maxIteration_ : "<<maxIteration_<<std::endl; 

            return (*this);
        }


        inline
            bool OptimizationEndCriteria::operator ()(int iteration,
                                  double fold, double normgold,
                                  double fnew, double normgnew,
                                  double normdiff)
        {
            /*
               return ( checkIterationNumber(iteration) ||
               checkStationnaryValue(fold,fnew) || checkAccuracyValue(fnew) || checkAccuracyValue(fold) ||
               checkStationnaryGradientNorm(normdiff) || checkAccuracyGradientNorm(normgnew) || checkAccuracyGradientNorm(normgold) );
             */
            return (checkIterationNumber (iteration) ||
                checkStationnaryValue (fold, fnew) || checkAccuracyValue (fnew)
                || checkAccuracyValue (fold)
                || checkAccuracyGradientNorm (normgnew)
                || checkAccuracyGradientNorm (normgold));
        }

    }

}



#endif
