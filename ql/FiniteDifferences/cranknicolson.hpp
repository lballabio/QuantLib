
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

/*! \file cranknicolson.hpp
    \brief Crank-Nicolson scheme for finite difference methods

    \fullpath
    ql/FiniteDifferences/%cranknicolson.hpp
*/

// $Id$

#ifndef quantlib_crank_nicolson_h
#define quantlib_crank_nicolson_h

#include "ql/date.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/operatortraits.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Crank-Nicolson scheme for finite difference methods
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

            void setTime(Time t);

            // operator interface
            arrayType applyTo(const arrayType&);
            arrayType solveFor(const arrayType&);

            // operator algebra
            Operator operator*(double,const Operator&);
            Operator operator+(const Identity<arrayType>&,const Operator&);
            \endcode
            
            \warning The differential operator must be linear for
            this evolver to work.
        */
        template <class Operator>
        class CrankNicolson {
            friend class FiniteDifferenceModel<CrankNicolson<Operator> >;
          private:
            // typedefs
            typedef typename OperatorTraits<Operator>::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            CrankNicolson(operatorType& D) : D_(D), dt_(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                dt_ = dt;
                explicitPart_ = Identity<arrayType>()-(dt_/2)*D_;
                implicitPart_ = Identity<arrayType>()+(dt_/2)*D_;
            }
            operatorType& D_;
            operatorType explicitPart_, implicitPart_;
            Time dt_;
        };

        // inline definitions

        template <class Operator>
        inline void CrankNicolson<Operator>::step(arrayType& a, Time t) {
            if (D_.isTimeDependent()) {
                D_.setTime(t);
                explicitPart_ = Identity<arrayType>()-(dt_/2)*D_;
                implicitPart_ = Identity<arrayType>()+(dt_/2)*D_;
            }
            a = implicitPart_.solveFor(explicitPart_.applyTo(a));
        }

    }

}


#endif
