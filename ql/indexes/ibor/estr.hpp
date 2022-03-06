/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Magnus Mencke

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
    \brief %ESTR index
*/

#ifndef quantlib_estr_hpp
#define quantlib_estr_hpp

#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %ESTR (Euro Short-Term Rate) rate fixed by the ECB.
    class Estr : public OvernightIndex {
      public:
        explicit Estr(const Handle<YieldTermStructure>& h =
                      Handle<YieldTermStructure>());
    };

}

#endif


#ifndef id_eeae03661f46e81096d39f2e18071eaf
#define id_eeae03661f46e81096d39f2e18071eaf
inline bool test_eeae03661f46e81096d39f2e18071eaf(const int* i) {
    return i != nullptr;
}
#endif
