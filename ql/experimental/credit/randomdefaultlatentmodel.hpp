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

#include <boost/thread/thread.hpp>

#include <ql/math/solvers1d/brent.hpp>
#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/math/latentmodel.hpp>
#include <ql/experimental/math/gaussiancopulapolicy.hpp>

// needed for specializations:
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>

namespace QuantLib {

    /* Multithreaded, but the single thread version can be easily rewritten.

       For specific copula random generation algorithms 'peformCalculations'
       need to be specialized on the copula policy type.
    */
    // Intended to replace RandomDefaultModel 
    template<class copulaPolicy, class USNG = SobolRsg>
    class RandomDefaultLM : public LatentModel<copulaPolicy>,
                            public LazyObject
    {
    private:
        typedef typename LatentModel<copulaPolicy>::FactorSampler<USNG> 
            copulaRNG_type;
        /*! Stores sims results in a bitfield buffer for lean memory storage.
        Although strictly speaking this is not guaranteed by the compiler it
        amounts to reducing the memory storage by half.
        Some computations, like conditional statistics, precise that all sims
        results be available. 
        */
        struct simEvent {// not template dependent .....move it
            simEvent(unsigned int n, unsigned int d) 
            : nameIdx(n), dayFromRef(d){}
            unsigned int nameIdx : 16; // can index up to 65535 names
            unsigned int dayFromRef : 16; //indexes up to 65535 days ~179 years
            bool operator<(const simEvent& evt) const {
                return dayFromRef < evt.dayFromRef; 
            }
        };
        /* The mechanics of the MC can be generalized into the LM base class by
        declaring a sampleEvent trait and making the worker refer to the base
        LM, only the part of the worker that turns a latent variable sample
        into an specific event (or realization of the modelled magnitude) is to 
        be defined on each specific case.
        */
        friend class Worker;
        class Worker {
        public:
            /*! @param numSims This thread simulations number.
                @param rsg Must be already positioned and done coherently with 
                  the number of simulations in this and other threads.
            */
            Worker(Size numSims, const copulaRNG_type& copulasRng,
                const RandomDefaultLM& caller
                ) 
            : numSims_(numSims),
              copulasRng_(copulasRng),
              data_(caller) {}
            void performSimulations() {
                for(Size i=numSims_; i; i--) nextSequence();
            }
            Size assignedLoad() const { return numSims_;}
        private:
            void nextSequence();
        public:
            std::vector<std::vector<simEvent> > simsBuffer;
        private:
            const Size numSims_;
            copulaRNG_type copulasRng_;
            const RandomDefaultLM& data_;
        };

    public:
        RandomDefaultLM(
            const boost::shared_ptr<Basket>& basket,
            const std::vector<std::vector<Real> >& factorWeights,
            const typename copulaPolicy::initTraits& ini = 
                copulaPolicy::initTraits(),
            Size nSims = 0,
            Size numThreads = 1,
            Real accuracy = 1.e-6, 
            BigNatural seed = 2863311530
            );

        void performCalculations() const;
        /*! Returns the probaility of having a given or larger number of 
        defaults in the basket portfolio at a given time.
        */
        //! \name Statistics
        //@{
        Probability probAtLeastNEvents(Size n, const Date& d) const;
        /*! Order of results refers to the simulated (super)pool not the 
        basket's pool.*/
        Disposable<std::vector<Probability> >  
            probsBeingNthEvent(Size n, const Date& d) const;
        //! Pearsons' default probability correlation. 
        Real defaultCorrelation(const Date& d, Size iName, Size jName) const;
        //@}
    protected:
        void update() { 
            LazyObject::update(); 
            initDates();
        }
    private:
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
        void performSimulations() const {
            // for thread safety, compute basket:...using the old basket by now
            std::vector<boost::shared_ptr<boost::thread> > thrds;
            for(Size i=0; i<numThreads_; i++)
                thrds.push_back(boost::make_shared<boost::thread>(
                    boost::bind(
                    &Worker::performSimulations, boost::ref(*workers_[i]))));
            std::for_each(thrds.begin(), thrds.end(), 
                boost::bind(&boost::thread::join, _1));
            //boost::thread_group and join_all is another option, though in the 
            // doc they say new is called
        }

        /* Private method to access simulation results and avoiding a copy of  
        each thread results buffer. PerformCalculations should have been called. 
        */
        const std::vector<simEvent>& getSim(const Size iSim) const {
            // no call to calculate, no check on limits; private method
            Size wrkrId = 
                std::min((Size)(iSim / nSimsPerThread_), numThreads_-1);
            return 
                workers_[wrkrId]->simsBuffer[iSim - wrkrId * nSimsPerThread_];
        }
    private:
        Real accuracy_;
        BigNatural seed_;
        const Size nSims_;
        // noncopiable:....
        mutable std::vector<boost::shared_ptr<Worker> > workers_;
        const Size numThreads_;
        const Size nSimsPerThread_;
        const Size nSimsLastThread_;

        boost::shared_ptr<Basket> basket_;

        static const Size maxHorizon_ = 4050; // over 11 years

        // Default probabilities for each name at the time of the maximun 
        //   horizon date. Cached for perf.
        mutable std::vector<Probability> horizonDefaultPs_;
    };


