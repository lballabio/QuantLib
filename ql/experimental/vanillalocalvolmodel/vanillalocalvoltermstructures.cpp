/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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

/*! \file vanillalocalvoltermstructures.cpp
    \brief swaption volatility term structure based on VanillaLocalVolModel
*/

#include <ql/handle.hpp>
#include <ql/indexes/swapindex.hpp>

#include <ql/experimental/vanillalocalvolmodel/vanillalocalvoltermstructures.hpp>
#include <ql/experimental/vanillalocalvolmodel/vanillalocalvolsmilesection.hpp>


namespace QuantLib {

	VanillaLocalVolSwaptionVTS::VanillaLocalVolSwaptionVTS(
		const Handle<SwaptionVolatilityStructure>&                                              atmVolTS,
		const std::vector< std::vector< ext::shared_ptr<VanillaLocalVolModelSmileSection> > >&  smiles,
		const std::vector< Period >&                                                            swapTerms,
		const ext::shared_ptr<SwapIndex>&                                                       index)
	: SwaptionVolatilityStructure(atmVolTS->referenceDate(),atmVolTS->calendar(),atmVolTS->businessDayConvention(), atmVolTS->dayCounter()),
		atmVolTS_(atmVolTS), smiles_(smiles), swapTerms_(swapTerms), index_(index) {
		QL_REQUIRE(!atmVolTS_.empty(), "atmVolTS required");
		QL_REQUIRE(smiles_.size() == swapTerms_.size(), "smiles_.size()==swapTerms_.size() required");
		for (Size k = 1; k < swapTerms_.size(); ++k) {
			QL_REQUIRE(months(swapTerms_[k - 1]) < months(swapTerms_[k]), "months(swapTerms_[k-1])<months(swapTerms_[k]) required");
		}
		for (Size k = 0; k < smiles_.size(); ++k) {
			QL_REQUIRE(smiles_[k].size() > 0, "smiles_[k].size()>0 required");
			for (Size i = 1; i < smiles_[k].size(); ++i) {
				QL_REQUIRE(smiles_[k][i - 1]->exerciseTime() < smiles_[k][i]->exerciseTime(), "smiles_[k][i-1]->exerciseTime()<smiles_[k][i]->exerciseTime() required");
			}
		}
	}

	ext::shared_ptr<SmileSection> VanillaLocalVolSwaptionVTS::smileSectionImpl( Time optionTime, Time swapLength) const {
		if (smiles_.size() == 0) return atmVolTS_->smileSection(optionTime, swapLength);  // fall back
		// we need to convert back times to periods and dates
		Period optionDays((Integer)round(365.0 * optionTime), Days);
		Date optionDate = referenceDate() + optionDays;
		Period swapTerm((Integer)round(12.0 * swapLength), Months);
		// first we interpolate in expiry direction for two swap term columns
		std::vector< Size > idxj(2);  // swap term indices enclosing swap length
		std::vector< ext::shared_ptr<VanillaLocalVolModelSmileSection> > smilesj(2);
		idxj[0] = swapTerms_.size() - 1;
		idxj[1] = 0;
		while ((idxj[0] > 0)                     && (months(swapTerms_[idxj[0]]) > months(swapTerm))) --idxj[0];
		while ((idxj[1] < swapTerms_.size() - 1) && (months(swapTerms_[idxj[1]]) < months(swapTerm))) ++idxj[1];
		if (smiles_[idxj[0]].size() == 0) return atmVolTS_->smileSection(optionTime, swapLength);  // fall back
		if (smiles_[idxj[1]].size() == 0) return atmVolTS_->smileSection(optionTime, swapLength);  // fall back
		Real rhoj = 0.5;
		if (months(swapTerms_[idxj[0]]) < months(swapTerms_[idxj[1]]))
			rhoj = (round(12.0 * swapLength) - months(swapTerms_[idxj[0]])) / (months(swapTerms_[idxj[1]]) - months(swapTerms_[idxj[0]]));
		for (Size k = 0; k < 2; ++k) {
			std::vector< Size > idxi(2);  // expiry indices enclosing optionTime
			idxi[0] = smiles_[idxj[k]].size() - 1;
			idxi[1] = 0;
			while ((idxi[0] > 0)                           && (smiles_[idxj[k]][idxi[0]]->exerciseTime() > optionTime)) --idxi[0];
			while ((idxi[1] < smiles_[idxj[k]].size() - 1) && (smiles_[idxj[k]][idxi[1]]->exerciseTime() < optionTime)) ++idxi[1];
			Real rhoi = 0.5;
			if (smiles_[idxj[k]][idxi[0]]->exerciseTime() < smiles_[idxj[k]][idxi[1]]->exerciseTime())
				rhoi = (optionTime - smiles_[idxj[k]][idxi[0]]->exerciseTime()) / (smiles_[idxj[k]][idxi[1]]->exerciseTime() - smiles_[idxj[k]][idxi[0]]->exerciseTime());
			// now we may interpolate in expiry direction
			Date fixingDate = index_->fixingCalendar().adjust(optionDate, Following); // we can only calculate a forward for a good business day
			Rate forward = index_->clone(swapTerms_[idxj[k]])->fixing(fixingDate);  // this might fail if optionDate is no good business day
			Real atmVol = atmVolTS_->volatility(optionTime, swapLength, forward);
			smilesj[k] = ext::shared_ptr<VanillaLocalVolModelSmileSection>(
				new VanillaLocalVolModelSmileSection(optionDate, forward, atmVol, smiles_[idxj[k]][idxi[0]], smiles_[idxj[k]][idxi[1]], rhoi, true, dayCounter(), referenceDate(), volatilityType(), atmVolTS_->shift(optionDate, swapLength))); // shift relies on same volatilityType as ATM	vol
		}

		Date fixingDate = index_->fixingCalendar().adjust(optionDate, Following); // we can only calculate a forward for a good business day
		Rate forward = index_->clone(swapTerm)->fixing(fixingDate);  // this might fail if optionDate is no good business day
		Real atmVol = atmVolTS_->volatility(optionTime, swapLength, forward); // we want to be as accuarate as possible on the ATM interpolation thus using times
		ext::shared_ptr<VanillaLocalVolModelSmileSection> smile(new VanillaLocalVolModelSmileSection(optionDate, forward, atmVol, smilesj[0], smilesj[1], rhoj, true, dayCounter(), referenceDate(), volatilityType(), atmVolTS_->shift(optionDate, swapLength)));
		return smile;
	}

}

