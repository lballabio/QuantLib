/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Yue Tian

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

#include <ql/instruments/makeiboroisbasisswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    MakeIBOROISBasisSwap::MakeIBOROISBasisSwap(const Period& swapTenor,
									   const boost::shared_ptr<IborIndex>& iborIndex,
									   const boost::shared_ptr<OvernightIndex>& overnightIndex,
									   Rate spread,
									   const Period& fwdStart)
    : swapTenor_(swapTenor), floatingIndex_(iborIndex), overnightIndex_(overnightIndex),
	  overnightSpread_(spread), forwardStart_(fwdStart), 
	  type_(IBOROISBasisSwap::Payer), nominal_(1.0), fixingDays_(2),
	  effectiveDate_(Date()), terminationDate_(Date()),
	  paymentConvention_(ModifiedFollowing),
	  endOfMonth_(1*Months<=swapTenor && swapTenor<=2*Years ? true : false),
	  floatingLegTenor_(iborIndex->tenor()),
      floatingLegCalendar_(iborIndex->fixingCalendar()),
      floatingLegConvention_(iborIndex->businessDayConvention()),
      floatingLegTerminationDateConvention_(iborIndex->businessDayConvention()),
      floatingLegRule_(DateGeneration::Backward),
      floatingLegDayCount_(iborIndex->dayCounter()),
	  overnightLegTenor_(iborIndex->tenor()),//exchange at the end of each ibor period
      overnightLegCalendar_(overnightIndex->fixingCalendar()),
      overnightLegConvention_(overnightIndex->businessDayConvention()),
      overnightLegTerminationDateConvention_(overnightIndex->businessDayConvention()),
      overnightLegRule_(DateGeneration::Backward),
      overnightLegDayCount_(overnightIndex->dayCounter()),
	  engine_(new DiscountingSwapEngine(overnightIndex_->forwardingTermStructure())){}

    MakeIBOROISBasisSwap::operator IBOROISBasisSwap() const {
        boost::shared_ptr<IBOROISBasisSwap> oisbasis = *this;
        return *oisbasis;
    }

    MakeIBOROISBasisSwap::operator boost::shared_ptr<IBOROISBasisSwap>() const {

        const Calendar& calendar = overnightIndex_->fixingCalendar();

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date referenceDate = Settings::instance().evaluationDate();
            Date spotDate = calendar.advance(referenceDate,
                                             fixingDays_*Days);
            startDate = spotDate+forwardStart_;
        }

        Date endDate;
        if (terminationDate_ != Date()) {
            endDate = terminationDate_;
        } else {
            if (endOfMonth_) {
                endDate = calendar.advance(startDate, swapTenor_,
                                           ModifiedFollowing,
                                           endOfMonth_);
            } else {
                endDate = startDate+swapTenor_;
            }
        }

        Schedule floatingSchedule(startDate, endDate,
								  floatingLegTenor_,
                                  floatingLegCalendar_,
								  floatingLegConvention_,
								  floatingLegTerminationDateConvention_,
								  floatingLegRule_,
								  endOfMonth_);

		Schedule overnightSchedule(startDate, endDate,
								   overnightLegTenor_,
                                   overnightLegCalendar_,
								   overnightLegConvention_,
								   overnightLegTerminationDateConvention_,
								   overnightLegRule_,
								   endOfMonth_);

        Rate usedOvernightSpread = overnightSpread_;
        if (overnightSpread_ == Null<Rate>()) {
            QL_REQUIRE(!overnightIndex_->forwardingTermStructure().empty(),
                       "null term structure set to this instance of " <<
                       overnightIndex_->name());
            IBOROISBasisSwap temp(type_, nominal_,
                              floatingSchedule,
							  floatingIndex_,
							  floatingLegDayCount_,
							  overnightSchedule,
							  overnightIndex_,
                              0.0,
                              overnightLegDayCount_,
                              paymentConvention_);
            // ATM on the forecasting curve
            bool includeSettlementDateFlows = false;
            temp.setPricingEngine(boost::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(
                                   overnightIndex_->forwardingTermStructure(),
                                   includeSettlementDateFlows)));
            usedOvernightSpread = temp.fairSpread();
        }

        boost::shared_ptr<IBOROISBasisSwap> oisbasis(new
            IBOROISBasisSwap(type_, nominal_,
                              floatingSchedule,
							  floatingIndex_,
							  floatingLegDayCount_,
							  overnightSchedule,
							  overnightIndex_,
                              usedOvernightSpread,
                              overnightLegDayCount_,
                              paymentConvention_));
        oisbasis->setPricingEngine(engine_);
        return oisbasis;
    }


    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withType(IBOROISBasisSwap::Type type) {
        type_ = type;
        return *this;
    }

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withSettlementDays(Natural fixingDays) {
        fixingDays_ = fixingDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withPaymentConvention(BusinessDayConvention bdc) {
		paymentConvention_ = bdc;
        return *this;
	}

	MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withEndOfMonth(bool flag) {
		endOfMonth_ = flag;
		return *this;
	}

	MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withFloatingLegTenor(const Period& t) {
		floatingLegTenor_ = t;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withFloatingLegCalendar(const Calendar& cal) {
		floatingLegCalendar_ = cal;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
		floatingLegConvention_ = bdc;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withFloatingLegTerminationDateConvention(
                                                BusinessDayConvention bdc) {
		floatingLegTerminationDateConvention_ = bdc;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withFloatingLegRule(DateGeneration::Rule r) {
		floatingLegRule_ = r;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withFloatingLegDayCount(const DayCounter& dc) {
		floatingLegDayCount_ = dc;
		return *this;
	}

	MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegTenor(const Period& t) {
		overnightLegTenor_ = t;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegCalendar(const Calendar& cal) {
		overnightLegCalendar_ = cal;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegConvention(BusinessDayConvention bdc) {
		overnightLegConvention_ = bdc;
		return *this;
	}
    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegTerminationDateConvention(
                                                BusinessDayConvention bdc) {
		overnightLegTerminationDateConvention_ = bdc;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegRule(DateGeneration::Rule r) {
		overnightLegRule_ = r;
		if (r==DateGeneration::Zero)
            overnightLegTenor_ = Period(Once);
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegDayCount(const DayCounter& dc) {
		overnightLegDayCount_ = dc;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withOvernightLegSpread(Spread sp) {
		overnightSpread_ = sp;
		return *this;
	}

    MakeIBOROISBasisSwap& MakeIBOROISBasisSwap::withDiscountingTermStructure(
                const Handle<YieldTermStructure>& discountingTermStructure) {
		engine_ = boost::shared_ptr<PricingEngine>(new
                            DiscountingSwapEngine(discountingTermStructure));
        return *this;
	}

}
