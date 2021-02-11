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

#ifndef quantlib_base_correl_lossmodel_hpp
#define quantlib_base_correl_lossmodel_hpp


#include <ql/quote.hpp>
#include <ql/quotes/simplequote.hpp>

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>
#include <ql/experimental/credit/basecorrelationstructure.hpp>

// move these to the CPP (and the template spezs)
#include <ql/experimental/credit/binomiallossmodel.hpp>
#include <ql/experimental/credit/gaussianlhplossmodel.hpp>
#include <ql/experimental/credit/inhomogeneouspooldef.hpp>
#include <utility>

namespace QuantLib {

    /*! Base Correlation loss model; interpolation is performed by portfolio 
    (live) amount percentage.\par
    Though the literature on this model is inmense, see for a more than 
    introductory level (precrisis) chapters 19, 20 and 21 of <b>Modelling single
    name and multi-name credit derivatives.</b> Dominic O'Kane, Wiley Finance, 
    2008\par
    For freely available documentation see:\par
    Credit Correlation: A Guide; JP Morgan Credit Derivatives Strategy; 
        12 March 2004 \par
    Introducing Base Correlations; JP Morgan Credit Derivatives Strategy; 
        22 March 2004 \par
    A Relative Value Framework for Credit Correlation; JP Morgan Credit 
        Derivatives Strategy; 27 April 2004 \par
    Valuing and Hedging Synthetic CDO Tranches Using Base Correlations; Bear 
        Stearns; May 17, 2004 \par
    Correlation Primer; Nomura Fixed Income Research, August 6, 2004 \par
    Base Correlation Explained; Lehman Brothers Fixed Income Quantitative 
        Credit Research; 15 November 2004 \par
    'Pricing CDOs with a smile' in Societe Generale Credit Research; 
        February 2005 \par
    For bespoke base correlation see: \par
    Base Correlation Mapping in Lehman Brothers' Quantitative Credit Research 
        Quarterly; Volume 2007-Q1 \par
    You can explore typical postcrisis data by perusing some of the JPMorgan 
    Global Correlation Daily Analytics \par
    Here the crisis model problems of ability to price stressed portfolios 
    or tranches over the maximum loss are the responsibility of the base models.
    Users should select their models according to this; choosing the copula or
    a random loss given default base model (or more exotic ones). \par
    Notice this is different to a bespoke base correlation loss (bespoke here 
    refering to basket composition, not just attachment levels) ; where 
    loss interpolation is on the expected loss value to match the two baskets. 
    Therefore the correlation surface should refer to the same basket intended
    to be priced. But this is left to the user and is not implemented in the 
    correlation surface (yet...)

    \todo Bespoke portfolios BC models are yet to be implemented.

    BaseModel_T must have a constructor with a single quote value
    */
    /* Criticism:
    This model is not as generic as it could be. In principle a default loss 
    model dependent on a single factor correlation parameter is the only 
    restriction on the base loss model(s) type. This class however is tied to a 
    LatentModel single factor. But there is no need for the 
    underlying model to be of a latent type. This link is due to the copula 
    initialization traits which have to be present for non trivial copula 
    policies initialization (e.g. Student-T base correl models)

    Maybe a possibility is to pass copiable instances of the model and relinking
    to the correlation in two internal copies.
    */
    template <class BaseModel_T, class Corr2DInt_T>
    class BaseCorrelationLossModel : public DefaultLossModel, 
        public virtual Observer {
    private:
        typedef typename BaseModel_T::copulaType::initTraits initTraits;
    public:
      BaseCorrelationLossModel(const Handle<BaseCorrelationTermStructure<Corr2DInt_T> >& correlTS,
                               std::vector<Real> recoveries,
                               const initTraits& traits = initTraits())
      : localCorrelationAttach_(ext::make_shared<SimpleQuote>(0.)),
        localCorrelationDetach_(ext::make_shared<SimpleQuote>(0.)),
        recoveries_(std::move(recoveries)), correlTS_(correlTS), copulaTraits_(traits) {
          registerWith(correlTS);
          registerWith(Settings::instance().evaluationDate());
      }

    private:
        // react to base correl surface notifications (quotes or reference date)
      void update() override {
          setupModels();
          // tell basket to notify instruments, etc, we are invalid
          if (!basket_.empty())
              basket_->notifyObservers();
      }

        /* Update model caches after basket assignement. */
      void resetModel() override {
          remainingNotional_ = basket_->remainingNotional();
          attachRatio_ = basket_->remainingAttachmentAmount() / remainingNotional_;
          detachRatio_ = basket_->remainingDetachmentAmount() / remainingNotional_;

          basketAttach_ = ext::make_shared<Basket>(basket_->refDate(), basket_->remainingNames(),
                                                   basket_->remainingNotionals(), basket_->pool(),
                                                   0.0, attachRatio_, basket_->claim());
          basketDetach_ = ext::make_shared<Basket>(basket_->refDate(), basket_->remainingNames(),
                                                   basket_->remainingNotionals(), basket_->pool(),
                                                   0.0, detachRatio_, basket_->claim());
          setupModels();
      }
        /* Most of the statistics are not implemented, not impossible but
        the model is intended for pricing rather than ptfolio risk management.
        */
      Real expectedTrancheLoss(const Date& d) const override;

    protected:
        /*! Sets up attach/detach models. Gets called on basket update. 
        To be specialized on the spacific model type.
        */
        void setupModels() const;
    private:
        mutable Real attachRatio_, detachRatio_;
        mutable Real remainingNotional_;

        //! Correlation buffer to pick up values from the surface and 
        //  trigger calculation.
        ext::shared_ptr<SimpleQuote> localCorrelationAttach_, 
            localCorrelationDetach_;
        mutable ext::shared_ptr<Basket> basketAttach_,
            basketDetach_;
        // just cached for the update method
        mutable std::vector<Real> recoveries_;
        Handle<BaseCorrelationTermStructure<Corr2DInt_T> > correlTS_;
        // Initialization parameters for models copula
        mutable typename BaseModel_T::copulaType::initTraits copulaTraits_;
        // Models of equity baskets.
        mutable ext::shared_ptr<BaseModel_T> scalarCorrelModelAttach_;
        mutable ext::shared_ptr<BaseModel_T> scalarCorrelModelDetach_;
    };


