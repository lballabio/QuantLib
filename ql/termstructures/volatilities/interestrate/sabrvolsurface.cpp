/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/termstructures/volatilities/interestrate/sabrvolsurface.hpp>
#include <ql/termstructures/volatilities/smilesection.hpp>

namespace QuantLib {

    SabrVolSurface::SabrVolSurface(
        const Handle<AbcdAtmVolCurve>& atmCurve,
        const boost::shared_ptr<InterestRateIndex>& index,
        const std::vector<Period>& optionTenors,
        const std::vector<Spread>& atmRateSpreads,
        const std::vector<std::vector<Handle<Quote> > >& volSpreads)
    : InterestRateVolSurface(index),
      atmCurve_(atmCurve),
      atmRateSpreads_(atmRateSpreads),
      optionTenors_(optionTenors),
      volSpreads_(volSpreads) {

          // checks

          // create ref smile sections
    }

    boost::shared_ptr<SmileSection> 
    SabrVolSurface::smileSectionImpl(Time t) const {

        BigInteger n = BigInteger(t*365.0);
        Date d = referenceDate()+n*Days;

        Rate atmRate = index_->forecastFixing(d);
        Volatility atmVol = atmCurve_->atmVol(d);

        Size s = atmRateSpreads_.size();
        std::vector<Rate> strikes(s);
        std::vector<Volatility> vols(s);
        // interpolating on ref smile sections
        std::vector<Volatility> volSpreads = volatilitySpreads(d);
        for (Size i=0; i<s; ++i) {
            strikes[i]=atmRate+atmRateSpreads_[i];
            vols[i]=atmVol+volSpreads[i];
        }

        // calculate sabr fit
        boost::array<Real, 4> sabrParameters1 = sabrGuesses(d);


        std::vector<Real> sabrParameters;
        // create the smile section
        boost::shared_ptr<SmileSection> result(new
            SabrSmileSection(d, atmRate, sabrParameters, dayCounter()));
        return result;
    }

}
