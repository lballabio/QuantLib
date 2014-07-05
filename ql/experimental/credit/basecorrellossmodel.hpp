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


//--PP #include <ql/experimental/credit/correlationstructure.hpp>
#include <ql/quote.hpp>
#include <ql/quotes/simplequote.hpp>

#include <ql/experimental/credit/basket.hpp>
#include <ql/experimental/credit/defaultlossmodel.hpp>

// move these to the CPP (and the template spezs)
#include <ql/experimental/credit/gaussianlhplossmodel.hpp>
#include <ql/experimental/credit/binomiallossmodel.hpp>
/////#include <ql/experimental/credit/losslatentmodel.hpp>//OLD ---------------------------------  

#include <ql/experimental/credit/basecorrelationstructure.hpp>

#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <ql/time/date.hpp>

namespace QuantLib {

// adding an abstract interpolatedBC class (non template) before the one I have I would not need the interpolation type parameter

    // This model is not as generic as it could be. In principle a default loss model dependent on a single factor correlation parameter is the only restriction on the base loss model. This class however is tied to a LatentModel single factor. But theoretically there is no need for the underlying model to be of a latent type. Another (temporary) restriction here is that the latent model is of Gaussian type.

    // Another possibility is to have a model that admits a constructor via a correl quote handle, then the constructor for the particular type is worked out in a way that this class owns the handle and can perform updates on the local correl through the quote....

    // The reason to prefer a template for the engine we refer to rather than a pointer is that the BC engine must own the correlation quote and the engine it points to has to be generated with this correl quote.
    template <class BaseModel_T, // is this template really neccesary? I couldnt just be a ptr to an (abstract type) DefaultLossModel??? The restriction is that the model should have a constructor that register it with a correl quote (the one member in this class).
            class Corr2DInt_T> /// models must have a constructor with a single quote value, models need to be not just single parameter but also latent model based.
    class BaseCorrelationLossModel : public DefaultLossModel {
      public:
        BaseCorrelationLossModel(
            //Handle<Quote> correlation, 
            // -- const Handle<GaussianLatentModel>& copula,// MOVE TO ABSTRACT LATENTMODEL
       ///////     const Handle<Basket_N>& basket,
            Handle<BaseCorrelationTermStructure<Corr2DInt_T> >& correlTS,//////// get rid of the T param
       //     Handle<DefaultLossModel> baseEngine,..no: it could come from another basket, need two (so I would need cloning).....
    ////////        const Date& refDate,
            const std::vector<Real>& recoveries,
            //LATENT MODEL TRAITS------------------------------------------------------------------------and store in member LHP has one too
            const typename BaseModel_T::copulaType::initTraits& traits = BaseModel_T::copulaType::initTraits()
            )
        : localCorrelation_(boost::shared_ptr<SimpleQuote>(new SimpleQuote(0.))),
   ///////////////////       copula_(copula),
          correlTS_(correlTS),
          recoveries_(recoveries),
          copulaTraits_(traits)//////////////,
      ////////////////////    , fileDbg_("C:\\fileDbg.txt", std::ios::out |std::ios::in | std::ios::trunc)
      //    attachModel_(boost::shared_ptr()), detachModel_(boost::shared_ptr()),

   //////       DefaultLossModel(basket, refDate)
        { 
            // set up remaining basket
   //////////////////////////////         calculate();
      ////// moved to init, no information yet here..      setupModels();
            // register with the correl surface in case a CDO engine or similar is registered with us.(and a lazyObject with the engine)
            registerWith(correlTS);///// check it has to be a handle etcc.......

            // there should be an update since we r regtrd with the base basket. In the updt one should recreate the bkts and def-loss-models.
        }

        void update() {
            // update basket first ('calculate')
     /////////////////////       DefaultLossModel::update();
            setupModels();
            //notifyObservers();
            DefaultLossModel::update();
        }

