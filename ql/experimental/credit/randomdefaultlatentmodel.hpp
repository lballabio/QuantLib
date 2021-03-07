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

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/constantlosslatentmodel.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/experimental/math/gaussiancopulapolicy.hpp>
#include <ql/experimental/math/latentmodel.hpp>
#include <ql/experimental/math/tcopulapolicy.hpp>
#include <ql/math/beta.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/statistics/histogram.hpp>
#include <ql/math/statistics/riskstatistics.hpp>
#include <ql/tuple.hpp>
#include <utility>

/* Intended to replace
    ql\experimental\credit\randomdefaultmodel.Xpp
*/

namespace QuantLib {

    /*! Simulation event trait class template forward declaration.
    Each latent model will be modelling different entities according to the
    meaning of the model function which depends on the random \$ Y_i\$
    variables. Still the generation of the factors and variables it is common to
    any model. Only within a given transformation function the model and event
    generated becomes concrete.

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
    // replaces class Loss
    template <class simEventOwner> struct simEvent;


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
    template<template <class, class> class derivedRandomLM, class copulaPolicy,
        class USNG = SobolRsg>
    class RandomLM : public virtual LazyObject,
                     public virtual DefaultLossModel {
    private:
        // Takes the parents type, all children have the same type, the
        // random generation is performed in this class only.
        typedef typename LatentModel<copulaPolicy>::template FactorSampler<USNG>
            copulaRNG_type;
    protected:
      RandomLM(Size numFactors, Size numLMVars, copulaPolicy copula, Size nSims, BigNatural seed)
      : seed_(seed), numFactors_(numFactors), numLMVars_(numLMVars), nSims_(nSims),
        copula_(std::move(copula)) {}

      void update() override {
          simsBuffer_.clear();
          // tell basket to notify instruments, etc, we are invalid
          if (!basket_.empty())
              basket_->notifyObservers();
          LazyObject::update();
        }

        void performCalculations() const override {
            static_cast<const derivedRandomLM<copulaPolicy, USNG>* >(
                this)->initDates();//in update?
            copulasRng_ = ext::make_shared<copulaRNG_type>(copula_, seed_);
            performSimulations();
        }

        void performSimulations() const {
            // Next sequence should determine the event and push it into buffer
            for (Size i = nSims_; i != 0U; i--) {
                const std::vector<Real>& sample =
                    copulasRng_->nextSequence().value;
                static_cast<const derivedRandomLM<copulaPolicy, USNG>* >(
                    this)->nextSample(sample);
            // alternatively make call an explicit local method...
            }
        }

        /* Method to access simulation results and avoiding a copy of
        each thread results buffer. PerformCalculations should have been called.
        Here in the monothread version this method is redundant/trivial but
        serves to detach the statistics access to the way the simulations are
        stored.
        */
        const std::vector<simEvent<derivedRandomLM<copulaPolicy, USNG> > >&
            getSim(const Size iSim) const { return simsBuffer_[iSim]; }

        /* Allows statistics to be written generically for fixed and random
        recovery rates. */
        Real getEventRecovery(
            const simEvent<derivedRandomLM<copulaPolicy, USNG> >& evt) const
        {
            return static_cast<const derivedRandomLM<copulaPolicy, USNG>* >(
                this)->getEventRecovery(evt);
        }

