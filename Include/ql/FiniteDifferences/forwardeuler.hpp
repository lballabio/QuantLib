
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

    $Id$
*/

// $Source$
// $Log$
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

#ifndef quantlib_forward_euler_h
#define quantlib_forward_euler_h

#include "ql/date.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/operatortraits.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Forward Euler scheme for finite difference methods
        /*! This class implements the explicit forward Euler scheme for the 
            discretization in time of the differential equation
            \f[ 
                \frac{\partial f}{\partial t} = Lf.
            \f]
            In this scheme, the above equation is discretized as
            \f[ 
                \frac{f^{(k)}-f^{(k-1)}}{\Delta t} = Lf^{(k)}
            \f]
            hence
            \f[
                f^{(k-1)} = \left( I - \Delta t L \right) f^{(k)}
            \f]
            from which \f$f^{(k-1)}\f$ can be obtained directly.
            
            \par
            In this implementation, the operator \f$L\f$ must be derived 
            from either TimeConstantOperator or TimeDependentOperator.
            Also, it must implement at least the following interface:

            \code
            // copy constructor/assignment
            // (these will be provided by the compiler if none is defined)
            Operator(const Operator&);

            // modifiers
            void setTime(Time t);  // those derived from TimeConstantOperator
                                   // might skip this if the compiler allows it.

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
            ForwardEuler(const operatorType& D) : D(D), dt(0.0) {}
            void step(arrayType& a, Time t) const;
            void setStep(Time dt) {
                this->dt = dt;
                explicitPart = Identity<arrayType>()-dt*D;
            }
            Operator D, explicitPart;
            Time dt;
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                // a bit of template metaprogramming to relax interface 
                // constraints on time-constant operators.
                // see T. L. Veldhuizen, "Using C++ Template Metaprograms", 
                // C++ Report, Vol 7 No. 4, May 1995
                // http://extreme.indiana.edu/~tveldhui/papers/
                template <int constant>
                class ForwardEulerTimeSetter {};
                // the following specialization will be instantiated if 
                // Operator is derived from TimeConstantOperator
                template<>
                class ForwardEulerTimeSetter<0> {
                  public:
                    static inline void setTime(Operator& D, 
                        Operator& explicitPart, Time t, Time dt) {}
                };
                // the following specialization will be instantiated if 
                // Operator is derived from TimeDependentOperator:
                // only in this case Operator will be required to implement 
                // void setTime(Time t)
                template<>
                class ForwardEulerTimeSetter<1> {
                  public:
                    static inline void setTime(Operator& D, 
                      Operator& explicitPart, Time t, Time dt) {
                        D.setTime(t);
                        explicitPart = Identity<arrayType>()-dt*D;
                    }
                };
            #endif
        };

        // inline definitions

        template<class Operator>
        inline void ForwardEuler<Operator>::step(arrayType& a, Time t) const {
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                ForwardEulerTimeSetter<Operator::isTimeDependent>::setTime(
                    D,explicitPart,t,dt);
            #else
                if (Operator::isTimeDependent) {
                    D.setTime(t);
                    explicitPart = Identity<arrayType>()-dt*D;
                }
            #endif
            a = explicitPart.applyTo(a);
        }

    }

}


#endif
