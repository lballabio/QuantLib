
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

/*! \file backwardeuler.hpp
    \brief backward Euler scheme for finite difference methods

    \fullpath
    ql/FiniteDifferences/%backwardeuler.hpp
*/

// $Id$

#ifndef quantlib_backward_euler_h
#define quantlib_backward_euler_h

#include "ql/FiniteDifferences/operatortraits.hpp"
#include "ql/FiniteDifferences/finitedifferencemodel.hpp"
#include "ql/FiniteDifferences/identity.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Backward Euler scheme for finite difference methods
        /*! See sect. \ref findiff for details on the method.
            
            In this implementation, the passed operator must be derived 
            from either TimeConstantOperator or TimeDependentOperator.
            Also, it must implement at least the following interface:

            \code
            typedef ... arrayType;
            
            // copy constructor/assignment
            // (these will be provided by the compiler if none is defined)
            Operator(const Operator&);
            Operator& operator=(const Operator&);

            // modifiers
            void setTime(Time t);

            // operator interface
            arrayType solveFor(const arrayType&);

            // operator algebra
            Operator operator*(double,const Operator&);
            Operator operator+(const Identity<arrayType>&,const Operator&);
            \endcode
        */
        template <class Operator>
        class BackwardEuler {
            friend class FiniteDifferenceModel<BackwardEuler<Operator> >;
          private:
            // typedefs
            typedef typename OperatorTraits<Operator>::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            BackwardEuler(operatorType& L) : D_(L), dt_(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                dt_ = dt;
                implicitPart_ = Identity<arrayType>()+dt_*D_;
            }
            operatorType& D_;
            operatorType implicitPart_;
            Time dt_;
        };

        // inline definitions

        template <class Operator>
        inline void BackwardEuler<Operator>::step(arrayType& a, Time t) {
            if (Operator::isTimeDependent) {
                L.setTime(t);
                implicitPart_ = Identity<arrayType>()+dt_*D_;
            }
            a = implicitPart_.solveFor(a);
        }

    }

}


#endif
