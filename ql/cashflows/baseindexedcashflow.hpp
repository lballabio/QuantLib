/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2011 Chris Kenyon

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

/*! \file baseindexedcashflow.hpp
 \brief Cash flow dependent on an index ratio (NOT a coupon, i.e. no accruals)
 */

#ifndef quantlib_baseindexed_cash_flow_hpp
#define quantlib_baseindexed_cash_flow_hpp

#include <ql/cashflow.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/index.hpp>

namespace QuantLib {

    class QuantLib::Index;

	//! when you observe an index, how do you interpolate between fixings?
	//! \enum as the index, keep flat, or linear
	enum indexInterpolationType {
		iiINDEX = 0, iiFLAT = 1, iiLINEAR = 2
	};
	
    //! Cash flow dependent on an index ratio (NOT a coupon, i.e. no accruals)
    /*! As index cashflow but also is relative to a fixed value which is not the index,
	    basically just an additional constructor.
     */
    class BaseIndexedCashFlow : public IndexedCashFlow {
      public:
        BaseIndexedCashFlow(Real notional,
							const boost::shared_ptr<Index> &index,
							const Date& baseDate,
							Real baseFixing, 
							const Date& fixingDate,
							const Date& paymentDate,
							bool growthOnly = false,
							indexInterpolationType interpolation = iiINDEX, 
							const Frequency &frequency = QuantLib::NoFrequency
							)
        : IndexedCashFlow(notional, index, baseDate, fixingDate, paymentDate, growthOnly),
		  baseFixing_(baseFixing), interpolation_(interpolation), frequency_(frequency)
		{
			QL_REQUIRE(fabs(baseFixing_)>1e-16,"|baseFixing|<1e-16, future divide-by-zero error");
			if(interpolation_ != iiINDEX){
				QL_REQUIRE(frequency_ != QuantLib::NoFrequency,"non-index interpolation w/o frequency");
			}
				
			indexInterpolationType hi;
			hi = iiINDEX;
		}
		
		//! value used for index on base date - does not have to agree with index on that date
		virtual Real baseFixing() const;
		//! you may not have a valid date
		virtual Date baseDate() const;
		
		//! do you want linear/constant/as-index interpolation of future data?  
		virtual indexInterpolationType interpolation() const { return interpolation_; }
		virtual Frequency frequency() const { return frequency_; }
        
		//! redefine to use baseFixing() and interpolation
		virtual Real amount() const;
		
	protected:
		Real baseFixing_;
		indexInterpolationType interpolation_;
		Frequency frequency_;
	};

}







#endif
