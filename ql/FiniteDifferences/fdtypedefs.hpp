
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
/*! \file fdtypedefs.hpp
    \brief default choices for template instantiations

    \fullpath
    ql/FiniteDifferences/%fdtypedefs.hpp
*/

// $Id$

#ifndef quantlib_fd_typedefs_h
#define quantlib_fd_typedefs_h

#include <ql/FiniteDifferences/cranknicolson.hpp>
#include <ql/FiniteDifferences/tridiagonaloperator.hpp>

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
