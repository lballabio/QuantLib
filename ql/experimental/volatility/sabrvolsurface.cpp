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

#include <ql/experimental/volatility/sabrvolsurface.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    SabrVolSurface::SabrVolSurface(const std::shared_ptr<InterestRateIndex>& index,
                                   Handle<BlackAtmVolCurve> atmCurve,
                                   const std::vector<Period>& optionTenors,
                                   std::vector<Spread> atmRateSpreads,
                                   std::vector<std::vector<Handle<Quote> > > volSpreads)
    : InterestRateVolSurface(index), atmCurve_(std::move(atmCurve)), optionTenors_(optionTenors),
      optionTimes_(optionTenors.size()), optionDates_(optionTenors.size()),
      atmRateSpreads_(std::move(atmRateSpreads)), volSpreads_(std::move(volSpreads)) {

        checkInputs();

        // Creation of reference smile sections

        // Hard coded
        isAlphaFixed_ = false;
        isBetaFixed_ = false;
        isNuFixed_ = false;
        isRhoFixed_ = false;
        vegaWeighted_ = true;

        sabrGuesses_.resize(optionTenors_.size());

        for (Size i=0; i<optionTenors_.size(); ++i) {

            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(optionDates_[i]);

            // Hard coded
            sabrGuesses_[i][0] = 0.025; // alpha
            sabrGuesses_[i][1] = 0.5;   // beta
            sabrGuesses_[i][2] = 0.3;   // rho
            sabrGuesses_[i][3] = 0.0;   // nu
        }
        registerWithMarketData();
    }

    std::array<Real, 4> SabrVolSurface::sabrGuesses(const Date& d) const {

        // the guesses for sabr parameters are assumed to be piecewise constant
        if (d<=optionDates_[0]) return sabrGuesses_[0];
        Size i=0;
        while (i<optionDates_.size()-1 && d<optionDates_[i])
            ++i;
        return sabrGuesses_[i];
    }

    void SabrVolSurface::updateSabrGuesses(const Date& d, std::array<Real, 4> newGuesses) const {

        Size i=0;
        while (i<optionDates_.size() && d<=optionDates_[i])
            ++i;
        sabrGuesses_[i][0] = newGuesses[0];
        sabrGuesses_[i][1] = newGuesses[1];
        sabrGuesses_[i][2] = newGuesses[2];
        sabrGuesses_[i][3] = newGuesses[3];

    }

    std::vector<Volatility> SabrVolSurface::volatilitySpreads(const Date& d) const {

        Size nOptionsTimes = optionTimes_.size();
        Size nAtmRateSpreads = atmRateSpreads_.size();
        std::vector<Volatility> interpolatedVols(nAtmRateSpreads);

        std::vector<Volatility> vols(nOptionsTimes); // the volspread at a given strike
        for (Size i=0; i<nAtmRateSpreads; ++i) {
            for (Size j=0; j<nOptionsTimes; ++j) {
                vols[j] = (**volSpreads_[j][i]).value();
            }
            LinearInterpolation interpolator(optionTimes_.begin(), optionTimes_.end(),
                                             vols.begin());
            interpolatedVols[i] = interpolator(timeFromReference(d),true);
        }
        return interpolatedVols;
    }


    void SabrVolSurface::update() {
        TermStructure::update();
        for (Size i=0; i<optionTenors_.size(); ++i) {
            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(optionDates_[i]);
        }
        notifyObservers();

    }

    std::shared_ptr<SmileSection>
    SabrVolSurface::smileSectionImpl(Time t) const {

        auto n = BigInteger(t * 365.0);
        Date d = referenceDate()+n*Days;
        // interpolating on ref smile sections
        std::vector<Volatility> volSpreads = volatilitySpreads(d);

        // calculate sabr fit
        std::array<Real, 4> sabrParameters1 = sabrGuesses(d);

        std::shared_ptr<SabrInterpolatedSmileSection> tmp(new
            SabrInterpolatedSmileSection(d,
                                         index_->fixing(d,true), atmRateSpreads_, true,
                                            atmCurve_->atmVol(d), volSpreads,
                                            sabrParameters1[0], sabrParameters1[1],
                                            sabrParameters1[2], sabrParameters1[3],
                                            isAlphaFixed_, isBetaFixed_,
                                            isNuFixed_, isRhoFixed_,
                                            vegaWeighted_/*,
                                            const std::shared_ptr<EndCriteria>& endCriteria,
                                            const std::shared_ptr<OptimizationMethod>& method,
                                            const DayCounter& dc*/));

        // update guess

        return tmp;

    }

    void SabrVolSurface::registerWithMarketData() {

        for (Size i=0; i<optionTenors_.size(); ++i) {
            for (Size j=0; j<atmRateSpreads_.size(); ++j) {
                registerWith(volSpreads_[i][j]);
            }
        }
    }

    void SabrVolSurface::checkInputs() const {

        Size nStrikes = atmRateSpreads_.size();
        QL_REQUIRE(nStrikes>1, "too few strikes (" << nStrikes << ")");
        for (Size i=1; i<nStrikes; ++i)
            QL_REQUIRE(atmRateSpreads_[i-1]<atmRateSpreads_[i],
                       "non increasing strike spreads: " <<
                       io::ordinal(i) << " is " << atmRateSpreads_[i-1] << ", " <<
                       io::ordinal(i+1) << " is " << atmRateSpreads_[i]);
        for (Size i=0; i<volSpreads_.size(); i++)
            QL_REQUIRE(atmRateSpreads_.size()==volSpreads_[i].size(),
                       "mismatch between number of strikes (" << atmRateSpreads_.size() <<
                       ") and number of columns (" << volSpreads_[i].size() <<
                       ") in the " << io::ordinal(i+1) << " row");
    }

    void SabrVolSurface::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SabrVolSurface>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            InterestRateVolSurface::accept(v);
    }

}