    // Remember ETL returns the EL on the live part of the basket. 
    template<class LM, class I>
    Real BaseCorrelationLossModel<LM, I>::expectedTrancheLoss(
        const Date& d) const 
    {
        Real correlK1 = correlTS_->correlation(d, attachRatio_);
        Real correlK2 = correlTS_->correlation(d, detachRatio_);

        /* reset correl and call base models which have the different baskets 
        associated.*/
        localCorrelationAttach_->setValue(correlK1);
        Real expLossK1 = 
            basketAttach_->expectedTrancheLoss(d);
        localCorrelationDetach_->setValue(correlK2);
        Real expLossK2 = 
            basketDetach_->expectedTrancheLoss(d);
        return expLossK2 - expLossK1;
    }


    // ----------------------------------------------------------------------


    /* Concrete specializations submodels construction. With the dummy template 
    parameter trick partial specializations leaving the interpolation open 
    would be possible.
    */

    #ifndef QL_PATCH_SOLARIS

    template<>
    inline void BaseCorrelationLossModel<GaussianLHPLossModel, 
        BilinearInterpolation>::setupModels() const 
    {
        // on this assignment any previous registration with the attach and 
        //   detach baskets should be removed
        scalarCorrelModelAttach_ = ext::make_shared<GaussianLHPLossModel>(
            Handle<Quote>(localCorrelationAttach_), recoveries_);
        scalarCorrelModelDetach_ = ext::make_shared<GaussianLHPLossModel>(
            Handle<Quote>(localCorrelationDetach_), recoveries_);

        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);
    }

    template<>
    inline void BaseCorrelationLossModel<GaussianBinomialLossModel, 
        BilinearInterpolation>::setupModels() const 
    {
        ext::shared_ptr<GaussianConstantLossLM> lmA = 
            ext::make_shared<GaussianConstantLossLM>(
                Handle<Quote>(localCorrelationAttach_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);
        ext::shared_ptr<GaussianConstantLossLM> lmD = 
            ext::make_shared<GaussianConstantLossLM>(
                Handle<Quote>(localCorrelationDetach_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);
        scalarCorrelModelAttach_ = 
            ext::make_shared<GaussianBinomialLossModel>(lmA);
        scalarCorrelModelDetach_ = 
            ext::make_shared<GaussianBinomialLossModel>(lmD);
            
        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);

    }

    template<>
    inline void BaseCorrelationLossModel<TBinomialLossModel, 
        BilinearInterpolation>::setupModels() const 
    {
        ext::shared_ptr<TConstantLossLM> lmA = 
            ext::make_shared<TConstantLossLM>(
                Handle<Quote>(localCorrelationAttach_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);
        ext::shared_ptr<TConstantLossLM> lmD = 
            ext::make_shared<TConstantLossLM>(
                Handle<Quote>(localCorrelationDetach_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);

        scalarCorrelModelAttach_ = 
            ext::make_shared<TBinomialLossModel>(lmA);
        scalarCorrelModelDetach_ = 
            ext::make_shared<TBinomialLossModel>(lmD);
            
        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);
    }

    /* \todo Fix this model, is failing for equity tranches at least, the
    base model works all right, its the link here.
    */
    template<>
    inline void BaseCorrelationLossModel<IHGaussPoolLossModel, 
        BilinearInterpolation>::setupModels() const 
    {
        ext::shared_ptr<GaussianConstantLossLM> lmA = 
            ext::make_shared<GaussianConstantLossLM>(
                Handle<Quote>(localCorrelationAttach_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);
        ext::shared_ptr<GaussianConstantLossLM> lmD = 
            ext::make_shared<GaussianConstantLossLM>(
                Handle<Quote>(localCorrelationDetach_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);

        // \todo Allow the sending specific model params, as the number of 
        //   buckets here.
        scalarCorrelModelAttach_ = 
            ext::make_shared<IHGaussPoolLossModel>(lmA, 500);
        scalarCorrelModelDetach_ = 
            ext::make_shared<IHGaussPoolLossModel>(lmD, 500);
            
        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);
    }

    #endif


    // Vanilla BC model
    #ifndef QL_PATCH_SOLARIS
    typedef BaseCorrelationLossModel<GaussianLHPLossModel, 
                BilinearInterpolation> GaussianLHPFlatBCLM;
    #endif

}

#endif
