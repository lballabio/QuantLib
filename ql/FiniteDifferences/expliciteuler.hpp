

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file expliciteuler.hpp
    \brief explicit Euler scheme for finite difference methods

    \fullpath
    ql/FiniteDifferences/%expliciteuler.hpp
*/

// $Id$

#ifndef quantlib_forward_euler_h
#define quantlib_forward_euler_h

#include <ql/FiniteDifferences/mixedscheme.hpp>

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
        class ExplicitEuler : public MixedScheme<Operator> {
            friend class FiniteDifferenceModel<ExplicitEuler<Operator> >;
          private:
            // typedefs
            typedef typename Operator::arrayType arrayType;
            typedef Operator operatorType;
            // constructors
            ExplicitEuler(const Operator& L)
            : MixedScheme<Operator>(L, 0.0) {}
        };

    }

}


#endif
