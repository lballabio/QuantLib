/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file eonia.hpp
    \brief %Eonia index
*/

#ifndef quantlib_sonia_hpp
#define quantlib_sonia_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %Sonia (Sterling Overnight Index Average) rate.
    class Sonia : public OvernightIndex {
      public:
        explicit Sonia(const Handle<YieldTermStructure>& h = {});
    };

}

#endif
