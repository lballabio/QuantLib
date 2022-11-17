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

/*! \file wibor.hpp
\brief %WIBOR rate
*/

#ifndef quantlib_wibor_hpp
#define quantlib_wibor_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/poland.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

	//! %WIBOR rate
	/*! Warsaw Interbank Offered Rate fixed by ACI.

	Conventions are taken from
	http://www.acipolska.pl/images/stories/Rules_for_Fixing_WIBID_and_WIBOR_Reference_Rates_EN.pdf

	\warning Roll convention and EoM not yet checked.
	*/
	class Wibor : public IborIndex {
	public:
		Wibor(const Period& tenor,
              const Handle<YieldTermStructure>& h = {})
			: IborIndex("WIBOR", tenor, (tenor == 1 * Days ? 0 : 2), PLNCurrency(),
				Poland(), ModifiedFollowing, false,
				Actual365Fixed(), h) {}
	};

}


#endif
