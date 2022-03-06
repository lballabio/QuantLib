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

/*! \file mosprime.hpp
\brief %MOSPRIME rate
*/

#ifndef quantlib_mosprime_hpp
#define quantlib_mosprime_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/russia.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/currencies/europe.hpp>

namespace QuantLib {

	//! %MOSPRIME rate
	/*! Moscow Prime Offered Rate fixed by NFEA.

	Conventions are taken from
	http://www.mosprime.com/uploads/files/MosPrime%20Rate%20Rules%20since%2030.01.2014.docx

	\warning Roll convention and EoM not yet checked.
	*/
	class Mosprime : public IborIndex {
	public:
		Mosprime(const Period& tenor,
			const Handle<YieldTermStructure>& h =
			Handle<YieldTermStructure>())
			: IborIndex("MOSPRIME", tenor, (tenor == 1 * Days ? 0 : 1), RUBCurrency(),
				Russia(), ModifiedFollowing, false,
				ActualActual(ActualActual::ISDA), h) {}
	};

}


#endif


#ifndef id_970e38e1b9b60b17b028ac58fa8b48f3
#define id_970e38e1b9b60b17b028ac58fa8b48f3
inline bool test_970e38e1b9b60b17b028ac58fa8b48f3(int* i) { return i != 0; }
#endif