       //////// void initialize(const Basket& basket) {
        void setupBasket(const boost::shared_ptr<Basket>& basket) {
            basket_ = basket;
            remainingNotional_ = basket->remainingNotional();
            attachRatio_ = basket->remainingAttachmentAmount()/remainingNotional_;
            detachRatio_ = basket->remainingDetachmentAmount()/remainingNotional_;

             basketAttach_ = boost::shared_ptr<Basket>(new 
                Basket(basket->refDate(),basket->remainingNames(), 
                         
                         basket->remainingNotionals(), 
                         basket->pool(),// NO REMAINING POOL!!!!! IS THIS OK????????? 
                      /////   basket.remainingDefaultKeys(), 
                         0.0, //0.0000000001, // FIX THIS IN THE LHP!!!!!!!!!
                         attachRatio_,
                         basket->claim()
                         ));
             // unreg, no ciclyc calls and they are recreated in reinitialization from the original basket registration: ADD ANY OTHER REGS IN THE BSKT REG
             basketAttach_->unregisterWith(Settings::instance().evaluationDate());//);
             basketAttach_->unregisterWith(basket->claim());

             basketDetach_ = boost::shared_ptr<Basket>(new 
                Basket(basket->refDate(),basket->remainingNames(), 
                         
                         basket->remainingNotionals(), 
                         basket->pool(),// NO REMAINING POOL!!!!! IS THIS OK????????? 
                      //////   basket.remainingDefaultKeys(), 
                         0.0, //0.0000000001, // FIX THIS IN THE LHP!!!!!!!!!
                         detachRatio_,
                         basket->claim()
                         ));
             // unreg, no ciclyc calls and they are recreated in reinitialization from the original basket registration: ADD ANY OTHER REGS IN THE BSKT REG
             basketDetach_->unregisterWith(Settings::instance().evaluationDate());//.value());
             basketDetach_->unregisterWith(basket->claim());

             setupModels();

            scalarCorrelModelAttach_->setupBasket(basket_);// a copy, is that ok?
            scalarCorrelModelDetach_->setupBasket(basket_);// a copy, is that ok?
             
        }
        
        Real expectedTrancheLoss(const Date& d) const;
    protected:
        /*! Sets up attach/detach models. Gets called on basket update. 
        */
        void setupModels(/*typename const BaseModel_T::copulaType::initTraits& traits*/);// {
            // runtime failure if used but not specialized, a bit redundant, this is a protected method
      //      QL_FAIL("Specialization not provided.");
      //  };//???????????????????????????????????????????????????????!!!!!!!!!!
        ////////////////////////////void setupModels() {
        ////////////////////////////    // runtime failure if used but not specialized, a bit redundant, this is a protected method
        ////////////////////////////    QL_FAIL("Specialization not provided.");
        ////////////////////////////};
        Real recoveryValueImpl(const Date& defaultDate, Size iName, //const std::string& name <- from the Pool
            const std::vector<DefaultProbKey>& defKeys = std::vector<DefaultProbKey>()) const {
                return recoveries_[iName];
        }
    private:
      //  const Basket* basket_;
        boost::shared_ptr<Basket> basket_;

        mutable Real attachRatio_, detachRatio_;
        mutable Real remainingNotional_;

        //! Correlation buffer to pick up values from the surface and trigger calculation.
    //..    Handle<Quote> localCorrelation_;// make it SimpleQuote and get rid of the expensive cast in the algo
        boost::shared_ptr<SimpleQuote> localCorrelation_;

        boost::shared_ptr<Basket> basketAttach_,
            basketDetach_;

        //////////////   const Handle<GaussianLatentModel> copula_;/// gaussian in derived only...
        // just cached for the update method
        mutable std::vector<Real> recoveries_;
        Handle<BaseCorrelationTermStructure<Corr2DInt_T> > correlTS_;

        // Initialization parameters for models copula
        mutable typename BaseModel_T::copulaType::initTraits copulaTraits_;
        // Models of equity baskets.
     ///   boost::shared_ptr<BaseModel_T> attachModel_, detachModel_;// OR BASE TYPE??? and get rid of one templt param????????????????????????????????????
   //...     BaseModel_T* attachModel_; BaseModel_T* detachModel_;
        boost::shared_ptr<BaseModel_T> scalarCorrelModelAttach_;
        boost::shared_ptr<BaseModel_T> scalarCorrelModelDetach_;


