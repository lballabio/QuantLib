/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mcpricer.hpp
    \brief base class for Monte Carlo pricers
*/

#ifndef quantlib_montecarlo_pricer_hpp
#define quantlib_montecarlo_pricer_hpp

#include <ql/methods/montecarlo/montecarlomodel.hpp>

namespace QuantLib {

    //! base class for Monte Carlo pricers
    /*! Eventually this class might be linked to the general tree of
        pricers, in order to have tools like impliedVolatility
        available.  Also, it could, eventually, offer greeks methods.
        Deriving a class from McPricer gives an easy way to write a
        Monte Carlo Pricer.  See McEuropean as example of one factor
        pricer, Basket as example of multi factor pricer.
    */
    template <template <class> class MC, class RNG, class S = Statistics>
    class McPricer {
      public:
        virtual ~McPricer() {}
        //! add samples until the required tolerance is reached
        Real value(Real tolerance,
                   Size maxSamples = QL_MAX_INTEGER,
                   Size minSamples = 1023) const;
        //! simulate a fixed number of samples
        Real valueWithSamples(Size samples,
                              Size minSamples = 1023) const;
        //! estimated error of the samples simulated so far
        Real errorEstimate() const;
        //! access to the sample accumulator for more statistics
        const S& sampleAccumulator(void) const;
      protected:
        McPricer() {}
        mutable boost::shared_ptr<MonteCarloModel<MC,RNG,S> > mcModel_;
    };


    // inline definitions
    template <template <class> class MC, class RNG, class S>
    inline Real McPricer<MC,RNG,S>::value(Real tolerance,
                                          Size maxSamples,
                                          Size minSamples) const {

        Size sampleNumber =
            mcModel_->sampleAccumulator().samples();
        if (sampleNumber<minSamples) {
            mcModel_->addSamples(minSamples-sampleNumber);
            sampleNumber = mcModel_->sampleAccumulator().samples();
        }

        Size nextBatch;
        Real order;
        Real result = mcModel_->sampleAccumulator().mean();
        Real accuracy = mcModel_->sampleAccumulator().errorEstimate()/result;
        while (accuracy > tolerance) {
            // conservative estimate of how many samples are needed
            order = accuracy*accuracy/tolerance/tolerance;

            nextBatch =
                Size(std::max<Real>(sampleNumber*order*0.8-sampleNumber,
                                    minSamples));
            // do not exceed maxSamples
            nextBatch = std::min(nextBatch, maxSamples-sampleNumber);
            QL_REQUIRE(nextBatch>0,
                       "max number of samples exceeded");

            sampleNumber += nextBatch;
            mcModel_->addSamples(nextBatch);
            result = mcModel_->sampleAccumulator().mean();
            accuracy = mcModel_->sampleAccumulator().errorEstimate()/
                result;
        }

        return result;
    }


    template <template <class> class MC, class RNG, class S>
    inline Real McPricer<MC,RNG,S>::valueWithSamples(Size samples,
                                                     Size minSamples) const {

        QL_REQUIRE(samples>=minSamples,
                   "number of requested samples (" << samples
                   << ") lower than minSamples (" << minSamples << ")");

        Size sampleNumber =
            mcModel_->sampleAccumulator().samples();

        QL_REQUIRE(samples>=sampleNumber,
                   "number of already simulated samples (" << sampleNumber
                   << ") greater than requested samples (" << samples << ")");

        mcModel_->addSamples(samples-sampleNumber);

        return mcModel_->sampleAccumulator().mean();
    }


    template <template <class> class MC, class RNG, class S>
    inline Real McPricer<MC,RNG,S>::errorEstimate() const {
        return mcModel_->sampleAccumulator().errorEstimate();
    }

    template <template <class> class MC, class RNG, class S>
    inline const S& McPricer<MC,RNG,S>::sampleAccumulator() const {
        return mcModel_->sampleAccumulator();
    }

}


#endif
