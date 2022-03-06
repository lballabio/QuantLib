/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file stepcondition.hpp
    \brief conditions to be applied at every time step
*/

#ifndef quantlib_step_condition_hpp
#define quantlib_step_condition_hpp

#include <ql/math/array.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    //! condition to be applied at every time step
    /*! \ingroup findiff */
    template <class array_type>
    class StepCondition {
      public:
        virtual ~StepCondition() = default;
        virtual void applyTo(array_type& a, Time t) const = 0;
    };


    //! %null step condition
    /*! \ingroup findiff */
    template <class array_type>
    class NullCondition : public StepCondition<array_type> {
      public:
        void applyTo(array_type&, Time) const override {}
    };

}


#endif


#ifndef id_46684faf242c60299777d72c426c203e
#define id_46684faf242c60299777d72c426c203e
inline bool test_46684faf242c60299777d72c426c203e(int* i) { return i != 0; }
#endif
