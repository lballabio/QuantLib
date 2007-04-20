/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2006 2007 Giorgio Facchinetti

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

/*! \file cmsmarket.hpp
*/

#ifndef quantlib_cms_market_h
#define quantlib_cms_market_h

#include <ql/termstructures/volatilities/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatilities/swaptionvolcube1.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/indexes/euribor.hpp>
#include <ql/math/optimization/conjugategradient.hpp>
#include <ql/math/optimization/simplex.hpp>

namespace QuantLib {

     typedef Leg Leg;
     
     class CmsCouponPricer;

     class CmsMarket: public LazyObject{
      public:

        CmsMarket(
            const std::vector<Period>& expiries,
            const std::vector< boost::shared_ptr<SwapIndex> >& swapIndices,
            const std::vector<std::vector<Handle<Quote> > >& bidAskSpreads,
            const std::vector< boost::shared_ptr<CmsCouponPricer> >& pricers,
            const Handle<YieldTermStructure>& yieldTermStructure);

        //! \name LazyObject interface
        //@{
        void update() { LazyObject::update();}
        //@}

        // call during calibration procedure 
        void reprice(const Handle<SwaptionVolatilityStructure>& volStructure,
                     Real meanReversion);
		
        //inspectors ...
        const std::vector<Period>& swapTenors() const {return swapTenors_;}
        Matrix meanReversions(){return meanReversions_;};
        Matrix impliedCmsSpreads(){return modelCmsSpreads_;};
        Matrix spreadErrors(){return spreadErrors_;};
        Matrix browse() const;

        //cms market calibration methods (they haven't Lazyness behaviour)
		Real weightedError(const Matrix& weights);
        Real weightedPriceError(const Matrix& weights);
        Real weightedForwardPriceError(const Matrix& weights);
        Disposable<Array> weightedErrors(const Matrix& weights);
        Disposable<Array> weightedPriceErrors(const Matrix& weights);
        Disposable<Array> weightedForwardPriceErrors(const Matrix& weights);
 
      private:
		void performCalculations() const;
		void registerWithMarketData();
		void createForwardStartingCms();
		void priceForwardStartingCms() const;
        void priceSpotFromForwardStartingCms() const;
        Real weightedMean(const Matrix& var, const Matrix& weights);
        Disposable<Array> weightedMeans(const Matrix& var, const Matrix& weights);

        std::vector<Period> expiries_;
        std::vector<Period> swapTenors_;
        Size nExercise_;
        Size nSwapTenors_;

        // market bid spreads
        mutable Matrix bids_;
        // market ask spreads
        mutable Matrix asks_;
        // market mid spreads
        mutable Matrix mids_;
        // Implied spreads to model prices
        mutable Matrix modelCmsSpreads_;
        // Differences between implied and mid spreads 
        mutable Matrix spreadErrors_;
        
        // prices of constant maturity swaps with spread = 0
        mutable Matrix prices_;           
        // market prices of Cms Leg corrisponding to bid spreads
        mutable Matrix marketBidCmsLegValues_;
        // market prices of Cms Leg corrisponding to ask spreads
        mutable Matrix marketAskCmsLegValues_;
        // market prices of Cms Leg corrisponding to mid spreads
        mutable Matrix marketMidCmsLegValues_;
        // model prices of Cms Leg corrisponding to mid spreads
        mutable Matrix modelCmsLegValues_;
        // Differences between modelCmsLegValue and marketMidCmsLegValue_ 
        mutable Matrix priceErrors_;

		mutable Matrix swapFloatingLegsPrices_,swapFloatingLegsBps_;

        // market prices of Forward Cms Leg corrisponding to bid spreads
        mutable Matrix marketBidForwardCmsLegValues_; 
        // market prices of Forward Cms Leg corrisponding to ask spreads
        mutable Matrix marketAskForwardCmsLegValues_;
        // market prices of Forward Cms Leg corrisponding to mid spreads
        mutable Matrix marketMidForwardCmsLegValues_;
        // model prices of Forward Cms Leg corrisponding to mid spreads
        mutable Matrix modelForwardCmsLegValues_;
        // Differences between modelForwardCmsLegValues and marketMidCmsLegValues 
        mutable Matrix forwardPriceErrors_;

        mutable Matrix meanReversions_;
        std::vector< boost::shared_ptr<CmsCouponPricer> > pricers_;
        std::vector< boost::shared_ptr<SwapIndex> > swapIndices_;
        const std::vector<std::vector<Handle<Quote> > > bidAskSpreads_;
        std::vector< std::vector< boost::shared_ptr<Swap> > > swaps_;
        std::vector< std::vector< boost::shared_ptr<Swap> > > forwardSwaps_;
        Handle<YieldTermStructure> yieldTermStructure_;
     };