 ////       mutable std::ofstream fileDbg_;
    };


// Remember ETL returns the EL on the live part of the basket. 
    template<class LM, class I>
    Real BaseCorrelationLossModel<LM, I>::expectedTrancheLoss(const Date& d) const {
 /////////////////////////       calculate();// compute remaining basket

        Real K1 = attachRatio_; //remainingBasket_.attachmentRatio();
        Real K2 = detachRatio_; //remainingBasket_.detachmentRatio();

        Real correlK1 = correlTS_->correlation(d, K1);// this assumes that the given BC surface referes to the current portfolio, i.e. the one with the current defaults in place. If the BC surface is a proxy/refers-to-bespoke-prtflio then this is wrong.....?
        Real correlK2 = correlTS_->correlation(d, K2);

        // reset correl and call base models which have the different baskets associated.
        //boost::dynamic_pointer_cast<SimpleQuote>(
        //    localCorrelation_.currentLink())->setValue(correlK1);
        localCorrelation_->setValue(correlK1);
        Real expLossK1 = 
            // attachModel_->expectedTrancheLoss(d);// SHOULD BE ASKING THE BASKET NOW, NOT THE MODEL!!
            basketAttach_->expectedTrancheLoss(d);
        //boost::dynamic_pointer_cast<SimpleQuote>(
        //    localCorrelation_.currentLink())->setValue(correlK2);
        localCorrelation_->setValue(correlK2);
        Real expLossK2 = 
            //detachModel_->expectedTrancheLoss(d);// SHOULD BE ASKING THE BASKET NOW, NOT THE MODEL!!
            basketDetach_->expectedTrancheLoss(d);

        //fileDbg_
        //    << d  << " ** " 
        //    << correlK1  << " ** " 
        //    << correlK2  << " ** " 
        //    << expLossK1 << " ** "  
        //    << expLossK2
        //    << std::endl;

 //       return (K2*expLossK2 -K1*expLossK1)/(K2-K1);
        return expLossK2 - expLossK1;

        Real tranche_K1K2_SurvProb = 
            1. - (expLossK2 - expLossK1) / 
         ////   ((K2-K1)*remainingBasket_.basketNotional());
            ((K2-K1)*basket_->basketNotional());// IS THIS WHAT I WANT TO DO?????????
        // EL K1-K2 :
        return (1.-tranche_K1K2_SurvProb) * 
            remainingNotional_;
////            remainingBasket_.trancheNotional(); // 3percent error
    }









    // spezs (WHY NOT PARTIAL SPEZ LEAVING THE INTERPOLATOR OPEN????????????????????????????)

    void BaseCorrelationLossModel<GaussianLHPLossModel, BilinearInterpolation>::setupModels() 
    {
        // ... Recreate the baskets.......important not to use 'remaining' the local models would do that. THIS IS NOT TRUE ANYMORE
        ////Handle<Basket_N> basketAttach(boost::shared_ptr<Basket_N>(new 
        ////    Basket_N(basket_->names(), basket_->notionals(), basket_->pool(), 
        ////             basket_->defaultKeys(), 0.0000000001// FIX THIS IN THE LHP!!!!!!!!!
        ////             , basket_->attachmentRatio(),
        ////             basket_->claim())));
/*
        Handle<Basket_N> basketAttach(boost::shared_ptr<Basket_N>(new 
            Basket_N(basket_->remainingNames(), 
                     basket_->notionals(), 
                     basket_->pool(), 
                     basket_->defaultKeys(), 
                     0.0000000001, // FIX THIS IN THE LHP!!!!!!!!!
                     basket_->attachmentRatio(),
                     basket_->claim()
                     )));

        Handle<Basket_N> basketDetach(boost::shared_ptr<Basket_N>(new 
            Basket_N(basket_->names(), basket_->notionals(), basket_->pool(), 
                     basket_->defaultKeys(), 0.0000000001// FIX THIS IN THE LHP!!!!!!!!!
                     , basket_->detachmentRatio(),  
                     basket_->claim())));
 */
        // ... Create the model in a way that it is linked to the local correl handle...
        //attachModel_(basketAttach, refDate_, 
        //    Handle<Quote>(localCorrelation_), recoveries_);
        //detachModel_(basketDetach, refDate_, 
        //    Handle<Quote>(localCorrelation_), recoveries_);


        // on this assignment any previous registration with the attach and detach baskets should be removed
        scalarCorrelModelAttach_ = boost::shared_ptr<GaussianLHPLossModel>(new 
            GaussianLHPLossModel(//////basketAttach, refDate_, 
            Handle<Quote>(localCorrelation_), recoveries_));
        scalarCorrelModelDetach_ = boost::shared_ptr<GaussianLHPLossModel>(new 
            GaussianLHPLossModel(//////basketAttach, refDate_, 
            Handle<Quote>(localCorrelation_), recoveries_));

        /*
        detachModel_ = boost::shared_ptr<GaussianLHPLossModel>(new 
            GaussianLHPLossModel(///////basketDetach, refDate_, 
            Handle<Quote>(localCorrelation_), recoveries_));
            */
        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);
        scalarCorrelModelAttach_->registerWith(basketAttach_);
        scalarCorrelModelDetach_->registerWith(basketDetach_);
    }

    void BaseCorrelationLossModel<GaussianBinomialLossModel, 
        BilinearInterpolation>::setupModels() 
    {
        boost::shared_ptr<GaussianConstantLossLM> lm = 
            boost::make_shared<GaussianConstantLossLM>(
                Handle<Quote>(localCorrelation_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size());
        scalarCorrelModelAttach_ = 
            boost::make_shared<GaussianBinomialLossModel>(lm);
        scalarCorrelModelDetach_ = 
            boost::make_shared<GaussianBinomialLossModel>(lm);
            
        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);
        ////////scalarCorrelModelAttach_->registerWith(basketAttach_);
        ////////scalarCorrelModelDetach_->registerWith(basketDetach_);

    }

    void BaseCorrelationLossModel<TBinomialLossModel, 
        BilinearInterpolation>::setupModels() 
    {
        boost::shared_ptr<TConstantLossLM> lm = 
            boost::make_shared<TConstantLossLM>(
                Handle<Quote>(localCorrelation_), recoveries_, 
                LatentModelIntegrationType::GaussianQuadrature, 
                recoveries_.size(), copulaTraits_);

        scalarCorrelModelAttach_ = 
            boost::make_shared<TBinomialLossModel>(lm);
        scalarCorrelModelDetach_ = 
            boost::make_shared<TBinomialLossModel>(lm);
            
        basketAttach_->setLossModel(scalarCorrelModelAttach_);
        basketDetach_->setLossModel(scalarCorrelModelDetach_);
        ////////scalarCorrelModelAttach_->registerWith(basketAttach_);
        ////////scalarCorrelModelDetach_->registerWith(basketDetach_);
    }


