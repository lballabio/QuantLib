/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fddividendshoutengine.hpp
    \brief base class for shout engine with dividends
*/

#ifndef quantlib_fd_dividend_shout_engine_hpp
#define quantlib_fd_dividend_shout_engine_hpp

#include <ql/PricingEngines/Vanilla/fddividendengine.hpp>
#include <ql/FiniteDifferences/shoutcondition.hpp>

namespace QuantLib {

    //! Finite-differences shout engine with dividends
    /*! \ingroup vanillaengines

        \bug results are not overly reliable.
    */
    typedef FDEngineAdapter<FDShoutCondition<FDDividendEngine>,
                            DividendVanillaOption::engine>
    FDDividendShoutEngine;
}


#endif