     class CmsMarketCalibration{
        
      public:
        
        enum CalibrationType {OnSpread, OnPrice, OnForwardCmsPrice };

        CmsMarketCalibration(
            Handle<SwaptionVolatilityStructure>& volCube,
            boost::shared_ptr<CmsMarket>& cmsMarket,
            const Matrix& weights,
            CalibrationType calibrationType);
        
        Handle<SwaptionVolatilityStructure> volCube_;
        boost::shared_ptr<CmsMarket> cmsMarket_;
        Matrix weights_;
        CalibrationType calibrationType_;
        Matrix sparseSabrParameters_, denseSabrParameters_, browseCmsMarket_;

        Array compute(const boost::shared_ptr<EndCriteria>& endCriteria,
                      const boost::shared_ptr<OptimizationMethod>& method,
                      const Array& guess,
                      bool isMeanReversionFixed);
        Real error(){return error_;}
        Real elapsed() {return elapsed_;}
        EndCriteria::Type endCriteria() { return endCriteria_; };

      private:

        class ParametersConstraint : public Constraint {
              private:
                class Impl : public Constraint::Impl {
                    Size nBeta_;
                  public:
                    Impl(Size nBeta)
                    : Constraint::Impl(),nBeta_(nBeta){}

                    bool test(const Array& params) const {
                        QL_REQUIRE(params.size()==nBeta_+1,"params.size()!=nBeta_+1");
                        bool areBetasInConstraints = true;
                        for(Size i=0;i<nBeta_;i++)
                            areBetasInConstraints = areBetasInConstraints && (params[i]>=0.0 && params[i]<=1.0);
                        return areBetasInConstraints             // betas
                            && params[nBeta_]>0.0 && params[nBeta_]<2.0;   // mean reversion
                    }
                };
              public:
                ParametersConstraint(Size nBeta)
                : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl(nBeta))) {}
            };

        class ObjectiveFunction : public CostFunction {
          public:
            ObjectiveFunction(CmsMarketCalibration* smileAndCms)
                :smileAndCms_(smileAndCms),
                volCube_(smileAndCms->volCube_),
                cmsMarket_(smileAndCms->cmsMarket_),
                weights_(smileAndCms->weights_),
                calibrationType_(smileAndCms->calibrationType_){};

                Real value(const Array& x) const;
                Disposable<Array> values(const Array& x) const;

          protected:
            Real switchErrorFunctionOnCalibrationType() const;
            Disposable<Array> switchErrorsFunctionOnCalibrationType() const;

            CmsMarketCalibration* smileAndCms_;
            Handle<SwaptionVolatilityStructure> volCube_;
            boost::shared_ptr<CmsMarket> cmsMarket_;
            Matrix weights_;
            CalibrationType calibrationType_;
          private:
            virtual void updateVolatilityCubeAndCmsMarket(const Array& x) const;
        };
 
        class ParametersConstraintWithFixedMeanReversion : public Constraint {
              private:
                class Impl : public Constraint::Impl {
                    Size nBeta_;
                  public:
                    Impl(Size nBeta)
                    : Constraint::Impl(),nBeta_(nBeta){}

                    bool test(const Array& params) const {
                        QL_REQUIRE(params.size()==nBeta_,"params.size()!=nBeta_");
                        bool areBetasInConstraints = true;
                        for(Size i=0;i<nBeta_;i++)
                            areBetasInConstraints = areBetasInConstraints && (params[i]>=0.0 && params[i]<=1.0);
                        return areBetasInConstraints;   
                    }
                };
              public:
                ParametersConstraintWithFixedMeanReversion(Size nBeta)
                : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl(nBeta))) {}
        };

        class ObjectiveFunctionWithFixedMeanReversion : public ObjectiveFunction {
          public:
            ObjectiveFunctionWithFixedMeanReversion(CmsMarketCalibration* smileAndCms,
                                                    Real fixedMeanReversion)
                :ObjectiveFunction(smileAndCms),
                fixedMeanReversion_(fixedMeanReversion){};

          private:
            virtual void updateVolatilityCubeAndCmsMarket(const Array& x) const;
            Real fixedMeanReversion_;
        };

        
        Real error_; 
		EndCriteria::Type endCriteria_;
        Real elapsed_;
    };

}

#endif
