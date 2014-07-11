/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2009, 2014 Jose Aparicio

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
#ifndef quantlib_randomdefault_latent_model_hpp
#define quantlib_randomdefault_latent_model_hpp

#include <ql/math/solvers1d/brent.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/experimental/math/latentmodel.hpp>
#include <ql/experimental/math/gaussiancopulapolicy.hpp>
#include <ql/experimental/math/tcopulapolicy.hpp>

// needed for specializations:
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

/* Intended to replace 
    ql\experimental\credit\randomdefaultmodel.Xpp
*/

namespace QuantLib {

    /*! Simulation event trait class template. Each latent model will be 
    modelling different entities according to the meaning of the model function
    which depends on the random \$ Y_i\$ variables. Still the generation of the 
    factors and variables it is common to any model. Only within a given 
    transformation function the model and event generated becomes concrete. 

    However here these simulations are already made specific to a default event.
    Yet other variables contingent to default can be modelled (recovery, 
    market...) So we are placed in a less generic stage where default is 
    modelled possibly jointly with other unespecified magnitudes. 

    Another role of this trait class is to compact in memory the simulation 
    data. The statistic post processing needs to have the results stored in 
    memory and simulations can not be consumed at generation time, typically 
    because some statistics are conditional on others (e.g. ESF) or/and 
    parametric (percentile, etc...)

    Simulation events do not derive from each other, and they are specialized
    for each type; duck typing applies for variable names (see the statistic 
    methods)
    */
    template <typename simEventOwner> struct simEvent {
    private:
        simEvent(){}//fail if no spez
    };
    /* Shouldnt it be?:
    template <template <class, class> class simEventOwner> struct simEvent {};
    */






    /*! Base class for latent model monte carlo simulation. Independent of the 
    copula type and the generator. 
    Generates the factors and variable samples and determines event threshold 
    but it is not responsible for actual event specification; thats the derived 
    classes responsibility according to what they model.
    Derived classes need mainly to implement nextSample (Worker::nextSample in 
    the multithreaded version) to compute the simulation event generated, if 
    any, from the latent variables sample. They also have the accompanying 
    event trait to specify.
    */
    /* CRTP used for performance to avoid virtual table resolution in the Monte 
    Carlo. Not only in sample generation but access; quite an amount of time can
    go into statistics computation, for a portfolio of tens of thousands 
    positions that part of the problem will be starting to overtake the 
    simulation costs.

    \todo: someone with sound experience on cache misses look into this, the 
    statistics will be getting memory in and out of the cpu heavily and it 
    might be possible to get performance out of that.
    \todo: parallelize the statistics computation, things like Var/ESF splits
    are very expensive.
    \todo: consider another design, taking the statistics outside the models.
    */
    template<class derivedRandomLM, class copulaPolicy, class USNG = SobolRsg>
    /* Shouldnt it be?:
    template<template <class, class> class derivedRandomLM,
    */
    class RandomLM : public virtual LazyObject, 
                     public virtual DefaultLossModel {
    private:
        // Takes the parents type, all children have the same type, the 
        // random generation is performed in this class only.
        typedef typename LatentModel<copulaPolicy>::template FactorSampler<USNG>
            copulaRNG_type;
    protected:
        RandomLM(Size numFactors, 
            Size numLMVars, 
            const copulaPolicy& copula,
            Size nSims, 
            BigNatural seed)
        : numFactors_(numFactors), numLMVars_(numLMVars), copula_(copula), 
          nSims_(nSims), seed_(seed) {}

        void update() {
            LazyObject::update();
        }

        void performCalculations() const {
            static_cast<const derivedRandomLM* >(this)->initDates();//in update?
            USNG baseUrng(numFactors_+ numLMVars_, seed_); 
            copulasRng_ = 
                boost::make_shared<copulaRNG_type>(baseUrng, copula_);
            performSimulations();
        }

        void performSimulations() const {
            // Next sequence should determine the event and push it into buffer
            for(Size i=nSims_; i; i--) {
                const std::vector<Real>& sample = 
                    copulasRng_->nextSequence().value;
                static_cast<const derivedRandomLM* >(this)->nextSample(sample);
            // alternatively make call an explicit local method...
            }
        }

        /* Method to access simulation results and avoiding a copy of  
        each thread results buffer. PerformCalculations should have been called.
        Here in the monothread version this method is redundant/trivial but 
        serves to detach the statistics access to the way the simulations are
        stored.
        */
        const std::vector<simEvent<derivedRandomLM> >& getSim(
            const Size iSim) const { return simsBuffer_[iSim]; }

        //! \name Statistics, DefaultLossModel interface.
        // These are virtual and allow for children-specific optimization and 
        //   variance reduction. The virtual table is ok, they are not part
        //   of the simulation.
        //@{
        /*! Returns the probaility of having a given or larger number of 
        defaults in the basket portfolio at a given time.
        */
        virtual Probability probAtLeastNEvents(Size n, const Date& d) const;
        /*! Order of results refers to the simulated (super)pool not the 
        basket's pool.*/
        virtual Disposable<std::vector<Probability> > probsBeingNthEvent(Size n,
            const Date& d) const;
        //! Pearsons' default probability correlation. 
        virtual Real defaultCorrelation(const Date& d, Size iName, 
            Size jName) const;
        //@}
    private:
        BigNatural seed_;
    protected:
        const Size numFactors_;
        const Size numLMVars_;

        const Size nSims_;

        mutable std::vector<std::vector<simEvent<derivedRandomLM> > > 
            simsBuffer_;

        mutable copulaPolicy copula_;
        mutable boost::shared_ptr<copulaRNG_type> copulasRng_;

        // Maximum time inversion horizon
        static const Size maxHorizon_ = 4050; // over 11 years
        /* Inversion probability limits are computed by children in initdates()
        This one to be given some thinking (duplicates code)
        */
    };


