
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
 * You should have received a copy_ of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file shortrateprocess.hpp
    \brief Short rate process

    \fullpath
    ql/InterestRateModelling/%shortrateprocess.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_short_rate_process_h
#define quantlib_interest_rate_modelling_short_rate_process_h

#include <ql/qldefines.hpp>
#include <ql/diffusionprocess.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    //describes a process followed by the short rate or a function of it
    class ShortRateProcess : public DiffusionProcess {
      public:
        virtual double variable(Time t, Rate r) const = 0;
        virtual Rate shortRate(Time t, double variable) const = 0;
    };

}


#endif
