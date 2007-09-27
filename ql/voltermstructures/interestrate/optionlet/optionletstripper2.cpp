/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/voltermstructures/interestrate/optionlet/optionletstripper2.hpp>
#include <ql/voltermstructures/interestrate/capfloor/capfloortermvolcurve.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/indexes/iborindex.hpp>


namespace QuantLib {

//===========================================================================//
//                              OptionletStripper2                           //
//===========================================================================//

    OptionletStripper2::OptionletStripper2(
                    const Handle<OptionletStripper>& optionletStripper,
                    const Handle<CapFloorTermVolCurve>& atmCapFloorTermVolCurve,
                    const std::vector<Rate>& atmStrikes,
                    Real alpha,
                    Real beta,
                    Real nu,
                    Real rho,
                    bool isAlphaFixed,
                    bool isBetaFixed,
                    bool isNuFixed,
                    bool isRhoFixed,
                    bool vegaWeighted,
                    const boost::shared_ptr<EndCriteria>& endCriteria,
                    const boost::shared_ptr<OptimizationMethod>& method)
    : optionletStripper_(optionletStripper),
      atmCapFloorTermVolCurve_(atmCapFloorTermVolCurve),
      dc_(optionletStripper_->surface()->dayCounter()),
      nOptionExpiries_(atmStrikes.size()),
      atmStrikes_(atmStrikes),
      mdlOptionletVols_(atmStrikes.size()),
      spreadsVolImplied_(atmStrikes.size()),
      calibratedOptionletVols_(atmStrikes.size()),
      atmOptionPrice_(atmStrikes.size()),
      caplets_(atmStrikes.size()),
      caps_(atmStrikes.size()),
      maxEvaluations_(10000),
      accuracy_(1.e-6),
      alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
      isAlphaFixed_(isAlphaFixed_), isBetaFixed_(isBetaFixed_), 
      isNuFixed_(isNuFixed_), isRhoFixed_(isRhoFixed_),
      vegaWeighted_(vegaWeighted),
      endCriteria_(endCriteria),
      method_(method)
    {
        registerWith(optionletStripper_);
        registerWith(atmCapFloorTermVolCurve_);
     }

    void OptionletStripper2::performCalculations() const {
       
        // optionletStripper data
        const std::vector<Rate>& strikes = optionletStripper_->strikes();
        const std::vector<Period>& optionletExpiriesTenors = optionletStripper_->optionletTenors();
        const std::vector<Time>& optionletExpiriesTimes = optionletStripper_->optionletTimes();
        const std::vector<Date>& optionletExpiriesDates = optionletStripper_->optionletDates(); 
        const Matrix& optionletVolatilities = optionletStripper_->optionletVolatilities();
        const boost::shared_ptr<IborIndex>& index = optionletStripper_->index();
        Size nOptionletExpiries = optionletExpiriesTenors.size();

        // atmCapFloorTermVolCurve data
        const std::vector<Period>& optionExpiriesTenors = atmCapFloorTermVolCurve_->optionTenors();
        const std::vector<Time>& optionExpiriesTimes = atmCapFloorTermVolCurve_->optionTimes();
        
        QL_REQUIRE(nOptionExpiries_==optionExpiriesTenors.size(),
                   "nOptionExpiries_!=optionExpiriesTenors.size()");

        QL_REQUIRE(nOptionExpiries_==optionletVolatilities.columns(),
                   "nOptionExpiries_(" << nOptionExpiries_ <<
                   ")!=optionletVolatilities.columns() (" << optionletVolatilities.columns()<<")");
        QL_REQUIRE(nOptionletExpiries==optionletVolatilities.rows(),
                   "nOptionletExpiries(" << nOptionletExpiries <<
                   ")!=optionletVolatilities.rows() (" << optionletVolatilities.rows()<<")");  

        std::vector<boost::shared_ptr<SabrInterpolatedSmileSection> > smileSections;

        for (Size i=0; i<nOptionletExpiries; ++i) {
            Handle<Quote> forward(boost::shared_ptr<Quote>(new
                SimpleQuote(index->fixing(optionletExpiriesDates[i]))));
            std::vector<Handle<Quote> > stdDevHandles;
            stdDevHandles = std::vector<Handle<Quote> >(strikes.size());
            for (Size j=0; j<strikes.size(); ++j) {
                stdDevHandles[j] = Handle<Quote>( boost::shared_ptr<Quote>(new
                    SimpleQuote(optionletVolatilities[i][j]*std::sqrt(optionletExpiriesTimes[i]))));
            }
            smileSections.push_back( 
                boost::shared_ptr<SabrInterpolatedSmileSection>(new
                    SabrInterpolatedSmileSection(
                               optionletExpiriesDates[i],
                               strikes,
                               stdDevHandles,
                               forward,
                               alpha_, beta_, nu_, rho_,
                               isAlphaFixed_, isBetaFixed_,
                               isNuFixed_, isRhoFixed_, vegaWeighted_,
                               endCriteria_, method_, dc_)
                )
            );
        }
        
        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            // atm option price 
            Rate dummyStrike = 0.;
            Volatility atmOptionVol = atmCapFloorTermVolCurve_->volatility(optionExpiriesTimes[optionIndex],dummyStrike);
            boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(atmOptionVol, dc_));
            caps_[optionIndex] = MakeCapFloor(CapFloor::Cap,
                                            optionExpiriesTenors[optionIndex], index,
                                            strikes[optionIndex], 0*Days, engine);  
            atmOptionPrice_[optionIndex] = caps_[optionIndex]->NPV();
            
