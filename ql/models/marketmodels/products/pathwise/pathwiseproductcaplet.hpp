/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

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


#ifndef quantlib_market_model_pathwise_caplet_hpp
#define quantlib_market_model_pathwise_caplet_hpp

#include <ql/types.hpp>
#include <ql/models/marketmodels/pathwisemultiproduct.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <vector>
#include <memory>

namespace QuantLib {

    class EvolutionDescription;
    class CurveState;

    //! market-model pathwise caplet
    /*! implementation of path wise methodology for caplets, essentially a test class
    since we have better ways of computing Greeks of caplets

    used in   MarketModelTest::testPathwiseVegas
    and       MarketModelTest::testPathwiseGreeks

    */

    class MarketModelPathwiseMultiCaplet : public MarketModelPathwiseMultiProduct
    {
     public:

       MarketModelPathwiseMultiCaplet(
                          const std::vector<Time>& rateTimes,
                          const std::vector<Real>& accruals,
                          const std::vector<Time>& paymentTimes,
                          const std::vector<Rate>& strikes);

        virtual ~MarketModelPathwiseMultiCaplet() {}

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
        std::vector<Real> rateTimes_;
        std::vector<Real> accruals_;
        std::vector<Time> paymentTimes_;
        std::vector<Rate> strikes_;
        Size numberRates_;
        // things that vary in a path
        Size currentIndex_;

        EvolutionDescription evolution_;
    };

    class MarketModelPathwiseMultiDeflatedCaplet : public MarketModelPathwiseMultiProduct
    {
     public:

       MarketModelPathwiseMultiDeflatedCaplet(
                          const std::vector<Time>& rateTimes,
                          const std::vector<Real>& accruals,
                          const std::vector<Time>& paymentTimes,
                          const std::vector<Rate>& strikes);

       MarketModelPathwiseMultiDeflatedCaplet(const std::vector<Time>& rateTimes,
                                     const std::vector<Real>& accruals,
                                     const std::vector<Time>& paymentTimes,
                                     Rate strike);

        virtual ~MarketModelPathwiseMultiDeflatedCaplet() {}

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
        std::vector<Real> rateTimes_;
        std::vector<Real> accruals_;
        std::vector<Time> paymentTimes_;
        std::vector<Rate> strikes_;
        Size numberRates_;
        // things that vary in a path
        Size currentIndex_;

        EvolutionDescription evolution_;
    };

    /*! MarketModelPathwiseMultiDeflatedCap to price several caps and get their derivatives
    simultaneously. Mainly useful for testing pathwise market vegas code.

    */

  class MarketModelPathwiseMultiDeflatedCap : public MarketModelPathwiseMultiProduct
    {
     public:

       MarketModelPathwiseMultiDeflatedCap(
                          const std::vector<Time>& rateTimes,
                          const std::vector<Real>& accruals,
                          const std::vector<Time>& paymentTimes,
                          Rate strike,
                          const std::vector<std::pair<Size,Size> >& startsAndEnds);



        virtual ~MarketModelPathwiseMultiDeflatedCap() {}

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
        MarketModelPathwiseMultiDeflatedCaplet underlyingCaplets_;

        Size numberRates_;

        std::vector<std::pair<Size,Size> > startsAndEnds_;

        // things that vary in a path
        Size currentIndex_;
        std::vector<Size> innerCashFlowSizes_;
        std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> > innerCashFlowsGenerated_;

    };


}
#endif