        //! \name Statistics, DefaultLossModel interface.
        // These are virtual and allow for children-specific optimization and
        //   variance reduction. The virtual table is ok, they are not part
        //   of the simulation.
        //@{
        /*! Returns the probaility of having a given or larger number of
        defaults in the basket portfolio at a given time.
        */
        Probability probAtLeastNEvents(Size n, const Date& d) const override;
        /*! Order of results refers to the simulated (super)pool not the
        basket's pool.
        Notice that this statistic suffers from heavy dispersion. To see
        techniques to improve it (not implemented here) see:
        Joshi, M., D. Kainth. 2004. Rapid and accurate development of prices
        and Greeks for nth to default credit swaps in the Li model. Quantitative
        Finance, Vol. 4. Institute of Physics Publishing, London, UK, 266-275
        and:
        Chen, Z., Glasserman, P. 'Fast pricing of basket default swaps' in
        Operations Research Vol. 56, No. 2, March/April 2008, pp. 286-303
        */
        Disposable<std::vector<Probability> > probsBeingNthEvent(Size n,
                                                                 const Date& d) const override;
        //! Pearsons' default probability correlation.
        Real defaultCorrelation(const Date& d, Size iName, Size jName) const override;
        Real expectedTrancheLoss(const Date& d) const override;
        virtual std::pair<Real, Real> expectedTrancheLossInterval(const Date& d,
            Probability confidencePerc) const;
        Disposable<std::map<Real, Probability> > lossDistribution(const Date& d) const override;
        virtual Histogram computeHistogram(const Date& d) const;
        Real expectedShortfall(const Date& d, Real percent) const override;
        Real percentile(const Date& d, Real percentile) const override;
        /*! Returns the VaR value for a given percentile and the 95 confidence
        interval of that value. */
        virtual ext::tuple<Real, Real, Real> percentileAndInterval(
            const Date& d, Real percentile) const;
        /*! Distributes the total VaR amount along the portfolio counterparties.
            The passed loss amount is in loss units.
        */
        Disposable<std::vector<Real> > splitVaRLevel(const Date& date, Real loss) const override;
        /*! Distributes the total VaR amount along the portfolio
            counterparties.

            Provides confidence interval for split so that portfolio
            optimization can be performed outside those limits.

            The passed loss amount is in loss units.
        */
        virtual Disposable<std::vector<std::vector<Real> > > splitVaRAndError(
            const Date& date, Real loss, Probability confInterval) const;
        //@}
    public:
      ~RandomLM() override = default;

    private:
        BigNatural seed_;
    protected:
        const Size numFactors_;
        const Size numLMVars_;

        const Size nSims_;

        mutable std::vector<std::vector<simEvent<derivedRandomLM<copulaPolicy,
            USNG > > > > simsBuffer_;

        mutable copulaPolicy copula_;
        mutable ext::shared_ptr<copulaRNG_type> copulasRng_;

