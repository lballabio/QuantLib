
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file cranknicolson.hpp
    \brief Crank-Nicolson scheme for finite difference methods

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/07/09 16:29:27  lballabio
// Some documentation and market element
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/06/18 10:20:25  nando
// 80 colums enforced
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_crank_nicolson_h
#define quantlib_crank_nicolson_h

#include "ql/date.hpp"
#include "ql/FiniteDifferences/identity.hpp"
#include "ql/FiniteDifferences/operatortraits.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        //! Crank-Nicolson scheme for finite difference methods
        /*! This class implements the implicit Crank-Nicolson scheme for 
            the discretization in time of the differential equation
            \f[ 
                \frac{\partial f}{\partial t} = Lf.
            \f]
            In this scheme, the above equation is discretized as
            \f[ 
                \frac{f^{(k)}-f^{(k-1)}}{\Delta t} = 
                L\frac{f^{(k)}+f^{(k-1)}}{2}
            \f]
            hence
            \f[
                \left( I + \frac{\Delta t}{2} L \right) f^{(k-1)} = 
                \left( I - \frac{\Delta t}{2} L \right) f^{(k)}
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

            void setTime(Time t);  // those derived from TimeConstantOperator
                                   // might skip this if the compiler allows it.

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
            CrankNicolson(const operatorType& D) : D(D), dt(0.0) {}
            void step(arrayType& a, Time t);
            void setStep(Time dt) {
                this->dt = dt;
                explicitPart = Identity<arrayType>()-(dt/2)*D;
                implicitPart = Identity<arrayType>()+(dt/2)*D;
            }
            operatorType D, explicitPart, implicitPart;
            Time dt;
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                // a bit of template metaprogramming to relax interface
                // constraints on time-constant operators
                // see T. L. Veldhuizen, "Using C++ Template Metaprograms",
                // C++ Report, Vol 7 No. 4, May 1995
                // http://extreme.indiana.edu/~tveldhui/papers/
                template <int constant>
                class CrankNicolsonTimeSetter {};
                // the following specialization will be instantiated if
                // Operator is derived from TimeConstantOperator
                template<>
                class CrankNicolsonTimeSetter<0> {
                  public:
                    static inline void setTime(Operator& D,
                                               Operator& explicitPart,
                                               Operator& implicitPart,
                                               Time      t,
                                               Time      dt) {}
                };
                // the following specialization will be instantiated if Operator
                // is derived from TimeDependentOperator:
                // only in this case Operator will be required
                // to implement void setTime(Time t)
                template<>
                class CrankNicolsonTimeSetter<1> {
                    typedef typename OperatorTraits<Operator>::arrayType
                        arrayType;
                  public:
                    static inline void setTime(Operator& D,
                                               Operator& explicitPart,
                                               Operator& implicitPart,
                                               Time      t,
                                               Time      dt) {
                        D.setTime(t);
                        explicitPart = Identity<arrayType>()-(dt/2)*D;
                        implicitPart = Identity<arrayType>()+(dt/2)*D;
                    }
                };
            #endif
        };

        // inline definitions

        template <class Operator>
        inline void CrankNicolson<Operator>::step(arrayType& a, Time t) {
            #if QL_TEMPLATE_METAPROGRAMMING_WORKS
                CrankNicolsonTimeSetter<Operator::isTimeDependent>::setTime(D,
                    explicitPart, implicitPart, t, dt);
            #else
                if (Operator::isTimeDependent) {
                    D.setTime(t);
                    explicitPart = Identity<arrayType>()-(dt/2)*D;
                    implicitPart = Identity<arrayType>()+(dt/2)*D;
                }
            #endif
            a = implicitPart.solveFor(explicitPart.applyTo(a));
        }

    }

}


#endif
