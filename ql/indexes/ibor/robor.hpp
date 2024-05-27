/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Matthias Lungwitz

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

/*! \file robor.hpp
\brief %ROBOR rate
*/

#ifndef quantlib_robor_hpp
#define quantlib_robor_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/romania.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

	//! %ROBOR rate
	/*! Romanian Interbank Offered Rate fixed by BNR.

	Conventions are taken from
	http://www.bnr.ro/files/d/Legislatie/En/RRR.pdf

	\warning Roll convention and EoM not yet checked.
	*/
	class Robor : public IborIndex {
	public:
		Robor(const Period& tenor,
              Handle<YieldTermStructure> h = {})
			: IborIndex("ROBOR", tenor, (tenor == 1 * Days ? 0 : 2), RONCurrency(),
				Romania(), ModifiedFollowing, false,
				Actual360(), std::move(h)) {}
	};

}


#endif
