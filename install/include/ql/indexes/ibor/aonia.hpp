/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2016 Fabrice Lecuyer

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

/*! \file aonia.hpp
\brief %Aonia index
*/

#ifndef quantlib_aonia_hpp
#define quantlib_aonia_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/australia.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/oceania.hpp>

namespace QuantLib {

    //! %Aonia index
    /*! Aonia (Australia Overnight Index Average) rate fixed by the RBA.

    See <http://www.isda.org/publications/pdf/Supplement-13-to-2000DefinitionsAnnex.pdf>.
    */
    class Aonia : public OvernightIndex {
      public:
        explicit Aonia(const Handle<YieldTermStructure>& h = {})
        : OvernightIndex("Aonia", 0, AUDCurrency(),
                         Australia(),
                         Actual365Fixed(), h) {}
    };

}

#endif