    /* It could be argued that this concept is part of the copula. In general 
    when the modelled magnitude is parametric one can solve for inversion to get
    the parameter value for a given magnitude value (provided the modelled 
    variable dependence in invertible). In this particular problem the parameter
    is Time and it is solved here where we are alredy in the context of the 
    modelled variable; while the copula policy ignores what it is being modelled
    -- See the comment for faster inversion in the code. Theres potential for
    a generic algorithm here.
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
                return dts_->defaultProbability(curveRef_ + 
                    Period(static_cast<Integer>(t), Days), true) - pd_;
            }
          private:
            const Handle<DefaultProbabilityTermStructure> dts_;
            Real pd_;
            const Date curveRef_;
        };
    }

    //---- Noninlined Defines -------------------------------------------------

    template<class C, class URNG>
        RandomDefaultLM<C, URNG>::RandomDefaultLM(
        const boost::shared_ptr<Basket>& basket,
        const std::vector<std::vector<Real> >& factorWeights,
        const typename C::initTraits& ini,
        Size nSims,
        Size numThreads,
        Real accuracy, 
        BigNatural seed
        ) 
    : basket_(basket), accuracy_(accuracy), seed_(seed), nSims_(nSims),
      numThreads_(std::max<Size>(1, std::min<Size>(numThreads, 
        boost::thread::hardware_concurrency()))),
      nSimsPerThread_(nSims/numThreads_),//chop to size_t
      nSimsLastThread_(numThreads_ == 1 ? 
        nSimsPerThread_ : nSims - nSimsPerThread_*(numThreads_-1)),
      LatentModel<C>(factorWeights, ini) 
    {
        // in the future change 'size' to 'liveSize'
        QL_REQUIRE(basket_->size() == factorWeights.size(), 
            "Incompatible basket and model sizes.");

        initDates();

        registerWith(basket_);
        // baskets are registered with evaluation date anyway
        registerWith(Settings::instance().evaluationDate());
    }

    /* Default implementation to be overriden by full spezializations on 
    combinations of the copula and rng
    */
    template<class C, class URNG>
    void RandomDefaultLM<C, URNG>::performCalculations() const {
        workers_.clear();
        // using model size, should match the number of names not 
        //   defaulted in the basket:
        URNG baseUrng(numFactors()+ size(), seed_); 
        // size() should equals numLiveNames_

        for(Size iThread = 0; iThread < numThreads_-1; iThread++) {
            /* reposition urng:
            tricky: a jump in the sequence does not mean the same jump in 
            the base uniform generator. It is the responsibility of the user
            sending a suitable generator.*/
            baseUrng.skipTo(nSimsPerThread_ * iThread);
            workers_.push_back(boost::make_shared<Worker>(
                Worker(nSimsPerThread_, 
                LatentModel<C>::FactorSampler<URNG>(baseUrng, 
                    copula_), *this)));// ursg must be copied into the rsg
        }
        // last thread takes care of left over sims (typical of a machine 
        //   with an odd number of cpus)
        baseUrng.skipTo(nSimsPerThread_ * (numThreads_-1));
        workers_.push_back(boost::make_shared<Worker>(
            Worker(nSimsLastThread_, 
            LatentModel<C>::FactorSampler<URNG>(baseUrng, 
            copula_), *this)));

        performSimulations();
    }

