
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


/*! \file finitedifferencemodel.hpp
    \fullpath Include/ql/FiniteDifferences/%finitedifferencemodel.hpp
    \brief generic finite difference model

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.7  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.6  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/07/09 16:29:27  lballabio
// Some documentation and market element
//
// Revision 1.4  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_finite_difference_model_h
#define quantlib_finite_difference_model_h

#include "ql/FiniteDifferences/stepcondition.hpp"
#include "ql/handle.hpp"
#include "ql/null.hpp"

namespace QuantLib {

    //! Finite difference framework
    /*! This namespace contains basic building blocks for the construction 
        of finite difference models, namely,
        
        - a generic model, FiniteDifferenceModel,
        - basic differential operators such as \f$D_+\f$ (DPlus), 
          \f$D_-\f$ (DMinus), \f$D_0\f$ (DZero), and \f$D_+D_-\f$
          (DPlusDMinus),
        - time schemes such as ForwardEuler, BackwardEuler, and 
          CrankNicolson,
        
        as well as more specialized classes.
    */
    namespace FiniteDifferences {

        //! Generic finite difference model
        /*! This class models with a finite difference method the 
            differential equation
            \f[ 
                \frac{\partial f}{\partial t} = Lf 
            \f]
            where \f$L\f$ is a differential operator in ``space'', i.e., one 
            which does not contain partial derivatives in \f$t\f$ but can 
            otherwise contain any derivative in any other variable of the 
            problem. 
            
            \par 
            A differential operator must be discretized in order to be used 
            in a finite difference model. Basic operators such as \f$D_+\f$
            (DZero) or \f$D_+D_-\f$ (DPlusDMinus) are provided in the 
            library which can be composed to form a discretization of a given 
            operator. 

            <b>Example: </b>
            \link custom_operator.cpp
            Black-Scholes operator
            \endlink
            
            \par
            The required interface of the operator depends upon the 
            evolver chosen for the model.

            \par
            While the passed operator encapsulates the spatial discretization 
            of the problem, evolvers encapsulate the discretization of the 
            time derivative. The library provides a few of them which 
            implement well known schemes, namely, 
            \link ForwardEuler 
            forward Euler
            \endlink,
            \link BackwardEuler
            backward Euler
            \endlink,
            and 
            \link CrankNicolson
            Crank-Nicolson
            \endlink
            schemes.
            
            A programmer could implement its own evolver, which does not 
            need to inherit from any base class.
            
            However, it must implement the following interface:

            \code
            class Evolver {
              public:
                typedef ... arrayType;
                typedef ... operatorType;
                // constructors
                Evolver(const operatorType& D);
                // member functions
                void step(arrayType& a, Time t) const;
                void setStep(Time dt);
            };
            \endcode
            
            Finally, it must be noted that the pricing of e.g. an option 
            requires the finite difference model to solve the corresponding 
            equation <i>backwards</i> in time. Therefore, given a 
            discretization \f$f_i\f$ of \f$f(t)\f$, 
            \texttt{evolver.step(\f$f_i\f$,\f$t\f$)} must calculate the 
            discretization of the function at the <i>previous</i> time,
            \f$f(t-dt)\f$.
        */
        template<class Evolver>
        class FiniteDifferenceModel {
          public:
            typedef typename Evolver::arrayType arrayType;
            typedef typename Evolver::operatorType operatorType;
            // constructor
            FiniteDifferenceModel(const operatorType& L) : evolver(L) {}
            // methods
            // arrayType grid() const { return evolver.xGrid(); }
            /*! solves the problem between the given times, possibly 
                applying a condition at every step.
                \warning being this a rollback, <tt>from</tt> must be a later 
                time than <tt>to</tt>.
            */
            void rollback(arrayType& a, Time from, Time to, int steps,
              Handle<StepCondition<arrayType> > condition =
                Handle<StepCondition<arrayType> >());
          private:
            Evolver evolver;
        };

        /*! \example custom_operator.cpp
            The operator \f$L_{BS}\f$ for the Black-Scholes equation in the 
            form 
            \f[ 
                \frac{\partial f}{\partial t} = Lf 
            \f]
            is
            \f[ 
                L_{BS} = - \frac{\sigma^2}{2} \frac{\partial^2}{\partial x^2}
                         - \nu \frac{\partial}{\partial x}
                         + r I.
            \f]
            
            Using the basic operators provided in the library, \f$L_{BS}\f$ 
            can be built as
            \f[
                L_{BS} = - \frac{\sigma^2}{2} D_{+}D_{-}
                         - \nu D_{0} + r I
            \f]
            as shown in the example code.
        */

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
          FiniteDifferenceModel::arrayType& a, Time from, Time to, int steps,
          Handle<StepCondition<arrayType> > condition) {
            Time dt = (from-to)/steps, t = from;
            evolver.setStep(dt);
            for (int i=0; i<steps; i++, t -= dt) {
                evolver.step(a,t);
                if (!condition.isNull())
                    condition->applyTo(a,t);
            }
        }
    }
}


#endif
