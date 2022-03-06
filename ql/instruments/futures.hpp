/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano
 Copyright (C) 2015 Maddalena Zanzi

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

/*! \file futures.hpp
    \brief Futures
*/

#ifndef quantlib_futures_hpp
#define quantlib_futures_hpp

#include <ql/qldefines.hpp>
#include <iosfwd>

namespace QuantLib {

    struct Futures {
        //! Futures type enumeration
        /*! These conventions specify the kind of futures type. */
        enum Type {
            IMM, /*!< Chicago Mercantile Internation Money Market, i.e.
                      third Wednesday of March, June, September, December */
            ASX  /*!< Australian Security Exchange, i.e. second Friday
                      of March, June, September, December */
        };
    };

    /*! \relates Futures */
    std::ostream& operator<<(std::ostream&, Futures::Type);

}

#endif


#ifndef id_bdd96b6812b56fe8f3c6ea2792ba2d57
#define id_bdd96b6812b56fe8f3c6ea2792ba2d57
inline bool test_bdd96b6812b56fe8f3c6ea2792ba2d57(int* i) { return i != 0; }
#endif
