/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file zerocondition.hpp
    \brief zero option exercise condition
*/

#ifndef quantlib_fd_zero_condition_h
#define quantlib_fd_zero_condition_h

#include <ql/FiniteDifferences/stepcondition.hpp>

namespace QuantLib {

    //! Zero exercise condition.
    /*! Used in CEV models */
    template <class array_type>
    class ZeroCondition :  public StepCondition<array_type> {
    public:
        void applyTo(array_type& a, Time) const {
            for(Size i=0; i < a.size(); i++) {
                a[i] = std::max(a[i], 0.0);
            }
        }
    };
}


#endif