////NEEDS REVISION WITH NEW CODE////////////////
//    void BaseCorrelationLossModel<BinomialConstantRRModel, BilinearInterpolation>::setupModels() 
//    {
//        // ... Recreate the baskets.......important not to use 'remaining' the local models would do that.
//        /*
//        Handle<Basket_N> basketAttach(boost::shared_ptr<Basket_N>(new 
//            Basket_N(basket_->names(), basket_->notionals(), basket_->pool(), 
//                     basket_->defaultKeys(), 0., basket_->attachmentRatio(),
//                     basket_->claim())));
//        Handle<Basket_N> basketDetach(boost::shared_ptr<Basket_N>(new 
//            Basket_N(basket_->names(), basket_->notionals(), basket_->pool(), 
//                     basket_->defaultKeys(), 0., basket_->detachmentRatio(),  
//                     basket_->claim())));
//*/
//        
//        // Interesting here: I could have a latent model dependent on a multifactor correl structure rather than on a single beta. For this I would need some interpolation policy on multidim correls/betas. Study whether the scheme is able to hold this.
//        //
//        //---NOTE: THIS IS NOT VALID ANYMORE I HAVE REMOVED THE CORREL QUOTE CONSTRUCTOR IN THE LATENT MODEL------------
//        /////////Handle<GaussianLatentModel> 
//
//        boost::shared_ptr<GaussianLatentModel> defaultCopula( new 
//            GaussianLatentModel(std::sqrt(localCorrelation_->value()), basket_->pool(), 25));
//
//        boost::shared_ptr<GaussianJointKRRDefaultLM>
//            latentModel( new GaussianJointKRRDefaultLM(
//                defaultCopula, recoveries_, 25));
//
//
///*
//
//            new 
//            /// SEE NOTE:-------- GaussianLatentModel(Handle<Quote>(localCorrelation_), 
//            GaussianLatentModel(std::sqrt(localCorrelation_->value()), basket_->pool(), //<<< Actually it  should be the remaining pool.... but now, as long as they are indexed by string...
//              //  basket_->remainingSize(), 
//                25 // quadrature
//                ))  */
//                /*)*/;
///*
//        // ... Create the model in a way that it is linked to the local correl handle...
//        attachModel_ = boost::shared_ptr<BinomialConstantRRModel>(new 
//            BinomialConstantRRModel(latentModel, basketAttach, refDate_, 
//            recoveries_));
//        detachModel_ = boost::shared_ptr<BinomialConstantRRModel>(new 
//            BinomialConstantRRModel(latentModel, basketDetach, refDate_, 
//            recoveries_));
//*/
//        //////scalarCorrelModel_ = boost::shared_ptr<BinomialConstantRRModel>(new 
//        //////    BinomialConstantRRModel(latentModel///, ////basketAttach, refDate_, 
//        //////   /// 
//        //////    ));
//        scalarCorrelModel_ = boost::shared_ptr<BinomialConstantRRModel>(new 
//            BinomialConstantRRModel(defaultCopula, recoveries_));
////////    }


    typedef BaseCorrelationLossModel<GaussianLHPLossModel, 
                BilinearInterpolation> GaussianLHPFlatBCLM;


}

#endif
