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

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/randomdefaultlatentmodel.hpp>
#include <ql/experimental/credit/spotlosslatentmodel.hpp> 
#include <ql/experimental/math/gaussiancopulapolicy.hpp>
#include <ql/experimental/math/latentmodel.hpp>
#include <ql/experimental/math/tcopulapolicy.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <cmath>

namespace QuantLib {


    template<class , class > class RandomLossLM;
    template<class copulaPolicy, class USNG>
        struct simEvent<RandomLossLM<copulaPolicy, USNG> > {
            simEvent(unsigned int n, unsigned int d, Real r) 
            : nameIdx(n), dayFromRef(d), 
                // truncates the value:
              compactRR(std::lround(r/rrGranular)) {}
            unsigned int nameIdx : 12; // can index up to 4095 names
            unsigned int dayFromRef : 12; // can index up to 4095 days = 11 yrs
        private:
            unsigned int compactRR : 8;
        public:
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

#ifndef __DOXYGEN__

    template <class C, class G> const Real 
        simEvent<RandomLossLM<C, G> >::rrGranular = 1./256.;// 2^8

#endif

    /*! Random spot recovery rate loss model simulation for an arbitrary copula.
    */
    template<class copulaPolicy, class USNG = SobolRsg>
    class RandomLossLM : public RandomLM<RandomLossLM, copulaPolicy, USNG>
    {
    private:
        typedef simEvent<RandomLossLM> defaultSimEvent;

        const std::shared_ptr<SpotRecoveryLatentModel<copulaPolicy> > copula_;
        // for time inversion:
        Real accuracy_;
    public:
        explicit RandomLossLM(
            const std::shared_ptr<SpotRecoveryLatentModel<copulaPolicy> >& 
                copula,
            Size nSims = 0,
            Real accuracy = 1.e-6, 
            BigNatural seed = 2863311530UL)
        : RandomLM< ::QuantLib::RandomLossLM, copulaPolicy, USNG>
            (copula->numFactors(), copula->size(), copula->copula(), 
                nSims, seed),
          copula_(copula), accuracy_(accuracy)
    {
        // redundant through basket?
        this->registerWith(Settings::instance().evaluationDate());
    }

        // grant access to static polymorphism:
        /* While this works on g++, VC9 refuses to compile it.
        Not completely sure whos right; individually making friends of the 
        calling members or writting explicitly the derived class T parameters 
        throws the same errors.
        The access is then open to the member fucntions.
        */
        friend class RandomLM< ::QuantLib::RandomLossLM, copulaPolicy, USNG>;
    protected:
        void nextSample(const std::vector<Real>& values) const;

        // see note on randomdefaultlatentmodel
        void initDates() const {
            /* Precalculate horizon time default probabilities (used to 
              determine if the default took place and subsequently compute its 
              event time)
            */
            Date today = Settings::instance().evaluationDate();
            Date maxHorizonDate = today  + Period(this->maxHorizon_, Days);

            const std::shared_ptr<Pool>& pool = this->basket_->pool();
            for(Size iName=0; iName < this->basket_->size(); ++iName)//use'live'
                horizonDefaultPs_.push_back(pool->get(pool->names()[iName]).
                    defaultProbability(this->basket_->defaultKeys()[iName])
                        ->defaultProbability(maxHorizonDate, true));
        }
       Real getEventRecovery(const defaultSimEvent& evt) const {
            return evt.recovery();
        }

        Real latentVarValue(const std::vector<Real>& factorsSample, 
            Size iVar) const {
                return copula_->latentVarValue(factorsSample, iVar);
        }
        Size basketSize() const { return this->basket_->size(); }
        // conditional to default, defined as spot-recovery.
        Real conditionalRecovery(Real latentVarSample, Size iName, 
            const Date& d) const;
    private:
      void resetModel() override {
          /* Explore: might save recalculation if the basket is the same
          (some situations, like BC or control variates) in that case do not
          update, only reset the copula's basket.
          */
          copula_->resetBasket(this->basket_.currentLink());

          QL_REQUIRE(2 * this->basket_->size() == copula_->size(),
                     "Incompatible basket and model sizes.");
          // invalidate current calculations if any and notify observers
          // NOLINTNEXTLINE(bugprone-parent-virtual-call)
          LazyObject::update();
      }
        // Default probabilities for each name at the time of the maximun 
        //   horizon date. Cached for perf.
        mutable std::vector<Probability> horizonDefaultPs_;
    };


