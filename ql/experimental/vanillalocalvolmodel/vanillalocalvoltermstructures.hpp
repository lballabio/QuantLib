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

/*! \file vanillalocalvoltermstructures.hpp
    \brief swaption volatility term structure based on VanillaLocalVolModel
*/

#ifndef quantlib_vanillalocalvoltermstructures_hpp
#define quantlib_vanillalocalvoltermstructures_hpp

#include <boost/shared_ptr.hpp>

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>


namespace QuantLib {

	class VanillaLocalVolModelSmileSection;
	class SwapIndex;

    class VanillaLocalVolSwaptionVTS : public SwaptionVolatilityStructure {
    private:
		const Handle<SwaptionVolatilityStructure> atmVolTS_;
		const std::vector< std::vector< ext::shared_ptr<VanillaLocalVolModelSmileSection> > > smiles_;
		const std::vector< Period > swapTerms_;
		const ext::shared_ptr<SwapIndex> index_;  // a template for all the swap index per swap terms
	protected:
		virtual ext::shared_ptr<SmileSection> smileSectionImpl(
			Time optionTime,
			Time swapLength) const;

		virtual Volatility volatilityImpl(Time optionTime,
			Time swapLength,
			Rate strike) const {
			return smileSectionImpl(optionTime, swapLength)->volatility(strike);
		}

    public:
		VanillaLocalVolSwaptionVTS(
			const Handle<SwaptionVolatilityStructure>&                                              atmVolTS,
			const std::vector< std::vector< ext::shared_ptr<VanillaLocalVolModelSmileSection> > >&  smiles,
			const std::vector< Period >&                                                            swapTerms,
			const ext::shared_ptr<SwapIndex>&                                                       index);

		virtual VolatilityType volatilityType() const { return atmVolTS_->volatilityType(); }

		virtual const Date&   referenceDate() const { return atmVolTS_->referenceDate(); }
        virtual const Period& maxSwapTenor()  const { return atmVolTS_->maxSwapTenor();  }
		virtual       Date    maxDate()       const { return atmVolTS_->maxDate();       }
		virtual       Rate    minStrike()     const { return atmVolTS_->minStrike();     }
		virtual       Rate    maxStrike()     const { return atmVolTS_->maxStrike();     }

    };


}

#endif
