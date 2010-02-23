/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Mark Joshi

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


#ifndef quantlib_market_model_pathwise_inverse_floater_hpp
#define quantlib_market_model_pathwise_inverse_floater_hpp

#include <ql/types.hpp>
#include <ql/models/marketmodels/pathwisemultiproduct.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <vector>
#include <memory>

namespace QuantLib 
{

    class EvolutionDescription;
    class CurveState;

    /*!
    Pathwise product inverse floater for doing Greeks
    Tested in MarketModels::testInverseFloater()

    */
class MarketModelPathwiseInverseFloater : public MarketModelPathwiseMultiProduct
    {
     public:



         MarketModelPathwiseInverseFloater(const std::vector<Time>& rateTimes,
                      const std::vector<Real>& fixedAccruals,
                      const std::vector<Real>& floatingAccruals,
                      const std::vector<Real>& fixedStrikes,
                      const std::vector<Real>& fixedMultipliers, 
                      const std::vector<Real>& floatingSpreads,
                      const std::vector<Time>& paymentTimes,
                      bool payer = true);

        virtual ~MarketModelPathwiseInverseFloater() {}

        virtual std::vector<Size> suggestedNumeraires() const;
        virtual const EvolutionDescription& evolution() const;
        virtual std::vector<Time> possibleCashFlowTimes() const;
        virtual Size numberOfProducts() const;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const;

        // has division by the numeraire already been done?
        virtual bool alreadyDeflated() const;


        //! during simulation put product at start of path
        virtual void reset();

        //! return value indicates whether path is finished, TRUE means done
        virtual bool nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated) ;

        //! returns a newly-allocated copy of itself
        virtual std::auto_ptr<MarketModelPathwiseMultiProduct> clone() const;

    private:
        std::vector<Real> rateTimes_,fixedAccruals_, floatingAccruals_,fixedStrikes_, fixedMultipliers_, floatingSpreads_;
        std::vector<Time> paymentTimes_;
    
        bool payer_;
        Real multiplier_;
        Size lastIndex_;
        
        EvolutionDescription evolution_;

        // things that vary in a path
        Size currentIndex_;

    };
}

#endif
