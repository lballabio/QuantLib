// Emacs will be in -*- Mode: c++ -*-
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@free.fr>
//  http://acm.emath.fr/~dicesare
//
//********************************************************
//
//  Optimization Method and Problem abstract classes
//
//********************************************************
#ifndef quantlib_optimization_optimizer_h
#define quantlib_optimization_optimizer_h

#include <iostream>
#include <iomanip>

#include "ql/array.hpp"
#include "ql/handle.hpp"

#include "ql/Optimization/costfunction.hpp"
#include "ql/Optimization/criteria.hpp"

using std::setw;

namespace QuantLib {

    namespace Optimization {

        class OptimizationMethod;
        class OptimizationProblem;
        /*! 
          Optimization Method abstract class for unconstrained optimization pb
        */
        class OptimizationMethod {
          public:
            typedef double value_type;
          protected:
            //! initial value of unknowns
            Array initialValue_;
            //! current iteration step in the Optimization process
            int iterationNumber_;
            //! optimization end criteria
            OptimizationEndCriteria endCriteria_;
            //! number of evaluation of cost function and its gradient
            int functionEvaluation_, gradientEvaluation_;
            //! function and gradient norm values of the last step
            value_type functionValue_, squaredNorm_;
            //! current values of the local minimum and the search direction
            Array x_, searchDirection_;
          public:
          explicit OptimizationMethod():iterationNumber_(0),
            functionEvaluation_(0), gradientEvaluation_(0),
            functionValue_(1), squaredNorm_(1) {}
            virtual ~OptimizationMethod() {}

            //! Set initial value
            inline void setInitialValue (const Array & initialValue) {
                iterationNumber_ = 0;
                initialValue_ = initialValue;
                x_ = initialValue;
                searchDirection_ = Array (x_.size ());
            }
            //! Set optimization end criteria
            inline void setEndCriteria (const OptimizationEndCriteria & endCriteria) {
                endCriteria_ = endCriteria;
            }

            //! initial value of the optimization method
            //inline const Array& initialValue() { return initialValue_;}
            //! current iteration number
            inline int &iterationNumber () { return iterationNumber_; }
            //! optimization end criteria
            inline OptimizationEndCriteria & endCriteria () { return endCriteria_; }
            //! number of evaluation of cost function
            inline int &functionEvaluation () { return functionEvaluation_; }
            //! number of evaluation of cost function gradient
            inline int &gradientEvaluation () { return gradientEvaluation_; }
            //! value of cost function
            inline value_type & functionValue () { return functionValue_; }
            //! value of cost function gradient norm
            inline value_type & gradientNormValue () { return squaredNorm_; }
            //! current value of the local minimum
            Array& x () { return x_; }
            //! current value of the search direction
            Array& searchDirection () { return searchDirection_; }

            //! minimize the optimization problem P
            virtual void Minimize(OptimizationProblem& P) = 0;
        };



        /*!
          Default Optimization problem output. Define your own by simply overloading
          init(..) and save(...)
        */
        class OptimizationProblemOutput {
            std::ostream & os_;
          public:
            typedef double value_type;

          OptimizationProblemOutput ():os_ (std::cout)
            {
            }
          OptimizationProblemOutput (std::ostream & os):os_ (os)
            {
            }
            virtual ~ OptimizationProblemOutput ()
            {
            }

            virtual void init (OptimizationMethod& method)
            {
#ifdef DISPLAY_OPTI
            os_ << "Initial value : " << method.x () << std::endl;
            os_ << std::endl
                << " fen : function evaluation number" << std::endl
                << " gen : gradient evaluation number" << std::endl
                <<
                "| iter | function value   |     gradient norm     |  line-search step     |  fen  |  gen  |"
                << std::
                
                endl <<
                "|------|------------------|-----------------------|-----------------------|-------|-------|"
                << std::endl;
#endif
            }

            virtual void save (int iterationNumber,
                       value_type function,
                       value_type normGradient,
                       value_type lineSearchStep,
                       OptimizationMethod& method)
            {
            method.functionValue () = function;
            method.gradientNormValue () = normGradient;
#ifdef DISPLAY_OPTI
            os_.setf (std::ios::scientific, std::ios::floatfield);
            os_ << "|" << setw (4) << iterationNumber
                << "  |" << setw (15) << function
                << "   |" << setw (19) << normGradient
                << "    |" << setw (19) << lineSearchStep
                << "    |" << setw (5) << method.functionEvaluation ()
                << "  |" << setw (5) << method.gradientEvaluation ()
                << "  |" << std::endl;
            os_.flush ();
#endif
            }
        };

        /*! 
          Unconstrained optimization pb
        */
        class OptimizationProblem {
          public:
            typedef double value_type;
          protected:
            //! Unconstrained cost function
            CostFunction& costFunction_;
            //! Unconstrained optimization method
            OptimizationMethod& method_;
            Handle<OptimizationProblemOutput> opo_;
          public:
            //! default constructor
            OptimizationProblem (CostFunction& f,	// Function and it gradient vector
                     OptimizationMethod& meth)	// Optimization method
          :	costFunction_ (f), method_ (meth),
            opo_ (Handle<OptimizationProblemOutput>(new OptimizationProblemOutput()))
            {
            opo_->init (method_);
            }
            //! default constructor
            OptimizationProblem (CostFunction &f,	// Function and it gradient vector
                OptimizationMethod &meth,	// Optimization method
                Handle <OptimizationProblemOutput> &opo)
            : costFunction_ (f), method_ (meth), opo_ (opo) {
                opo_->init (method_);
            }

            //! destructor
            ~OptimizationProblem ()
            {
            }

            //! call cost function computation and increment evaluation counter
            value_type value (const Array & x)
            {
            method_.functionEvaluation ()++;
            return costFunction_.value (x);
            }

            //! call cost function gradient computation and increment evaluation counter
            void firstDerivative (Array & grad_f, const Array & x)
            {
            method_.gradientEvaluation ()++;
            costFunction_.firstDerivative (grad_f, x);
            }

            //! call cost function computation and it gradient
            value_type valueAndFirstDerivative (Array & grad_f, const Array & x)
            {
            method_.functionEvaluation ()++;
            method_.gradientEvaluation ()++;
            return costFunction_.valueAndFirstDerivative (grad_f, x);
            }

            //! Unconstrained optimization method
            OptimizationMethod &optimisationMethod ()
            {
            return method_;
            }

            //! Minimization
            void Minimize() { method_.Minimize(*this); }

            Array& minimumValue() { return method_.x (); }

            //! Save results
            void Save (int iterationNumber,
                   value_type function,
                   value_type normGradient,
                   value_type lineSearchStep, OptimizationMethod& method)
            {
            opo_->save (iterationNumber, function, normGradient, lineSearchStep,
                    method);
            }
        };

    }

}
#endif
