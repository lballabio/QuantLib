/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore

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

/*! \file duration.hpp
    \brief Duration type enumeration
*/

#ifndef quantlib_duration_hpp
#define quantlib_duration_hpp

#include <ql/qldefines.hpp>
#include <iosfwd>

namespace QuantLib {

    //! %duration type
    struct Duration {
        enum Type { Simple, Macaulay, Modified };
    };

    /*! \relates BusinessDayConvention */
    std::ostream& operator<<(std::ostream&,
                             Duration::Type);

}

#endif


#ifndef id_db052e630e09ec87cca20ab6fc4c4cbb
#define id_db052e630e09ec87cca20ab6fc4c4cbb
inline bool test_db052e630e09ec87cca20ab6fc4c4cbb(int* i) { return i != 0; }
#endif