    // ---- performCalculations full specializations ---------------------------

    /* The role of the spezs is 
    1.- to properly adjust the base rng construction 
    2.- to decide accordingly to the nature of the generation of samples if the 
        simulation is going to be multithread or not. In some cases the 
        specializations are just error treatment since the absence of the 
        'skipTo' method in template classes would stop compilation. Yet the 
        presence of the method does not mean the combination is valid (as it is 
        not with rejection samplers).
    */
   // template<> template<>
    void RandomDefaultLM<GaussianCopulaPolicy, RandomSequenceGenerator<
        BoxMullerGaussianRng<MersenneTwisterUniformRng> > >
        ::performCalculations() const 
    {
            QL_REQUIRE(numThreads_ == 1, 
                "Multithreading not possible with MersenneTwister.");
            workers_.clear();
            RandomSequenceGenerator<
                BoxMullerGaussianRng<MersenneTwisterUniformRng> >
                baseUrng(numFactors() + size(), 
                    BoxMullerGaussianRng<MersenneTwisterUniformRng>(
                        MersenneTwisterUniformRng(seed_))); 

            workers_.push_back(boost::make_shared<Worker>(
                Worker(nSimsPerThread_, LatentModel<GaussianCopulaPolicy>
                    ::FactorSampler<RandomSequenceGenerator<
                        BoxMullerGaussianRng<MersenneTwisterUniformRng> > >(
                            baseUrng), *this)));
            performSimulations();
    }

   // template<> template<>
    void RandomDefaultLM<GaussianCopulaPolicy, 
        RandomSequenceGenerator<MersenneTwisterUniformRng> >
        ::performCalculations() const 
    {
            QL_REQUIRE(numThreads_ == 1, 
                "Multithreading not possible with MersenneTwister.");
            workers_.clear();
            RandomSequenceGenerator<MersenneTwisterUniformRng>
                baseUrng(numFactors()+size(), MersenneTwisterUniformRng(seed_));

            workers_.push_back(boost::make_shared<Worker>(
                Worker(nSimsPerThread_, LatentModel<GaussianCopulaPolicy>
                ::FactorSampler<RandomSequenceGenerator<
                    MersenneTwisterUniformRng> >(baseUrng, copula_), *this)));
            performSimulations();
    }

  //  template<> template<>
    void RandomDefaultLM<TCopulaPolicy, 
        RandomSequenceGenerator<PolarStudentTRng<MersenneTwisterUniformRng> > >
        ::performCalculations() const 
    {
            QL_REQUIRE(numThreads_ == 1, 
                "Multithreading not possible with MersenneTwister.");
            workers_.clear();

            MersenneTwisterUniformRng baseUrng(seed_);

            workers_.push_back(boost::make_shared<Worker>(
                Worker(nSimsPerThread_, LatentModel<TCopulaPolicy>
                ::FactorSampler<RandomSequenceGenerator<
                    PolarStudentTRng<MersenneTwisterUniformRng> > >(baseUrng, 
                        numFactors()+size(), copula_), *this)));
            performSimulations();
    }