        // Maximum time inversion horizon
        static const Size maxHorizon_ = 4050; // over 11 years
        // Inversion probability limits are computed by children in initdates()
    };


    /* ---- Statistics ---------------------------------------------------  */

    template<template <class, class> class D, class C, class URNG>
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
            const std::vector<simEvent<D<C, URNG> > >& events =
                getSim(iSim);
            for(Size iEvt=0; iEvt < events.size(); iEvt++)
                // duck type on the members:
                if(val > events[iEvt].dayFromRef) simCount++;
            if(simCount >= n) counts++;
        }
        return counts/nSims_;
        // \todo Provide confidence interval
    }

    template<template <class, class> class D, class C, class URNG>
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
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);
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
                       hitsByDate.begin(),
                       divide_by<Real>(Real(nSims_)));
        return hitsByDate;
        // \todo Provide confidence interval
    }


    template<template <class, class> class D, class C, class URNG>
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
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);
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


    template<template <class, class> class D, class C, class URNG>
    Real RandomLM<D, C, URNG>::expectedTrancheLoss(
        const Date& d) const {
            return expectedTrancheLossInterval(d, 0.95).first;
    }


    template<template <class, class> class D, class C, class URNG>
    std::pair<Real, Real> RandomLM<D, C, URNG>::expectedTrancheLossInterval(
        const Date& d, Probability confidencePerc) const
    {
        calculate();
        Date today = Settings::instance().evaluationDate();
        Date::serial_type val = d.serialNumber() - today.serialNumber();

        Real attachAmount = basket_->attachmentAmount();
        Real detachAmount = basket_->detachmentAmount();

        // Real trancheLoss= 0.;
        GeneralStatistics lossStats;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);

            Real portfSimLoss=0.;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                // if event is within time horizon...
                if(val > static_cast<Date::serial_type>(
					   events[iEvt].dayFromRef)) {
                    Size iName = events[iEvt].nameIdx;
                    // ...and is contained in the basket.
                        portfSimLoss +=
                            basket_->exposure(basket_->names()[iName],
                                Date(events[iEvt].dayFromRef +
                                    today.serialNumber())) *
                                        (1.-getEventRecovery(events[iEvt]));
               }
            }
            lossStats.add(// d  ates? current losses? realized defaults, not yet
                std::min(std::max(portfSimLoss - attachAmount, 0.),
                    detachAmount - attachAmount) );
        }
        return std::make_pair(lossStats.mean(), lossStats.errorEstimate() *
            InverseCumulativeNormal::standard_value(0.5*(1.+confidencePerc)));
    }


    template<template <class, class> class D, class C, class URNG>
    Disposable<std::map<Real, Probability> >
        RandomLM<D, C, URNG>::lossDistribution(const Date& d) const {

        Histogram hist = computeHistogram(d);
        std::map<Real, Probability> distrib;

        // prob of losses less or equal to
        Real suma = hist.frequency(0);
        distrib.insert(std::make_pair(0., suma));
        for(Size i=1; i<hist.bins(); i++) {
            suma += hist.frequency(i);
            distrib.insert(std::make_pair( hist.breaks()[i-1], suma ));
        }
        return distrib;
    }


    template<template <class, class> class D, class C, class URNG>
    Histogram RandomLM<D, C, URNG>::computeHistogram(const Date& d) const {
        std::vector<Real> data;
        std::set<Real> keys;// attainable loss values
        keys.insert(0.);
        Date today = Settings::instance().evaluationDate();
        Date::serial_type val = d.serialNumber() - today.serialNumber();
        // redundant test? should have been tested by the basket caller?
        QL_REQUIRE(d >= today,
            "Requested percentile date must lie after computation date.");
        calculate();

        Real attachAmount = basket_->attachmentAmount();
        Real detachAmount = basket_->detachmentAmount();

        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);

            Real portfSimLoss=0.;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                if(val > static_cast<Date::serial_type>(
					 events[iEvt].dayFromRef)) {
                    Size iName = events[iEvt].nameIdx;
          // test needed (here and the others) to reuse simulations:
          //          if(basket_->pool()->has(copula_->pool()->names()[iName]))
                        portfSimLoss +=
                            basket_->exposure(basket_->names()[iName],
                                Date(events[iEvt].dayFromRef +
                                    today.serialNumber())) *
                                        (1.-getEventRecovery(events[iEvt]));
                }
            }
            data.push_back(std::min(std::max(portfSimLoss - attachAmount, 0.),
                detachAmount - attachAmount));
            keys.insert(data.back());
        }
        // avoid using as many points as in the simulation.
        Size nPts = std::min<Size>(data.size(), 150);// fix
        return Histogram(data.begin(), data.end(), nPts);
    }


    template<template <class, class> class D, class C, class URNG>
    Real RandomLM<D, C, URNG>::expectedShortfall(const Date& d,
        Real percent) const {

        const Date today = Settings::instance().evaluationDate();
        QL_REQUIRE(d >= today,
            "Requested percentile date must lie after computation date.");
        calculate();

        Real attachAmount = basket_->attachmentAmount();
        Real detachAmount = basket_->detachmentAmount();

        Date::serial_type val = d.serialNumber() - today.serialNumber();
        if(val <= 0) return 0.;// plus basket realized losses

        //GenericRiskStatistics<GeneralStatistics> statsX;
        std::vector<Real> losses;
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);
            Real portfSimLoss=0.;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                if(val > static_cast<Date::serial_type>(
					  events[iEvt].dayFromRef)) {
                    Size iName = events[iEvt].nameIdx;
                    // ...and is contained in the basket.
                    //if(basket_->pool()->has(copula_->pool()->names()[iName]))
                        portfSimLoss +=
                            basket_->exposure(basket_->names()[iName],
                                Date(events[iEvt].dayFromRef +
                                    today.serialNumber())) *
                                        (1.-getEventRecovery(events[iEvt]));
                }
            }
            portfSimLoss = std::min(std::max(portfSimLoss - attachAmount, 0.),
                detachAmount - attachAmount);
            losses.push_back(portfSimLoss);
        }

        std::sort(losses.begin(), losses.end());
        Real posit = std::ceil(percent * nSims_);
        posit = posit >= 0. ? posit : 0.;
        Size position = static_cast<Size>(posit);
        Real perctlInf = losses[position];//q_{\alpha}

        // the prob of values strictly larger than the quantile value.
        Probability probOverQ =
            static_cast<Real>(std::distance(losses.begin() + position,
                losses.end())) / static_cast<Real>(nSims_);

        return ( perctlInf * (1.-percent-probOverQ) +//<-correction term
            std::accumulate(losses.begin() + position, losses.end(), 
			    Real(0.))/nSims_
                )/(1.-percent);

        /* Alternative ESF definition; find the first loss larger than the
        one of the percentile. Notice the choice here, the expected shortfall
        is understood in the sense that we are looking for the average given
        than losses are above a certain value rather than above a certain
        probability:
        (Unlikely to be the algorithm of choice)*/
        /*
        std::vector<Real>::iterator itPastPerc =
            std::find_if(losses.begin() + position, losses.end(),
                         greater_or_equal_to<Real>(perctlInf));
        // notice if the sample is flat at the end this might be zero
        Size pointsOverVal = nSims_ - std::distance(itPastPerc, losses.end());
        return pointsOverVal == 0 ? 0. :
            std::accumulate(itPastPerc, losses.end(), 0.) / pointsOverVal;
        */

        /* For the definition of ESF see for instance: 'Quantitative Risk
        Management' by A.J. McNeil, R.Frey and P.Embrechts, princeton series in
        finance, 2005; equations on page 39 sect 2.12:
        $q_{\alpha}(F) = inf{x \in R : F(x) \le \alpha}$
        and equation 2.25 on p. 45:
        $ESF_{\alpha} = \frac{1}{1-\alpha} [E(L; L \ge q_{\alpha} ) +
            q_{\alpha} (1-\alpha-P(L \ge q_{\alpha})) ]$
        The second term accounts for non continuous distributions.
        */
    }


    template<template <class, class> class D, class C, class URNG>
    Real RandomLM<D, C, URNG>::percentile(const Date& d, Real perc) const {
        // need to specify return type in tuples' get is parametric
        return ext::get<0>(percentileAndInterval(d, perc));
    }


    /* See Appendix-A of "Evaluating value-at-risk methodologies: Accuracy
        versus computational time.", M. Pritsker, Wharton FIC, November 1996
    Strictly speaking this gives the interval with a 95% probability of
    the true value being within the interval; which is different to the error
    of the stimator just computed. See the reference for a discussion.
    */
    template<template <class, class> class D, class C, class URNG>
    ext::tuple<Real, Real, Real> // disposable?
        RandomLM<D, C, URNG>::percentileAndInterval(const Date& d,
            Real percentile) const {

        QL_REQUIRE(percentile >= 0. && percentile <= 1.,
            "Incorrect percentile");
        calculate();

        Real attachAmount = basket_->attachmentAmount();
        Real detachAmount = basket_->detachmentAmount();

        std::vector<Real> rankLosses;
        Date today = Settings::instance().evaluationDate();
        Date::serial_type val = d.serialNumber() - today.serialNumber();
        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);
            Real portfSimLoss=0.;
            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                if(val > static_cast<Date::serial_type>(
					 events[iEvt].dayFromRef)) {
                    Size iName = events[iEvt].nameIdx;
                 //   if(basket_->pool()->has(copula_->pool()->names()[iName]))
                        portfSimLoss +=
                            basket_->exposure(basket_->names()[iName],
                                Date(events[iEvt].dayFromRef +
                                    today.serialNumber())) *
                                        (1.-getEventRecovery(events[iEvt]));
                }
            }
            portfSimLoss = std::min(std::max(portfSimLoss - attachAmount, 0.),
                detachAmount - attachAmount);
            // update dataset for rank stat:
            rankLosses.push_back(portfSimLoss);
        }

        std::sort(rankLosses.begin(), rankLosses.end());
        Size quantilePosition = static_cast<Size>(floor(nSims_*percentile));
        Real quantileValue = rankLosses[quantilePosition];

        // compute confidence interval:
        const Probability confInterval = 0.95;// as an argument?
        Real lowerPercentile, upperPercentile;
        Size r = quantilePosition - 1;
        Size s = quantilePosition + 1;
        bool rLocked = false,
            sLocked = false;
        // Size rfinal = 0,
        //      sfinal = 0;
        for(Size delta=1; delta < quantilePosition; delta++) {
            Real cached =
                incompleteBetaFunction(Real(s), Real(nSims_+1-s),
                                       percentile, 1.e-8, 500);
            Real pMinus =
            /* There was a fix in the repository on the gammadistribution. It
            might impact these, it might be neccesary to multiply these values
            by '-1'*/
                incompleteBetaFunction(Real(r+1), Real(nSims_-r),
                                       percentile, 1.e-8, 500)
                - cached;
            Real pPlus  =
                incompleteBetaFunction(Real(r), Real(nSims_-r+1),
                                       percentile, 1.e-8, 500)
                - cached;
            if((pMinus > confInterval) && !rLocked ) {
                // rfinal = r + 1;
               rLocked = true;
            }
            if((pPlus >= confInterval) && !sLocked) {
                // sfinal = s;
                sLocked = true;
            }
            if(rLocked && sLocked) break;
            r--;
            s++;
            s = std::min(nSims_-1, s);
        }
        lowerPercentile = rankLosses[r];
        upperPercentile = rankLosses[s];

        return {quantileValue, lowerPercentile, upperPercentile};
    }


    template<template <class, class> class D, class C, class URNG>
    Disposable<std::vector<Real> > RandomLM<D, C, URNG>::splitVaRLevel(
        const Date& date, Real loss) const
    {
        std::vector<Real> varLevels = splitVaRAndError(date, loss, 0.95)[0];
        // turn relative units into absolute:
        std::transform(varLevels.begin(), varLevels.end(), varLevels.begin(),
                       multiply_by<Real>(loss));
        return varLevels;
    }


    // parallelize this one(if possible), it is really expensive
    template<template <class, class> class D, class C, class URNG>
    /* FIX ME: some trouble on limit cases, like zero loss or no losses over the
    requested level.*/
    Disposable<std::vector<std::vector<Real> > >
        RandomLM<D, C, URNG>::splitVaRAndError(const Date& date, Real loss,
            Probability confInterval) const
    {
        /* Check 'loss' value integrity: i.e. is within tranche limits? (should
            have been done basket...)*/
        calculate();

        Real attachAmount = basket_->attachmentAmount();
        Real detachAmount = basket_->detachmentAmount();
        Size numLiveNames = basket_->remainingSize();

        std::vector<Real> split(numLiveNames, 0.);
        std::vector<GeneralStatistics> splitStats(numLiveNames,
            GeneralStatistics());
        Date today = Settings::instance().evaluationDate();
        Date::serial_type val = date.serialNumber() - today.serialNumber();

        for(Size iSim=0; iSim < nSims_; iSim++) {
            const std::vector<simEvent<D<C, URNG> > >& events = getSim(iSim);
            Real portfSimLoss=0.;
            //std::vector<Real> splitBuffer(numLiveNames_, 0.);
            std::vector<simEvent<D<C, URNG> > > splitEventsBuffer;

            for(Size iEvt=0; iEvt < events.size(); iEvt++) {
                if(val > static_cast<Date::serial_type>(
					 events[iEvt].dayFromRef)) {
                    Size iName = events[iEvt].nameIdx;
                // if(basket_->pool()->has(copula_->pool()->names()[iName])) {
                        portfSimLoss +=
                            basket_->exposure(basket_->names()[iName],
                                Date(events[iEvt].dayFromRef +
                                    today.serialNumber())) *
                                        (1.-getEventRecovery(events[iEvt]));
                        //and will sort later if buffer applies:
                        splitEventsBuffer.push_back(events[iEvt]);
                }
            }
            portfSimLoss = std::min(std::max(portfSimLoss - attachAmount, 0.),
                detachAmount - attachAmount);

            /* second pass; split is conditional to total losses within target
            losses/percentile:  */
            Real ptflCumulLoss = 0.;
            if(portfSimLoss > loss) {
                std::sort(splitEventsBuffer.begin(), splitEventsBuffer.end());
                //NOW THIS:
                split.assign(numLiveNames, 0.);
                /*  if the name triggered a loss in the portf limits assign
                this loss to that name..  */
                for(Size i=0; i<splitEventsBuffer.size(); i++) {
                    Size iName = splitEventsBuffer[i].nameIdx;
                    Real lossName =
            // allows amortizing (others should be like this)
            // basket_->remainingNotionals(Date(simsBuffer_[i].dayFromRef +
            //      today.serialNumber()))[iName] *
                        basket_->exposure(basket_->names()[iName],
                            Date(splitEventsBuffer[i].dayFromRef +
                                today.serialNumber())) *
                                (1.-getEventRecovery(splitEventsBuffer[i]));

                    Real tranchedLossBefore =
                        std::min(std::max(ptflCumulLoss - attachAmount, 0.),
                        detachAmount - attachAmount);
                    ptflCumulLoss += lossName;
                    Real tranchedLossAfter =
                        std::min(std::max(ptflCumulLoss - attachAmount, 0.),
                        detachAmount - attachAmount);
                    // assign new losses:
                    split[iName] += tranchedLossAfter - tranchedLossBefore;
                }
                for(Size iName=0; iName<numLiveNames; iName++) {
                    splitStats[iName].add(split[iName] /
                        std::min(std::max(ptflCumulLoss - attachAmount, 0.),
                            detachAmount - attachAmount) );
                }
            }
        }

        // Compute error in VaR split
        std::vector<Real> means, rangeUp, rangeDown;
        Real confidFactor = InverseCumulativeNormal()(0.5+confInterval/2.);
        for(Size iName=0; iName<numLiveNames; iName++) {
            means.push_back(splitStats[iName].mean());
            Real error = confidFactor * splitStats[iName].errorEstimate() ;
            rangeDown.push_back(means.back() - error);
            rangeUp.push_back(means.back() + error);
        }

        std::vector<std::vector<Real> > results;
        results.push_back(means);
        results.push_back(rangeDown);
        results.push_back(rangeUp);

        return results;
    }




    // --------- Time inversion solver target function: -----------------------

    /* It could be argued that this concept is part of the copula (more generic)
    In general when the modelled magnitude is parametric one can solve for
    inversion to get the parameter value for a given magnitude value (provided
    the modelled variable dependence in invertible). In this particular problem
    the parameter is Time and it is solved here where we are alredy in the
    context of default
    See default transition models for another instance of this inversion.
    Alternatively use the faster trick (flat HR) mentioned in the code or make
    the algorithm parametric on the type of interpolation in the default TS.
    */
    namespace detail {// not template dependent .....move it
        //! Utility for the numerical time solver
        class Root {
          public:
            /* See a faster algorithm (neeeds to locate the points) in
            D.O'KANE p.249 sect 13.5 */
            Root(const Handle<DefaultProbabilityTermStructure>& dts, Real pd)
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

    /*
    ---------------------------------------------------------------------------
    ---------------------------------------------------------------------------
    */

    // move this one to a separte file?
    /*! Random default with deterministic recovery event type.\par
    Stores sims results in a bitfield buffer for lean memory storage.
    Although strictly speaking this is not guaranteed by the compiler it
    amounts to reducing the memory storage by half.
    Some computations, like conditional statistics, precise that all sims
    results be available.
    */
    template<class , class > class RandomDefaultLM;
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

    /*! Default only latent model simulation with trivially fixed recovery
        amounts.
    */
    template<class copulaPolicy, class USNG = SobolRsg>
    class RandomDefaultLM : public RandomLM<RandomDefaultLM, copulaPolicy, USNG>
    {
    private:
        typedef simEvent<RandomDefaultLM> defaultSimEvent;

        // \todo Consider this to be only a ConstantLossLM instead
        const ext::shared_ptr<DefaultLatentModel<copulaPolicy> > model_;
        const std::vector<Real> recoveries_;
        // for time inversion:
        Real accuracy_;
    public:
        // \todo: Allow a constructor building its own default latent model.
      explicit RandomDefaultLM(const ext::shared_ptr<DefaultLatentModel<copulaPolicy> >& model,
                               const std::vector<Real>& recoveries = std::vector<Real>(),
                               Size nSims = 0, // stats will crash on div by zero, FIX ME.
                               Real accuracy = 1.e-6,
                               BigNatural seed = 2863311530UL)
      : RandomLM< ::QuantLib::RandomDefaultLM, copulaPolicy, USNG>(
            model->numFactors(), model->size(), model->copula(), nSims, seed),
        model_(model),
        recoveries_(recoveries.empty() ? std::vector<Real>(model->size(), 0.) : recoveries),
        accuracy_(accuracy) {
          // redundant through basket?
          this->registerWith(Settings::instance().evaluationDate());
          this->registerWith(model_);
        }
        explicit RandomDefaultLM(
            const ext::shared_ptr<ConstantLossLatentmodel<copulaPolicy> >& model,
            Size nSims = 0,// stats will crash on div by zero, FIX ME.
            Real accuracy = 1.e-6,
            BigNatural seed = 2863311530UL)
        : RandomLM< ::QuantLib::RandomDefaultLM, copulaPolicy, USNG>
            (model->numFactors(), model->size(), model->copula(),
                nSims, seed ),
          model_(model),
          recoveries_(model->recoveries()),
          accuracy_(accuracy)
        {
            // redundant through basket?
            this->registerWith(Settings::instance().evaluationDate());
            this->registerWith(model_);
        }

        // grant access to static polymorphism:
        /* While this works on g++, VC9 refuses to compile it.
        Not completely sure whos right; individually making friends of the
        calling members or writting explicitly the derived class T parameters
        throws the same errors.
        The access is then open to the member fucntions.
        Another solution is to use this http://accu.org/index.php/journals/296

        It might well be that gcc is allowing some c11 features silently, which
        wont pass on a lower gcc version.
        */
        friend class RandomLM< ::QuantLib::RandomDefaultLM, copulaPolicy, USNG>;
    protected:
        void nextSample(const std::vector<Real>& values) const;
        void initDates() const {
            /* Precalculate horizon time default probabilities (used to
              determine if the default took place and subsequently compute its
              event time)
            */
            Date today = Settings::instance().evaluationDate();
            Date maxHorizonDate = today  + Period(this->maxHorizon_, Days);

            const ext::shared_ptr<Pool>& pool = this->basket_->pool();
            for(Size iName=0; iName < this->basket_->size(); ++iName)//use'live'
                horizonDefaultPs_.push_back(pool->get(pool->names()[iName]).
                    defaultProbability(this->basket_->defaultKeys()[iName])
                        ->defaultProbability(maxHorizonDate, true));
        }
        Real getEventRecovery(const defaultSimEvent& evt) const {
            return recoveries_[evt.nameIdx];
        }
        Real expectedRecovery(const Date&, Size iName, const DefaultProbKey&) const override {
            // deterministic
            return recoveries_[iName];
        }

        Real latentVarValue(const std::vector<Real>& factorsSample,
            Size iVar) const {
            return model_->latentVarValue(factorsSample, iVar);
        }
        //allows statistics to know the portfolio size (could be moved to parent
        //invoking duck typing on the variable name or a handle to the basket)
        Size basketSize() const { return model_->size(); }
    private:
      void resetModel() override /*const*/ {
          /* Explore: might save recalculation if the basket is the same
          (some situations, like BC or control variates) in that case do not
          update, only reset the model's basket.
          */
          model_->resetBasket(this->basket_.currentLink());

          QL_REQUIRE(this->basket_->size() == model_->size(),
                     "Incompatible basket and model sizes.");
          QL_REQUIRE(recoveries_.size() == this->basket_->size(),
                     "Incompatible basket and recovery sizes.");
          // invalidate current calculations if any and notify observers
          // NOLINTNEXTLINE(bugprone-parent-virtual-call)
          LazyObject::update();
      }
        // This one and the buffer might be moved to the parent, only some
        //   dates might be specific to a particular model.
        // Default probabilities for each name at the time of the maximun
        //   horizon date. Cached for perf.
        mutable std::vector<Probability> horizonDefaultPs_;
    };





    template<class C, class URNG>
    void RandomDefaultLM<C, URNG>::nextSample(
        const std::vector<Real>& values) const
    {
        const ext::shared_ptr<Pool>& pool = this->basket_->pool();
        // starts with no events
        this->simsBuffer_.push_back(std::vector<defaultSimEvent> ());

        for(Size iName=0; iName<model_->size(); iName++) {
            Real latentVarSample =
                model_->latentVarValue(values, iName);
            Probability simDefaultProb =
               model_->cumulativeY(latentVarSample, iName);
            // If the default simulated lies before the max date:
            if (horizonDefaultPs_[iName] >= simDefaultProb) {
                const Handle<DefaultProbabilityTermStructure>& dfts =
                    pool->get(pool->names()[iName]).// use 'live' names
                    defaultProbability(this->basket_->defaultKeys()[iName]);
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
                this->simsBuffer_.back().push_back(defaultSimEvent(iName,
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




    // Common usage typedefs (notice they vary in the multithread version)
    // ---------- Gaussian default generators options ------------------------
    /* Uses copula direct normal inversion and MT generator
    typedef RandomDefaultLM<GaussianCopulaPolicy,
        RandomSequenceGenerator<MersenneTwisterUniformRng> >
            GaussianRandomDefaultLM;
    */
    /* Uses BoxMuller for gaussian generation, bypassing copula inversions
    typedef RandomDefaultLM<GaussianCopulaPolicy, RandomSequenceGenerator<
        BoxMullerGaussianRng<MersenneTwisterUniformRng> > >
            GaussianRandomDefaultLM;
    */
    /* Default case, uses the copula inversion directly and sobol sequence */
    typedef RandomDefaultLM<GaussianCopulaPolicy> GaussianRandomDefaultLM;

    // ---------- T default generators options ----------------------------
    /* Uses copula inversion and MT base generation
    typedef RandomDefaultLM<TCopulaPolicy,
      RandomSequenceGenerator<MersenneTwisterUniformRng> > TRandomDefaultLM;
    */
    /* Uses MT and polar direct strudent-T generation
    typedef RandomDefaultLM<TCopulaPolicy,
        RandomSequenceGenerator<PolarStudentTRng<MersenneTwisterUniformRng> > >
            TRandomDefaultLM;
    */
    /* Default case, uses sobol sequence and copula inversion */
    typedef RandomDefaultLM<TCopulaPolicy> TRandomDefaultLM;

}

#endif
