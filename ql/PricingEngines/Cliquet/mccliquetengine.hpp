
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mccliquetengine.hpp
    \brief Monte Carlo Cliquet option engine
*/

#ifndef quantlib_mc_cliquet_engine_hpp
#define quantlib_mc_cliquet_engine_hpp

#include <ql/Instruments/cliquetoption.hpp>
#include <ql/PricingEngines/mcsimulation.hpp>

namespace QuantLib {

    /*

    //! Monte Carlo cliquet engine
    template<class S, class SG, class PG>
    class McCliquetEngine : public CliquetEngine,
                            public McSimulation<S, PG, PathPricer<Path> > {
      public:
        McCliquetEngine(bool antitheticVariate,
                        bool controlVariate,
                        Size maxTimeStepPerYear,
                        SG sequenceGenerator)
        : McSimulation<S, PG, PathPricer<Path> >(antitheticVariate,
                                                 controlVariate), 
          maxTimeStepPerYear_(maxTimeStepPerYear),
          sequenceGenerator_(sequenceGenerator) {}
        void calculate() const;
      protected:
        TimeGrid timeGrid() const;
        boost::shared_ptr<PG> pathGenerator() const;
        boost::shared_ptr<PathPricer<Path> > pathPricer() const;
      private:
        Size maxTimeStepPerYear_;
        SG sequenceGenerator_;
    };


    class CliquetOptionPathPricer : public PathPricer<Path> {
      public:
        CliquetOptionPathPricer(
                           Option::Type type,
                           double underlying,
                           double moneyness,
                           double accruedCoupon,
                           double lastFixing,
                           double localCap,
                           double localFloor,
                           double globalCap,
                           double globalFloor,
                           bool redemptionOnly,
                           const RelinkableHandle<TermStructure>& riskFreeTS);
        double operator()(const Path& path) const;
      private:
        Option::Type type_;
        double underlying_, moneyness_, accruedCoupon_;
        double lastFixing_, localCap_, localFloor_, globalCap_, globalFloor_;
        bool redemptionOnly_;
        RelinkableHandle<TermStructure> riskFreeTS_;
    };


    // template definitions

    template<class S, class SG, class PG>
    inline TimeGrid McCliquetEngine<S, SG, PG>::timeGrid() const {
        std::vector<Time> resetTimes;
        Date referenceDate = arguments.riskFreeTS->referenceDate();
        for (Size i = 0; i< arguments.resetDates.size(); i++) {
            resetTimes[i] = 
                arguments.riskFreeTS->dayCount().yearFraction(
                                      referenceDate, arguments.resetDates[i]);
        }

        try {
            boost::shared_ptr<BlackConstantVol> constVolTS = 
                (*(arguments.blackVolTS)).currentLink();
            return TimeGrid(resetTimes.begin(), resetTimes.end());
        } catch (...) {
            return TimeGrid(resetTimes.begin(), resetTimes.end(),
                            resetTimes.back()*maxTimeStepsPerYear_);
        }

    }

    template<class S, class SG, class PG>
    inline
    boost::shared_ptr<PG> McCliquetEngine<S, SG, PG>::pathGenerator() const {
        boost::shared_ptr<DiffusionProcess> bs(
            new BlackScholesProcess(arguments_.riskFreeTS, 
                                    arguments_.dividendTS,
                                    arguments_.volTS, 
                                    arguments_.underlying));

        return boost::shared_ptr<PG>(new PG(bs, timeGrid(), 
                                            sequenceGenerator_));

    }

    template<class S, class SG, class PG>
    inline
    boost::shared_ptr<PathPricer<Path> >
    McCliquetEngine<S, SG, PG>::pathPricer() const {
        //! Initialize the path pricer
        return boost::shared_ptr<PathPricer<Path> >(
            new CliquetOptionPathPricer(arguments_.type,
                                        arguments_.underlying, 
                                        arguments_.moneyness,
                                        arguments_.accruedCoupon, 
                                        arguments_.lastFixing,
                                        arguments_.localCap, 
                                        arguments_.localFloor,
                                        arguments_.globalCap, 
                                        arguments_.globalFloor,
                                        arguments_.riskFreeTS));
    }

    template<class S, class SG, class PG>
    inline void McCliquetEngine<S, SG, PG>::calculate() const {

        QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                   "not an European Option");

        //! Initialize the one-factor Monte Carlo
        if (controlVariate_) {

            boost::shared_ptr<PathPricer<Path> >
                controlPP = controlPathPricer();
            QL_REQUIRE(!controlPP.isNull(),
                       "engine does not provide "
                       "control variation path pricer");

            boost::shared_ptr<PricingEngine> controlPE = 
                controlPricingEngine();

            QL_REQUIRE(!controlPE.isNull(),
                       "engine does not provide "
                       "control variation pricing engine");

            CliquetOption::arguments* controlArguments =
                dynamic_cast<CliquetOption::arguments*>(
                                                      controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const VanillaOption::results* controlResults =
                dynamic_cast<const VanillaOption::results*>(
                                                        controlPE->results());
            double controlVariateValue = controlResults->value;

            mcModel_ = 
                boost::shared_ptr<MonteCarloModel<S, PG, PathPricer<Path> > >(
                    new MonteCarloModel<S, PG, PathPricer<Path> >(
                        pathGenerator(), pathPricer(), S(), antitheticVariate_,
                        controlPP, controlVariateValue));

        } else {
            mcModel_ = 
                boost::shared_ptr<MonteCarloModel<S, PG, PathPricer<Path> > >(
                    new MonteCarloModel<S, PG, PathPricer<Path> >(
                        pathGenerator(), pathPricer(), S(), 
                        antitheticVariate_));
        }

        value(0.01);

        results_.value = mcModel_->sampleAccumulator().mean();
        results_.errorEstimate = 
            mcModel_->sampleAccumulator().errorEstimate();
    }

    */
}


#endif
