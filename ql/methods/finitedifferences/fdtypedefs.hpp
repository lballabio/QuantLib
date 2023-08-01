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

/*! \file fdtypedefs.hpp
    \brief default choices for template instantiations
*/

#ifndef quantlib_fd_typedefs_hpp
#define quantlib_fd_typedefs_hpp

#include <ql/methods/finitedifferences/cranknicolson.hpp>
#include <ql/methods/finitedifferences/parallelevolver.hpp>

namespace QuantLib {

    /*! \deprecated Define your typedef if needed.
                    Deprecated in version 1.32.
    */
    [[deprecated("Define your typedef if needed")]]
    typedef FiniteDifferenceModel<
                    CrankNicolson<TridiagonalOperator>  >
                                  StandardFiniteDifferenceModel;

    QL_DEPRECATED_DISABLE_WARNING

    /*! \deprecated Define your typedef if needed.
                    Deprecated in version 1.32.
    */
    [[deprecated("Define your typedef if needed")]]
    typedef FiniteDifferenceModel<ParallelEvolver<
                    CrankNicolson<TridiagonalOperator> > >
                                  StandardSystemFiniteDifferenceModel;

    QL_DEPRECATED_ENABLE_WARNING

    /*! \deprecated Define your typedef if needed.
                    Deprecated in version 1.32.
    */
    [[deprecated("Define your typedef if needed")]]
    typedef StepCondition<Array> StandardStepCondition;

}


#endif