    template<class D, class C, class URNG>
    Probability RandomLM<D, C, URNG>::probAtLeastNEvents(Size n, 
        const Date& d) const 
    {
        calculate();
        Date today = Settings::instance().evaluationDate();

        QL_REQUIRE(d>today, "Date for statistic must be in the future.");
        // casted to natural to avoid warning, we have just checked the sign
        Natural val = d.serialNumber() - today.serialNumber();

        if(n==0) return 1.;

        Real counts = 0.;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            Size simCount = 0;
            const std::vector<simEvent<D> >& events = 
                getSim(iSim);
            for(Size iEvt=0; iEvt < events.size(); iEvt++)
                // duck type on the members:
                if(val > events[iEvt].dayFromRef) simCount++;
                if(simCount >= n) counts++;
        }
        return counts/nSims_;
        // \todo Provide confidence interval
    }

    template<class D, class C, class URNG>
    Disposable<std::vector<Probability> > 
        RandomLM<D, C, URNG>::probsBeingNthEvent(Size n, 
            const Date& d) const 
    {
        calculate();
        Size basketSize = basket_->size();

        QL_REQUIRE(n>0 && n<=basketSize, "Impossible number of defaults.");
        Date today = Settings::instance().evaluationDate();

        QL_REQUIRE(d>today, "Date for statistic must be in the future.");
        // casted to natural to avoid warning, we have just checked the sign
        Natural val = d.serialNumber() - today.serialNumber();

		std::vector<Probability> hitsByDate(basketSize, 0.);
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D> >& events = getSim(iSim);
            std::map<unsigned short, unsigned short> namesDefaulting;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                // if event is within time horizon...
                if(val > events[iEvt].dayFromRef)
					//...count it. notice insertion sorts by date.
					namesDefaulting.insert(std::make_pair<unsigned short, 
                      unsigned short>(events[iEvt].dayFromRef, 
                        events[iEvt].nameIdx));
			}
            if(namesDefaulting.size() >= n) {
                std::map<unsigned short, unsigned short>::const_iterator 
                    itdefs = namesDefaulting.begin();
                // locate nth default in time:
                std::advance(itdefs, n-1);
                // update statistic:
                hitsByDate[itdefs->second]++;
            }
        }
        std::transform(hitsByDate.begin(), hitsByDate.end(), 
            hitsByDate.begin(), std::bind2nd(std::divides<Real>(), nSims_));
        return hitsByDate;
        // \todo Provide confidence interval
    }


	template<class D, class C, class URNG>
    Real RandomLM<D, C, URNG>::defaultCorrelation(const Date& d,
        Size iName, Size jName) const 
    {
        // a control variate with the probabilities is possible
        calculate();
        Date today = Settings::instance().evaluationDate();

        QL_REQUIRE(d>today, "Date for statistic must be in the future.");
        // casted to natural to avoid warning, we have just checked the sign
        Natural val = d.serialNumber() - today.serialNumber();

        Real expectedDefiDefj = 0.;// E[1_i 1_j]
        // the rest of magnitudes have known values (probabilities) but that 
        //   would distort the simulation results.
        Real expectedDefi = 0.;
        Real expectedDefj = 0.;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D> >& events = getSim(iSim);
            Real imatch = 0., jmatch = 0.;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                if((val > events[iEvt].dayFromRef) &&
                   (events[iEvt].nameIdx == iName)) imatch = 1.;
                if((val > events[iEvt].dayFromRef) &&
                   (events[iEvt].nameIdx == jName)) jmatch = 1.;
            }
            expectedDefiDefj += imatch * jmatch;
            expectedDefi += imatch;
            expectedDefj += jmatch;
        }
        expectedDefiDefj = expectedDefiDefj / (nSims_-1);// unbiased
        expectedDefi = expectedDefi / nSims_;
        expectedDefj = expectedDefj / nSims_;

        return (expectedDefiDefj - expectedDefi*expectedDefj) / 
            std::sqrt((expectedDefi*expectedDefj*(1.-expectedDefi)
                *(1.-expectedDefj)));
        // \todo Provide confidence interval
    }








    /*
    ---------------------------------------------------------------------------
    ---------------------------------------------------------------------------
    */

    // move this one to a separte file?

    /*! Default only latent model simulation with trivially fixed recovery 
        amounts.
    */
    template<class copulaPolicy, class USNG = SobolRsg>
    class RandomDefaultLM : 
        public RandomLM<RandomDefaultLM<copulaPolicy, USNG>, copulaPolicy, USNG>
    {
        // grant access to static polymorphism:
        friend class RandomLM<RandomDefaultLM<copulaPolicy, USNG>, 
            copulaPolicy, USNG>;
        typedef simEvent<RandomDefaultLM<copulaPolicy, USNG> > defaultSimEvent;
    private:
        const DefaultLatentModel<copulaPolicy> copula_;
        const std::vector<Real> recoveries_;
        // for time inversion:
        Real accuracy_;
    public:
        // \todo: Allow a constructor with ConstantLossLatentmodel and no 
        //    recoveries... and drop the default value recovery vector...
        // \todo: Allow a constructor building its own default latent model.
        RandomDefaultLM(
            const DefaultLatentModel<copulaPolicy>& copula,//////////////////////////// change to pointer!
            const std::vector<Real>& recoveries = std::vector<Real>(),
            Size nSims = 0,// stats will crash on div by zero, FIX ME.
            Real accuracy = 1.e-6, 
            BigNatural seed = 2863311530);
        //! \name Statistics
        //@{
        virtual Real expectedTrancheLoss(const Date& d) const;
        //@}
    protected:
        void nextSample(const std::vector<Real>& values) const;

        Real latentVarValue(const std::vector<Real>& factorsSample, 
            Size iVar) const {
            return copula_.latentVarValue(factorsSample, iVar);
        }
        //allows statistics to know the portfolio size (could be moved to parent 
        //invoking duck typing on the variable name or a handle to the basket)
        Size basketSize() const { return copula_.size(); }
    private:
        void resetModel() /*const*/ {
            /* Explore: might save recalculation if the basket is the same 
            (some situations, like BC or control variates) in that case do not 
            update, only reset the copula's basket.
            */
            copula_.resetBasket(basket_.currentLink());

            QL_REQUIRE(basket_->size() == copula_.size(), 
                "Incompatible basket and model sizes.");
            QL_REQUIRE(recoveries_.size() == basket_->size(), 
                "Incompatible basket and recovery sizes.");
            // invalidate current calculations if any and notify observers
            LazyObject::update();
        }
        // This one and the buffer might be moved to the parent, only some 
        //   dates might be specific to a particular model.
        void initDates() const {
            /* Precalculate horizon time default probabilities (used to 
              determine if the default took place and subsequently compute its 
              event time)
            */
            Date today = Settings::instance().evaluationDate();
            Date maxHorizonDate = today  + Period(maxHorizon_, Days);

            const boost::shared_ptr<Pool>& pool = copula_.basket_->pool();
            for(Size iName=0; iName < copula_.size(); ++iName)//use'live'basket
                horizonDefaultPs_.push_back(pool->get(pool->names()[iName]).
                    defaultProbability(copula_.basket_->defaultKeys()[iName])
                        ->defaultProbability(maxHorizonDate, true));
        }

        // Default probabilities for each name at the time of the maximun 
        //   horizon date. Cached for perf.
        mutable std::vector<Probability> horizonDefaultPs_;
    };

    /*! Random default with deterministic recovery event type.\par
    Stores sims results in a bitfield buffer for lean memory storage.
    Although strictly speaking this is not guaranteed by the compiler it
    amounts to reducing the memory storage by half.
    Some computations, like conditional statistics, precise that all sims
    results be available. 
    */
    template<class copulaPolicy, class USNG>
    struct simEvent<RandomDefaultLM<copulaPolicy, USNG> > {
        simEvent(unsigned int n, unsigned int d) 
        : nameIdx(n), dayFromRef(d){}
        unsigned int nameIdx : 16; // can index up to 65535 names
        unsigned int dayFromRef : 16; //indexes up to 65535 days ~179 years
        bool operator<(const simEvent& evt) const {
            return dayFromRef < evt.dayFromRef; 
        }
    };





    template<class C, class URNG>
    void RandomDefaultLM<C, URNG>::nextSample(
        const std::vector<Real>& values) const 
    {
        const boost::shared_ptr<Pool>& pool = copula_.basket_->pool();
        // starts with no events
        simsBuffer_.push_back(std::vector<defaultSimEvent> ());

        for(Size iName=0; iName<copula_.size(); iName++) {
            Real latentVarSample = 
                copula_.latentVarValue(values, iName);
            Probability simDefaultProb = 
               copula_.cumulativeY(latentVarSample, iName);
            // If the default simulated lies before the max date:
            if (horizonDefaultPs_[iName] >= simDefaultProb) {
                const Handle<DefaultProbabilityTermStructure>& dfts = 
                    pool->get(pool->names()[iName]).// use 'live' names
                    defaultProbability(copula_.basket_->defaultKeys()[iName]);
                // compute and store default time with respect to the 
                //  curve ref date:
                Size dateSTride =
                    static_cast<Size>(Brent().solve(// casted from Real:
                        detail::Root(dfts, simDefaultProb),
                            accuracy_,0.,1.));
                   /*
                   // value if one approximates to a flat HR; 
                   //   faster (>x2) but it introduces an error:..
                   // \todo: see how to include this 'polymorphically'. 
                   // While not the case in pricing in risk metrics/real
                   //   probabilities the curves are often flat
                    static_cast<Size>(ceil(maxHorizon_ * 
                                        std::log(1.-simDefaultProb)
                    /std::log(1.-data_.horizonDefaultPs_[iName])));
                   */
                simsBuffer_.back().push_back(defaultSimEvent(iName, 
                    dateSTride));
               //emplace_back
            }
        /* Used to remove sims with no events. Uses less memory, faster 
        post-statistics. But only if all names in the portfolio have low 
        default probability, otherwise is more expensive and sim access has 
        to be modified. However low probability is also an indicator that 
        variance reduction is needed. */
        }
    }
    



    template<class C, class R> 
    RandomDefaultLM<C, R>::RandomDefaultLM(
        const DefaultLatentModel<C>& copula,
        const std::vector<Real>& recoveries,
        Size nSims,
        Real accuracy, 
        BigNatural seed) 
      : copula_(copula), //<- renmae to latentModel_ or defautlLM_; why the copy? <<<<<<<<<<<<<<<<<
        accuracy_(accuracy),
        // set to 0 RR if empty, RRs to be ignored
        recoveries_(recoveries.size()==0 ? std::vector<Real>(copula.size(), 0.)
            : recoveries),
        RandomLM(copula.numFactors(), copula.size(), copula.copula(), 
            nSims, seed )
    {   
        // redundant through basket?
        registerWith(Settings::instance().evaluationDate());

    }

    template<class C, class URNG>
    Real RandomDefaultLM<C, URNG>::expectedTrancheLoss(const Date& d) const {
        calculate();

        Real attachAmount = basket_->remainingAttachmentAmount();
        Real detachAmount = basket_->remainingDetachmentAmount();

        BigInteger val = d.serialNumber() - 
            Settings::instance().evaluationDate().value().serialNumber();
        // for each sim collect defaults before date d and add the losses. 
        //   return the average.
        Real trancheLoss= 0.;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<defaultSimEvent>& events = getSim(iSim);
            Real portfSimLoss=0.;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                // if event is within time horizon...
                if(val > events[iEvt].dayFromRef) {
                    Size iName = events[iEvt].nameIdx;
                    // ...and is contained in the basket.
                  //superpool simulations not yet
                  // if(basket_->pool()->has(copula_->pool()->names()[iName]))
                        portfSimLoss += basket_->remainingNotionals()[iName] * // NEED LOCAL NOTIONALS... updatable on basket reasignment
                            (1.-recoveries_[iName]);//recovery index only works if the RR vector is of live-names size.
                }
            }
            trancheLoss += std::min(std::max(portfSimLoss - attachAmount, 0.), detachAmount - attachAmount);
        }

        return trancheLoss / nSims_;
    }





    // Common usage typedefs (notice they vary in the multithread version)
    // For the multithread implementation these souldnt be used, fortunately 
    //   they will fail because skipTo is absent.
    typedef RandomDefaultLM<GaussianCopulaPolicy, 
        RandomSequenceGenerator<BoxMullerGaussianRng<
            MersenneTwisterUniformRng> > > GaussianMTRandomDefaultLM;
    // This one uses the copula inversion directly:
    /* typedef RandomDefaultLM<GaussianCopulaPolicy, MersenneTwisterUniformRng>
    GaussianMTRandomDefaultLM;*/
    /*  typedef RandomDefaultLM<TCopulaPolicy, 
      RandomSequenceGenerator<MersenneTwisterUniformRng> > TMTRandomDefaultLM;*/
    typedef RandomDefaultLM<TCopulaPolicy, 
        RandomSequenceGenerator<PolarStudentTRng<MersenneTwisterUniformRng> > > 
            TMTRandomDefaultLM;






    /* It could be argued that this concept is part of the copula (more generic).
    In general when the modelled magnitude is parametric one can solve for 
    inversion to get the parameter value for a given magnitude value (provided 
    the modelled variable dependence in invertible). In this particular problem 
    the parameter is Time and it is solved here where we are alredy in the 
    context of default 
    See default transition models for another instance of this inversion.
    Alternatively use the faster trick (flat HR) mentioned in the code or make
    the algorithm parametric on the type of interpolation in the DefautlTS
    */
    namespace detail {// not template dependent .....move it
        //! Utility for the numerical time solver
        class Root {
          public:
            /* SEE A FASTER ALGO (neeeds to locate the points) ON 
            D.O'KANE p.249 sect 13.5 */
            Root(const Handle<DefaultProbabilityTermStructure> dts, Real pd)
                : dts_(dts), pd_(pd), curveRef_(dts->referenceDate()) {}
            /* The cast I am forcing here comes from the requirement of 1D 
            solvers to take in a target (cost) function of Real domain. It could
            be possible to change the template arg F in the 1D solvers to a 
            boost function and then use the (template arg) domain argument type 
            of the function for use with the 'guess' and operator() ?
             */
            Real operator()(Real t) const {
                QL_REQUIRE (t >= 0.0, "t < 0");
                /* As long as this doesnt involve modifying a mutable member
                it should be thread safe (they are const methods and access is 
                read only)
                */
                return dts_->defaultProbability(curveRef_ + 
                    Period(static_cast<Integer>(t), Days), true) - pd_;
            }
          private:
            const Handle<DefaultProbabilityTermStructure> dts_;
            Real pd_;
            const Date curveRef_;
        };
    }

}

#endif
