
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

/*! \file options.hpp
    \brief Base option class

    \fullpath
    ql/%options.hpp
*/

// $Id$

#ifndef quantlib_options_h
#define quantlib_options_h

#include "ql/qldefines.hpp"

namespace QuantLib {

    //! base option class
    /*! This class has the only purpose of encapsulating the option type 
        enumeration.
    */
    class Option {
      public:
        enum Type { Call, Put, Straddle };
    };

}


#endif
