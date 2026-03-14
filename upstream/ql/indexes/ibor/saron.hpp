/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Paolo D'Elia

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file saron.hpp
    \brief %SARON index
*/

#ifndef quantlib_saron_hpp
#define quantlib_saron_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %SARON (Swiss Average Rate Overnight) index.
    class Saron : public OvernightIndex {
      public:
        explicit Saron(const Handle<YieldTermStructure>& h = {});
    };

}

#endif
