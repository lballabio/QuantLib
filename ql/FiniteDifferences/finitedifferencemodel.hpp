
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

    \fullpath
    Include/ql/FiniteDifferences/%finitedifferencemodel.hpp
    \brief generic finite difference model

*/

// $Id$
// $Log$
// Revision 1.2  2001/09/04 15:15:28  lballabio
// Finite difference docs updated
//
// Revision 1.1  2001/09/03 14:00:08  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.11  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.10  2001/08/28 13:37:35  nando
// unsigned int instead of int
//
// Revision 1.9  2001/08/09 14:59:46  sigmud
// header modification
//
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
    /*! See sect. \ref findiff */
    namespace FiniteDifferences {

        //! Generic finite difference model
        /*! See sect. \ref findiff */
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
            void rollback(arrayType& a, Time from, Time to, 
              unsigned int steps, 
              Handle<StepCondition<arrayType> > condition =
                Handle<StepCondition<arrayType> >());
          private:
            Evolver evolver;
        };

        // template definitions
        template<class Evolver>
        void FiniteDifferenceModel<Evolver>::rollback(
            FiniteDifferenceModel::arrayType& a,
            Time from,
            Time to,
            unsigned int steps,
            Handle<StepCondition<arrayType> > condition) {
                Time dt = (from-to)/steps, t = from;
                evolver.setStep(dt);
                for (unsigned int i=0; i<steps; i++, t -= dt) {
                    evolver.step(a,t);
                    if (!condition.isNull())
                        condition->applyTo(a,t);
            }
        }
    }
}


#endif
