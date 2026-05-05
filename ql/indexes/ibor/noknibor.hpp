/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file noknibor.hpp
    \brief NOK-NIBOR index
*/

#ifndef quantext_noknibor_hpp
#define quantext_noknibor_hpp

#include <ql/currencies/europe.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/norway.hpp>
#include <ql/time/daycounters/actual360.hpp>

namespace QuantLib {

    //! NOK-NIBOR index
    /*! NOK-NIBOR rate published by Oslo Boers.
        See <http://www.oslobors.no/ob_eng>.
        https://nore-benchmarks.com/
        https://finansfag.no/wp-content/uploads/2024/10/Rentekonvensjon-6.0_engelsk_oppdatert-18.09.2024_final.pdf
        https://papers.ssrn.com/sol3/papers.cfm?abstract_id=5099269

        \remark Using Norway calendar, should be Oslo.

        \warning Check roll convention and EOM.
    */
    class NOKNibor : public IborIndex {
      public:
        NOKNibor(const Period &tenor, const Handle<YieldTermStructure> &h = Handle<YieldTermStructure>())
        : IborIndex("NOK-NIBOR", tenor, 2, NOKCurrency(), Norway(), ModifiedFollowing, false, Actual360(), h) {}
    };

}

#endif
