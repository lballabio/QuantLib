
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

/*! \file forwardeuler.hpp
    \brief forward Euler scheme for finite difference methods

    \fullpath
    ql/FiniteDifferences/%forwardeuler.hpp
*/

// $Id$

#ifndef quantlib_forward_euler_h
#define quantlib_forward_euler_h

#include "ql/date.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/operatortraits.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Forward Euler scheme for finite difference methods
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
            arrayType applyTo(const arrayType&);
            \endcode
        */
        template <class Operator>
        class ForwardEuler {
            friend class FiniteDifferenceModel<ForwardEuler<Operator> >;
          private:
            // typedefs
            typedef typename OperatorTraits<Operator>::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            ForwardEuler(operatorType& E) : D_(E), dt_(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                dt_ = dt;
                explicitPart_ = Identity<arrayType>()-dt_*D_;
            }
            Operator& D_;
            Operator explicitPart_;
            Time dt_;
        };

        // inline definitions

        template<class Operator>
        inline void ForwardEuler<Operator>::step(arrayType& a, Time t) {
            if (D_.isTimeDependent()) {
                D_.setTime(t);
                explicitPart_ = Identity<arrayType>()-dt_*D_;
            }
            a = explicitPart_.applyTo(a);
        }

    }

}


#endif
