
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file expliciteuler.hpp
    \brief explicit Euler scheme for finite difference methods

    \fullpath
    ql/FiniteDifferences/%expliciteuler.hpp
*/

// $Id$

#ifndef quantlib_forward_euler_h
#define quantlib_forward_euler_h

#include <ql/FiniteDifferences/finitedifferencemodel.hpp>

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

            // inspectors
            Size size();

            // modifiers
            void setTime(Time t);

            // operator interface
            arrayType applyTo(const arrayType&);
            static Operator identity(Size size);

            // operator algebra
            Operator operator*(double, const Operator&);
            Operator operator-(const Operator&, const Operator&);
            \endcode

            \todo add Richardson extrapolation
        */
        template <class Operator>
        class ExplicitEuler {
            friend class FiniteDifferenceModel<ExplicitEuler<Operator> >;
          private:
            // typedefs
            typedef typename Operator::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            ExplicitEuler(const Operator& L)
            : L_(L), I_(Operator::identity(L.size())), dt_(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                dt_ = dt;
                explicitPart_ = I_-dt_*L_;
            }
            Operator L_;
            Operator I_;
            Operator explicitPart_;
            Time dt_;
        };

        // inline definitions

        template <class Operator>
        inline void ExplicitEuler<Operator>::step(arrayType& a, Time t) {
            if (L_.isTimeDependent()) {
                L_.setTime(t);
                explicitPart_ = I_-dt_*L_;
            }
            a = explicitPart_.applyTo(a);
        }

    }

}


#endif
