/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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

#ifndef quantlib_eonia_hpp
#define quantlib_eonia_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %Eonia (Euro Overnight Index Average) rate fixed by the ECB.
    class Eonia : public OvernightIndex {
      public:
        explicit Eonia(const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
    };

}

#endif


#ifndef id_2b55457c43cb154d7007cf8d427df86b
#define id_2b55457c43cb154d7007cf8d427df86b
inline bool test_2b55457c43cb154d7007cf8d427df86b(const int* i) {
    return i != nullptr;
}
#endif
