
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

#include "ql/FiniteDifferences/finitedifferencemodel.hpp"

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

            // inspectors
            unsigned int size(); 

            // modifiers
            void setTime(Time t);

            // operator interface
            arrayType applyTo(const arrayType&);
            arrayType solveFor(const arrayType&);
            static Operator identity(unsigned int size);

            // operator algebra
            Operator operator*(double, const Operator&);
            Operator operator+(const Operator&, const Operator&);
            Operator operator+(const Operator&, const Operator&);
            \endcode
            
            \warning The differential operator must be linear for
            this evolver to work.
        */
        template <class Operator>
        class CrankNicolson {
            friend class FiniteDifferenceModel<CrankNicolson<Operator> >;
          private:
            // typedefs
            typedef typename Operator::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            CrankNicolson(const Operator& L) 
            : L_(L), I_(Operator::identity(L.size())), dt_(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                dt_ = dt;
                explicitPart_ = I_-(dt_/2)*L_;
                implicitPart_ = I_+(dt_/2)*L_;
            }
            Operator L_;
            Operator I_;
            Operator explicitPart_, implicitPart_;
            Time dt_;
        };

        // inline definitions

        template <class Operator>
        inline void CrankNicolson<Operator>::step(arrayType& a, Time t) {
            if (L_.isTimeDependent()) {
                L_.setTime(t);
                explicitPart_ = I_-(dt_/2)*L_;
                L_.setTime(t-dt_);
                implicitPart_ = I_+(dt_/2)*L_;
            }
            a = implicitPart_.solveFor(explicitPart_.applyTo(a));
        }

    }

}


#endif