    template<class C, class URNG>
    void RandomDefaultLM<C, URNG>::Worker::nextSequence()
    {
        const std::vector<Real>& values = 
            copulasRng_.nextSequence().value;

        const boost::shared_ptr<Pool>& pool = data_.basket_->pool();

        simsBuffer.push_back(std::vector<simEvent>());// starts with no events

        for(Size iName=0; iName<data_.size();/*data_.numLiveNames_;*/ iName++) {
            Real latentVarSample = data_.latentVarValue(values, iName);
            Probability simDefaultProb = 
               data_.copula_.cumulativeY(latentVarSample, iName);
           // If the default simulated lies before the max date:
           if (data_.horizonDefaultPs_[iName] >= simDefaultProb) {
               const Handle<DefaultProbabilityTermStructure>& dfts = 
                    pool->get(pool->names()[iName]).  // use 'live' names
                    defaultProbability(data_.basket_->defaultKeys()[iName]);
               // compute and store default time with respect to the 
               //  curve ref date:
               Size dateSTride =
                   static_cast<Size>(Brent().solve(// casted from Real:
                   detail::Root(dfts, simDefaultProb),data_.accuracy_,0.,1.));
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
               simsBuffer.back().push_back(simEvent(iName, dateSTride));
               //emplace_back
            }
        }
        /* Use to remove sims with no events. Uses less memory, faster 
        post-statistics. But only if all names in the portfolio have low 
        default probability, otherwise is more expensive and sim access has 
        to be modified. However low probability is also an indicator that 
        variance reduction is needed. */
        //if(simsBuffer.back().empty()) {
        //    emptySims_++;// Size; intilzd to zero
        //    simsBuffer.pop_back();
        //}
    }


    template<class C, class URNG>
    Probability RandomDefaultLM<C, URNG>::probAtLeastNEvents(
        Size n, const Date& d) const 
    {
        calculate();
        Date today = Settings::instance().evaluationDate();

        QL_REQUIRE(d>today, "Date for statistic must be in the future.");

        BigInteger val = d.serialNumber() - today.serialNumber();
        
        if(n==0) return 1.;

        Real counts = 0.;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            Integer simCount = 0;
            const std::vector<simEvent>& events = getSim(iSim);
            for(Size iEvt=0; iEvt < events.size(); iEvt++)
                if(val > events[iEvt].dayFromRef) simCount++;
                if(simCount >= n) counts++;
        }
        return counts/nSims_;
        // \todo Provide confidence interval
    }


	template<class C, class URNG>
    Real RandomDefaultLM<C, URNG>::defaultCorrelation(const Date& d,
        Size iName, Size jName) const 
    {
        // a control variate with the probabilities is possible
        calculate();
        Date today = Settings::instance().evaluationDate();
        QL_REQUIRE(d>today, "Date for statistic must be in the future.");

        BigInteger val = d.serialNumber() - today.serialNumber();
        Real expectedDefiDefj = 0.;// E[1_i 1_j]
        // the rest of magnitudes have known values (probabilities) but that 
        //   would distort the simulation results.
        Real expectedDefi = 0.;
        Real expectedDefj = 0.;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent>& events = getSim(iSim);
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


	template<class C, class URNG>
    Disposable<std::vector<Probability> > 
    RandomDefaultLM<C, URNG>::probsBeingNthEvent(
        Size n, const Date& d) const {

        QL_REQUIRE(n>0 && n<=basket_->size(), "Impossible number of defaults.");
        calculate();
        Date today = Settings::instance().evaluationDate();

        QL_REQUIRE(d>today, "Date for statistic must be in the future.");

        BigInteger val = d.serialNumber() - today.serialNumber();

		std::vector<Probability> 
            //hitsByDate(basket_->remainingSize(), 0.);
            hitsByDate(basket_->size(), 0.);
        for(Size iSim=0; iSim < simsBuffer_.size(); iSim++) {
            std::map<unsigned short, unsigned short> namesDefaulting;
            for(Size iEvt=0; iEvt < simsBuffer_[iSim].size(); iEvt++) {
                // if event is within time horizon...
                if(val > simsBuffer_[iSim][iEvt].dayFromRef)
					//...count it. notice insertion sorts by date.
					namesDefaulting.insert(std::make_pair<unsigned short, 
                      unsigned short>(simsBuffer_[iSim][iEvt].dayFromRef, 
                        simsBuffer_[iSim][iEvt].nameIdx));
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

}

#endif