    // --------------------------------------------------------------


    template<class C, class URNG>
    void RandomLossLM<C, URNG>::nextSample(
        const std::vector<Real>& values) const 
    {
        const std::shared_ptr<Pool>& pool = this->basket_->pool();
        this->simsBuffer_.push_back(std::vector<defaultSimEvent> ());

        // half the model is defaults, the other half are RRs...
        for(Size iName=0; iName<copula_->size()/2; iName++) {
            // ...but samples must be full
            /* This is really a trick, we are passing a longer than
            expected set of values in the sample but the last idiosyncratic
            values corresponding to the RR are not used. They are used below
            only if we are in default. This works due to the way the SpotLossLM
            is split in two almost disjoint latent models and that theres no
            check on the vector size in the LM base class.
            */
            Real latentVarSample = 
                copula_->latentVarValue(values, iName);
            Probability simDefaultProb = 
                copula_->cumulativeY(latentVarSample, iName);
            // If the default simulated lies before the max date:
            if (horizonDefaultPs_[iName] >= simDefaultProb) {
                const Handle<DefaultProbabilityTermStructure>& dfts = 
                    pool->get(pool->names()[iName]).  // use 'live' names
                    defaultProbability(this->basket_->defaultKeys()[iName]);
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
                // Determine the realized recovery rate:
                /* For this; 'conditionalRecovery' needs to compute the pdef on 
                the realized def event date from the simulation. Yet, this might
                have fallen between todays date and the default TS reference 
                date(usually a two day gap) To avoid requesting a negative time
                probability the date is moved to the TS date 
                Unless the gap is ridiculous this has no practical effect for 
                the RR value*/
                Date today = Settings::instance().evaluationDate();
                Date eventDate = today+Period(static_cast<Integer>(dateSTride), 
                    Days);
                if(eventDate<dfts->referenceDate()) 
                    eventDate = dfts->referenceDate();
                Real latentRRVarSample = 
                    copula_->latentRRVarValue(values, iName);
                Real recovery = 
                    copula_->conditionalRecovery(latentRRVarSample,
                        iName, eventDate);
                this->simsBuffer_.back().push_back(
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


    // Common uses: Not valid in multithread version.
    // ---------- Gaussian default generators options ------------------------
    /* Uses copula direct normal inversion and MT generator 
    typedef RandomLossLM<GaussianCopulaPolicy,
        RandomSequenceGenerator<MersenneTwisterUniformRng> >
            GaussianRandomLossLM;
    */
    /* Uses BoxMuller for gaussian generation, bypassing copula inversions
    typedef RandomLossLM<GaussianCopulaPolicy, RandomSequenceGenerator<
        BoxMullerGaussianRng<MersenneTwisterUniformRng> > >
            GaussianRandomLossLM;
    */
    /* Default case, uses the copula inversion directly and sobol sequence */
    typedef RandomLossLM<GaussianCopulaPolicy> GaussianRandomLossLM;

    // ---------- T default generators options ----------------------------
    /*
    typedef RandomLossLM<TCopulaPolicy, 
      RandomSequenceGenerator<MersenneTwisterUniformRng> > TRandomLossLM;
    */
    /*
    typedef RandomLossLM<TCopulaPolicy, 
        RandomSequenceGenerator<PolarStudentTRng<MersenneTwisterUniformRng> > >
            TRandomLossLM;
    */
    typedef RandomLossLM<TCopulaPolicy> TRandomLossLM;

}

#endif
