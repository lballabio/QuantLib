
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

    \fullpath
    Include/ql/FiniteDifferences/%backwardeuler.hpp
    \brief backward Euler scheme for finite differemce methods
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
        /*! This class implements the implicit backward Euler scheme for the
            discretization in time of the differential equation
            \f[
                \frac{\partial f}{\partial t} = Lf.
            \f]
            In this scheme, the above equation is discretized as
            \f[
                \frac{f^{(k)}-f^{(k-1)}}{\Delta t} = Lf^{(k-1)}
            \f]
            hence
            \f[
                \left( I + \Delta t L \right) f^{(k-1)} = f^{(k)}
            \f]
            from which \f$f^{(k-1)}\f$ can be obtained.

            \par
            In this implementation, the operator \f$L\f$ must be derived
            from either TimeConstantOperator or TimeDependentOperator.
            Also, it must implement at least the following interface:

            \code
            // copy constructor/assignment
            // (these will be provided by the compiler if none is defined)
            Operator(const Operator&);
            Operator& operator=(const Operator&);

            // modifiers
            void setTime(Time t);  // those derived from TimeConstantOperator
                                   // might skip this if the compiler allows it.

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
            BackwardEuler(const operatorType& L) : L(L), dt(0.0) {}
            void step(arrayType& a, Time t) const;
            void setStep(Time dt) {
                this->dt = dt;
                implicitPart = Identity<arrayType>()+dt*L;
            }
            operatorType L, implicitPart;
            Time dt;
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                // a bit of template metaprogramming to relax interface
                // constraints on time-constant operators
                // see T. L. Veldhuizen, "Using C++ Template Metaprograms",
                // C++ Report, Vol 7 No. 4, May 1995
                // http://extreme.indiana.edu/~tveldhui/papers/
                template <int constant>
                class BackwardEulerTimeSetter {};
                // the following specialization will be instantiated if
                // Operator is derived from TimeConstantOperator
                template<>
                class BackwardEulerTimeSetter<0> {
                  public:
                    static inline void setTime(Operator& L,
                    Operator& implicitPart, Time t, Time dt) {}
                };
                // the following specialization will be instantiated if
                // Operator is derived from TimeDependentOperator:
                // only in this case Operator will be required to
                // implement void setTime(Time t)
                template<>
                class BackwardEulerTimeSetter<1> {
                    typedef typename OperatorTraits<Operator>::arrayType
                        arrayType;
                  public:
                    static inline void setTime(Operator& L,
                      Operator& implicitPart, Time t, Time dt) {
                        L.setTime(t);
                        implicitPart = Identity<arrayType>()+dt*L;
                    }
                };
            #endif
        };

        // inline definitions

        template <class Operator>
        inline void BackwardEuler<Operator>::step(arrayType& a, Time t) const {
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                BackwardEulerTimeSetter<Operator::isTimeDependent>::setTime(
                    L,implicitPart,t,dt);
            #else
                if (Operator::isTimeDependent) {
                    L.setTime(t);
                    implicitPart = Identity<arrayType>()+dt*L;
                }
            #endif
            a = implicitPart.solveFor(a);
        }

    }

}


#endif