            std::vector<double> tmp;
            std::vector<boost::shared_ptr<CapFloor> > tmpCaplet;
            for (Size i=0; i<nOptionletExpiries; ++i) {
                if(i <= caps_[optionIndex]->leg().size()){
                    tmp.push_back(smileSections[i]->volatility(atmStrikes_[optionIndex]));
                    //boost::shared_ptr<BlackCapFloorEngine> engine1(new
                    // BlackCapFloorEngine(smileSections[i]->volatility(atmStrikes_[optionIndex]), dc_));
                    tmpCaplet.push_back(MakeCapFloor(CapFloor::Cap,
                                                index->tenor(), index,
                                                strikes[optionIndex], optionletExpiriesTenors[i]-index->tenor(), engine));
                    //tmp.push_back(tmpCaplet.back()->NPV());
                }
            }
            mdlOptionletVols_[optionIndex] = tmp;
            caplets_[optionIndex] = tmpCaplet;
        }

        spreadsVolImplied_ = spreadsVolImplied();
        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            std::vector<double> tmp;
            for (Size i=0; i<mdlOptionletVols_[optionIndex].size(); ++i) {
                tmp.push_back(mdlOptionletVols_[optionIndex][i] + spreadsVolImplied_[optionIndex]);
            }
            calibratedOptionletVols_[optionIndex] = tmp;
        }     
    }

    std::vector<double> OptionletStripper2::spreadsVolImplied() const {
        
        std::vector<double> result;
        Volatility guess = 0.0001, minVol = -0.1, maxVol = 0.1;
        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {  
            ObjectiveFunction f(caplets_[optionIndex], mdlOptionletVols_[optionIndex],
                                dc_, atmOptionPrice_[optionIndex]);
            Brent solver;
            solver.setMaxEvaluations(maxEvaluations_);
            result.push_back(solver.solve(f, accuracy_, guess, minVol, maxVol));
        }
        return result;      
    }
        
    std::vector<double> OptionletStripper2::spreadsVol() const {
        calculate();
        return spreadsVolImplied_;
    };
    
    std::vector<double> OptionletStripper2::atmOptionPrice() const {
        calculate();
        return atmOptionPrice_;
    };

    std::vector<double> OptionletStripper2::mdlOptionletVols(Size i) const {
        calculate();
        return mdlOptionletVols_[i];
    };

//===========================================================================//
//                 OptionletStripper2::ObjectiveFunction                     //
//===========================================================================//

    OptionletStripper2::ObjectiveFunction::ObjectiveFunction(
                const std::vector<boost::shared_ptr<CapFloor> >& caplets,
                const std::vector<double>& modVols,
                const DayCounter& dc,
                Real targetValue):
       caplets_(caplets),
       modVols_(modVols),
       dc_(dc),
       targetValue_(targetValue){ 
               
           QL_REQUIRE(caplets_.size() == modVols_.size(),
                   "caplets_.size() (" << caplets_.size()<<
                   ")!=modVols_.size() (" << modVols_.size()<<")");
       }
        
    Real OptionletStripper2::ObjectiveFunction::operator()(Volatility spreadVol) const {
        Real price =0.;
        for (Size i=0; i<caplets_.size(); ++i) {
            boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(modVols_[i]+spreadVol, dc_));
            caplets_[i]->setPricingEngine(engine);
            price += caplets_[i]->NPV();
        }
        return price-targetValue_;
    }
}
