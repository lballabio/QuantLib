
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdtypedefs.hpp
    \brief default choices for template instantiations
*/

#ifndef quantlib_fd_typedefs_h
#define quantlib_fd_typedefs_h

#include <ql/FiniteDifferences/cranknicolson.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        //! default choice for finite-difference model
        typedef FiniteDifferenceModel<
                    CrankNicolson<TridiagonalOperator>  >
                        StandardFiniteDifferenceModel;

        //! default choice for step condition
        typedef StepCondition<Array> StandardStepCondition;

    }

}


#endif
