
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

/*! \file fdtypedefs.hpp 
    \brief default choices for template instantiations

    \fullpath
    ql/FiniteDifferences/%fdtypedefs.hpp
*/

// $Id$

#ifndef quantlib_fd_typedefs_h
#define quantlib_fd_typedefs_h

#include "ql/FiniteDifferences/finitedifferencemodel.hpp"
#include "ql/FiniteDifferences/cranknicolson.hpp"
#include "ql/FiniteDifferences/tridiagonaloperator.hpp"
#include "ql/FiniteDifferences/stepcondition.hpp"

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
