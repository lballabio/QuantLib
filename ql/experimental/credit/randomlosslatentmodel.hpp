/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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
#ifndef quantlib_randomloss_latent_model_hpp
#define quantlib_randomloss_latent_model_hpp

#include <ql/math/solvers1d/brent.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/math/latentmodel.hpp>
#include <ql/experimental/math/gaussiancopulapolicy.hpp>
#include <ql/experimental/math/tcopulapolicy.hpp>

#include <ql/experimental/credit/randomdefaultlatentmodel.hpp>
#include <ql/experimental/credit/spotlosslatentmodel.hpp> 

// needed for specializations:
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    /*! Random spot recovery rate loss model simulation for an arbitrary copula.
    */
    template<class copulaPolicy, class USNG = SobolRsg>
    class RandomLossLM : public RandomLM<RandomLossLM<copulaPolicy, USNG>, 
        copulaPolicy, USNG>
    {
      // grant access to static polymorphism:
      friend class RandomLM<RandomLossLM<copulaPolicy, USNG>, 
          copulaPolicy, USNG>;
      typedef simEvent<RandomLossLM<copulaPolicy, USNG> > defaultSimEvent;
    private:
        const SpotRecoveryLatentModel<copulaPolicy> copula_;
       // for time inversion:
        Real accuracy_;
    public:
        RandomLossLM(
            const SpotRecoveryLatentModel<copulaPolicy>& copula,
            Size nSims = 0,
            Real accuracy = 1.e-6, 
            BigNatural seed = 2863311530);
    protected:
        void nextSample(const std::vector<Real>& values) const;

        Real latentVarValue(const std::vector<Real>& factorsSample, 
            Size iVar) const {
                return copula_.latentVarValue(factorsSample, iVar);
        }
        Size basketSize() const { return basket_->size(); }
        // conditional to default, defined as spot-recovery.
        Real conditionalRecovery(Real latentVarSample, Size iName, 
            const Date& d) const;
    public:
        //statistics:
        Real expectedTrancheLoss(const Date& d) const;
        std::pair<Real, Real> expectedTrancheLossInterval(const Date& d, 
            Probability confidencePerc) const;
    private:
        void resetModel() /*const*/ {
            /* Explore: might save recalculation if the basket is the same 
            (some situations, like BC or control variates) in that case do not 
            update, only reset the copula's basket.
            */
            copula_.resetBasket(basket_.currentLink());

            QL_REQUIRE(2 * basket_->size() == copula_.size(),
                "Incompatible basket and model sizes.");
            // invalidate current calculations if any and notify observers
            LazyObject::update();
        }
        // see not on randomdefaultlatentmodel
        void initDates() const {
            /* Precalculate horizon time default probabilities (used to 
              determine if the default took place and subsequently compute its 
              event time)
            */
            Date today = Settings::instance().evaluationDate();
            Date maxHorizonDate = today  + Period(maxHorizon_, Days);

            const boost::shared_ptr<Pool>& pool = basket_->pool();
            for(Size iName=0; iName < basket_->size(); ++iName)//use'live'basket
                horizonDefaultPs_.push_back(pool->get(pool->names()[iName]).
                    defaultProbability(basket_->defaultKeys()[iName])
                        ->defaultProbability(maxHorizonDate, true));
        }
        // Default probabilities for each name at the time of the maximun 
        //   horizon date. Cached for perf.
        mutable std::vector<Probability> horizonDefaultPs_;
    };

    template<class copulaPolicy, class USNG>
        struct simEvent<RandomLossLM<copulaPolicy, USNG> > {
            simEvent(unsigned int n, unsigned int d, Real r) 
            : nameIdx(n), dayFromRef(d), 
                // truncates the value:
                compactRR(r/rrGranular+.5) {}
            unsigned int nameIdx : 12; // can index up to 4095 names
            unsigned int dayFromRef : 12; // can index up to 4095 days = 11 yrs
            unsigned int compactRR : 8;//RRPrecission 7;  // SHOULD BE PRIVATE
            // ..............still one bit left
            bool operator<(const simEvent& evt) const {
                return dayFromRef < evt.dayFromRef; 
            }
            Real recovery() const {
                /* we pay the price of this product (plus the division at 
                construction) for the memory we save. Precission is lost though,
                e.g. figures from 0.0 to 0.00390625/2. are stored as 0.0
                */
                return rrGranular * compactRR;
            }
            static const Real rrGranular;// = 1./256.;// 2^8
    };
    template <class C, class G> const Real 
        simEvent<RandomLossLM<C, G> >::rrGranular = 1./256.;// 2^8




    template<class C, class URNG>
    void RandomLossLM<C, URNG>::nextSample(
        const std::vector<Real>& values) const 
    {
        const boost::shared_ptr<Pool>& pool = basket_->pool();
        simsBuffer_.push_back(std::vector<defaultSimEvent> ());

        // half the model is defaults, the other half are RRs...
        for(Size iName=0; iName<copula_.size()/2; iName++) {
            // ...but samples must be full
            /* This is really a trick, we are passing a longer than
            expected set of values in the sample but the last idiosyncratic
            values corresponding to the RR are not used. They are used below
            only if we are in default. This works due to the way the SpotLossLM
            is split in two almost disjoint latent models and that theres no
            check on the vector size in the LM base class.
            */
            Real latentVarSample = 
                copula_.latentVarValue(values, iName);
            Probability simDefaultProb = 
                copula_.cumulativeY(latentVarSample, iName);
            // If the default simulated lies before the max date:
            if (horizonDefaultPs_[iName] >= simDefaultProb) {
                const Handle<DefaultProbabilityTermStructure>& dfts = 
                    pool->get(pool->names()[iName]).  // use 'live' names
                    defaultProbability(basket_->defaultKeys()[iName]);
                // compute and store default time with respect to the 
                //  curve ref date:
                Size dateSTride =
                    static_cast<Size>(Brent().solve(// casted from Real:
                    detail::Root(dfts, simDefaultProb), accuracy_, 0., 1.));
               /*
               // value if one approximates to a flat HR; 
               //   faster (>x2) but it introduces an error:..
               // \todo: see how to include this 'polymorphically'. While
               //   not the case in pricing in risk metrics/real  
               //   probabilities the curves are often flat
                static_cast<Size>(ceil(maxHorizon_ * 
                                    std::log(1.-simDefaultProb)
                /std::log(1.-data_.horizonDefaultPs_[iName])));
               */
               // Determine the realized recovery rate.

               // theres a two day gap between todays and the default TS ref 
               // date that triggers a negative date when requesting the probability to the TS
                ////      Date today = Settings::instance().evaluationDate();
                Date today = dfts->referenceDate();/// NO GOOD, NOW DATES MEAN DIFFERENT THINGS!!!!!!!!!!!!!!!!!!!! NEED FIXING!!!

                Real latentRRVarSample = copula_.latentRRVarValue(values, iName);
                Real recovery = 
                    copula_.conditionalRecovery(latentRRVarSample, iName, 
                        today+Period(static_cast<Integer>(dateSTride), Days));
                simsBuffer_.back().push_back(
                   defaultSimEvent(iName, dateSTride, recovery));
                //emplace_back
            }
        /* Used to remove sims with no events. Uses less memory, faster 
        post-statistics. But only if all names in the portfolio have low 
        default probability, otherwise is more expensive and sim access has 
        to be modified. However low probability is also an indicator that 
        variance reduction is needed. */
        //if(simsBuffer.back().empty()) {
        //    emptySims_++;// Size; intilzd to zero
        //    simsBuffer.pop_back();
        //}
        }
    }


    template<class C, class URNG>
    RandomLossLM<C, URNG>::RandomLossLM(
        const SpotRecoveryLatentModel<C>& copula,
        Size nSims,
        Real accuracy, 
        BigNatural seed) 
    : ///////////////////////////basket_(basket), 
      accuracy_(accuracy), 
      copula_(copula), //<<-------------------------------------------------------------------------------CHECK THIS COPY
      RandomLM(copula.numFactors(), copula.size(), copula.copula(), 
          nSims, seed)
    {
        // redundant through basket?
        registerWith(Settings::instance().evaluationDate());
    }

    template<class C, class URNG>
    Real RandomLossLM<C, URNG>::expectedTrancheLoss(
        const Date& d) const {
            return expectedTrancheLossInterval(d, 0.95).first;
    }

    template<class C, class URNG>// return disposable...
    std::pair<Real, Real> RandomLossLM<C, URNG>::expectedTrancheLossInterval(
        const Date& d, Probability confidencePerc) const 
    {
        calculate();
        Date today = Settings::instance().evaluationDate();
        BigInteger val = d.serialNumber() - today.serialNumber();

        Real trancheLoss= 0.;
        GeneralStatistics lossStats;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<defaultSimEvent>& events = getSim(iSim);

            Real portfSimLoss=0.;
            for(Size iEvt=0; iEvt <  events.size(); iEvt++) {
                // if event is within time horizon...
                if(val > events[iEvt].dayFromRef) {
                    Size iName = events[iEvt].nameIdx;
                    // ...and is contained in the basket.
                        portfSimLoss += 
                            basket_->exposure(basket_->names()[iName], 
                                Date(events[iEvt].dayFromRef + 
                                    today.serialNumber())) *
    //                        (1.-simsBuffer_[iSim][iEvt].recovery / 100.);
                            (1.-events[iEvt].recovery());
               }
            }
            lossStats.add(// dates? current losses? realized defaults, not yet
              std::min(std::max(portfSimLoss - basket_->attachmentAmount(), 0.),
                basket_->detachmentAmount() - basket_->attachmentAmount()) );
        }
        return std::make_pair(lossStats.mean(), lossStats.errorEstimate() * 
            InverseCumulativeNormal::standard_value(0.5*(1.+confidencePerc)));
    }






    // Common uses: Not valid in multithread version.
    typedef RandomLossLM<GaussianCopulaPolicy, 
        RandomSequenceGenerator<BoxMullerGaussianRng<
            MersenneTwisterUniformRng> > > GaussianRandomLossLM;
    typedef RandomLossLM<TCopulaPolicy, 
        RandomSequenceGenerator<PolarStudentTRng<MersenneTwisterUniformRng> > > 
            TRandomLossLM;


}

#endif
