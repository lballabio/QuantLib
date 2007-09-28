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

#include <ql/voltermstructures/interestrate/sabrvolsurface.hpp>
#include <ql/voltermstructures/smilesection.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    SabrVolSurface::SabrVolSurface(
        const boost::shared_ptr<InterestRateIndex>& index,
        const Handle<BlackAtmVolCurve>& atmCurve,
        const std::vector<Period>& optionTenors,
        const std::vector<Spread>& atmRateSpreads,
        const std::vector<std::vector<Handle<Quote> > >& volSpreads)
    : InterestRateVolSurface(index),
      atmCurve_(atmCurve),
      optionTenors_(optionTenors),
      optionTimes_(optionTenors.size()),
      optionDates_(optionTenors.size()),
      atmRateSpreads_(atmRateSpreads),
      volSpreads_(volSpreads) {

        // Checks
        Size nStrikes = atmRateSpreads_.size();
        QL_REQUIRE(nStrikes>1, "too few strikes (" << nStrikes << ")");
        for (Size i=1; i<nStrikes; ++i)
            QL_REQUIRE(atmRateSpreads_[i-1]<atmRateSpreads_[i],
                       "non increasing strike spreads: " <<
                       io::ordinal(i-1) << " is " << atmRateSpreads_[i-1] << ", " <<
                       io::ordinal(i) << " is " << atmRateSpreads_[i]);
        for (Size i=0; i<volSpreads_.size(); i++)
            QL_REQUIRE(atmRateSpreads_.size()==volSpreads_[i].size(),
                       "mismatch between number of strikes (" << atmRateSpreads_.size() <<
                       ") and number of columns (" << volSpreads_[i].size() <<
                       ") in the " << io::ordinal(i) << " row");

        // Creation of reference smile sections
        Size nOptions = optionTenors_.size();
        Size nAtmRateSpreads = atmRateSpreads_.size();

        // Hard coded
        isAlphaFixed_ = false;
        isBetaFixed_ = false;
        isNuFixed_ = false;
        isRhoFixed_ = false;
        vegaWeighted_ = true;

        sabrGuesses_.resize(nOptions);
        
        for (Size i=0; i<nOptions; ++i) {
                        
            optionDates_[i] = optionDateFromTenor(optionTenors_[i]);
            optionTimes_[i] = timeFromReference(optionDates_[i]);            
              
            // Hard coded
            sabrGuesses_[i][0] = 0.025; // alpha
            sabrGuesses_[i][1] = 0.5;   // beta
            sabrGuesses_[i][2] = 0.3;   // rho
            sabrGuesses_[i][3] = 0.0;   // nu
        }

        // Register market data
        for (Size i=0; i<nOptions; ++i) {
            for (Size j=0; j<nAtmRateSpreads; ++j) {        
                registerWith(volSpreads_[i][j]);
            }
        }

    }

    boost::array<Real, 4> SabrVolSurface::sabrGuesses(const Date& d) const {
    
        // the guesses for sabr parameters are assumed to be piecewise constant
        Size i=0;
        while( d<=optionDates_[i] && i<optionDates_.size())
            i++;
        return sabrGuesses_[i];
    }
    
    void SabrVolSurface::updateSabrGuesses(const Date& d, boost::array<Real, 4> newGuesses) {

        Size i=0;
        while( d<=optionDates_[i] && i<optionDates_.size())
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

    void SabrVolSurface::performCalculations () const {
    
    }
    
    
    boost::shared_ptr<SmileSection>
    SabrVolSurface::smileSectionImpl(Time t) const {

        BigInteger n = BigInteger(t*365.0);
        Date d = referenceDate()+n*Days;   
        Size s = atmRateSpreads_.size();
        std::vector<Rate> strikes(s);
        std::vector<Volatility> vols(s);
        // interpolating on ref smile sections
        std::vector<Volatility> volSpreads = volatilitySpreads(d);
        Rate atmRate = index_->forecastFixing(d);
        Volatility atmVol = atmCurve_->atmVol(d);
        for (Size i=0; i<s; ++i) {
            strikes[i] = atmRate + atmRateSpreads_[i];
            vols[i] = atmVol + volSpreads[i];
        }

        // calculate sabr fit
        boost::array<Real, 4> sabrParameters1 = sabrGuesses(d);
        const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
            boost::shared_ptr<SABRInterpolation>(new
                SABRInterpolation(strikes.begin(), strikes.end(),
                                  vols.begin(),
                                  timeFromReference(d),
                                  atmRate,
                                  sabrParameters1[0], sabrParameters1[1],
                                  sabrParameters1[2], sabrParameters1[3],
                                  isAlphaFixed_,
                                  isBetaFixed_,
                                  isNuFixed_,
                                  isRhoFixed_,
                                  vegaWeighted_));
        sabrInterpolation->update();
        QL_REQUIRE(sabrInterpolation->interpolationError()<0.01, "Bad interpolation");

        std::vector<Real> sabrParameters(4);
        sabrParameters[0] = sabrInterpolation->alpha();
        sabrParameters[1] = sabrInterpolation->beta();
        sabrParameters[2] = sabrInterpolation->nu();
        sabrParameters[3] = sabrInterpolation->rho();

        // Store sabr parameters
        //boost::array<Real, 4> g;
        //g[0] = sabrParameters[0];
        //g[1] = sabrParameters[1];
        //g[2] = sabrParameters[2];
        //g[3] = sabrParameters[3];
        //updateSabrGuesses(d, g);

        // create the smile section
        boost::shared_ptr<SmileSection> result(new
            SabrSmileSection(d, atmRate, sabrParameters, dayCounter()));
        return result;
    }

    void SabrVolSurface::accept(AcyclicVisitor& v) {
        Visitor<SabrVolSurface>* v1 =
            dynamic_cast<Visitor<SabrVolSurface>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            InterestRateVolSurface::accept(v);
    }

}
