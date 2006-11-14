/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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

#include <ql/Volatilities/swaptionvolmatrix.hpp>
#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Volatilities/swaptionvolcubebysabr.hpp>
#include <ql/CashFlows/cmscoupon.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/Optimization/method.hpp>
#include <ql/Optimization/problem.hpp>
#include <ql/Optimization/conjugategradient.hpp>
#include <ql/Optimization/simplex.hpp>

namespace QuantLib {

     typedef std::vector<boost::shared_ptr<CashFlow> > Leg;

     class CmsMarket{
      public:

        CmsMarket(
            const std::vector<Period>& expiries,
            const std::vector< boost::shared_ptr<SwapIndex> >& swapIndices,
            const std::vector<std::vector<Handle<Quote> > >& bidAskSpreads,
            const Matrix& meanReversions,
            const boost::shared_ptr<VanillaCMSCouponPricer>& pricer,
            const Handle<YieldTermStructure>& yieldTermStructure,
            const Handle<SwaptionVolatilityStructure>& volStructure);

        void createForwardStartingCms();
        void reprice(const Handle<SwaptionVolatilityStructure>& volStructure,
                     Real meanReversion);
        Real weightedError(const Matrix& weights);
        Real weightedPriceError(const Matrix& weights);
        Real weightedForwardPriceError(const Matrix& weights);
        
        const std::vector<Period>& swapTenors() const {
                return swapTenors_;
            }
        Matrix meanReversions(){return meanReversions_;};
        Matrix impliedCmsSpreads(){return modelCmsSpreads_;};
        Matrix spreadErrors(){return spreadErrors_;};
        Matrix browse() const;
 
      private:

        std::vector<Period> expiries_;
        std::vector<Period> swapTenors_;
        Size nExercise_;
        Size nSwapTenors_;

        // market bid spreads
        Matrix bids_;
        // market ask spreads
        Matrix asks_;
        // market mid spreads
        Matrix mids_;
        // Implied spreads to model prices
        Matrix modelCmsSpreads_;
        // Differences between implied and mid spreads 
        Matrix spreadErrors_;
        
        // prices of constant maturity swaps with spread = 0
        Matrix prices_;           
        // market prices of Cms Leg corrisponding to bid spreads
        Matrix marketBidCmsLegValues_;
        // market prices of Cms Leg corrisponding to ask spreads
        Matrix marketAskCmsLegValues_;
        // market prices of Cms Leg corrisponding to mid spreads
        Matrix marketMidCmsLegValues_;
        // model prices of Cms Leg corrisponding to mid spreads
        Matrix modelCmsLegValues_;
        // Differences between modelCmsLegValue and marketMidCmsLegValue_ 
        Matrix priceErrors_;


        // market prices of Forward Cms Leg corrisponding to bid spreads
        Matrix marketBidForwardCmsLegValues_; 
        // market prices of Forward Cms Leg corrisponding to ask spreads
        Matrix marketAskForwardCmsLegValues_;
        // market prices of Forward Cms Leg corrisponding to mid spreads
        Matrix marketMidForwardCmsLegValues_;
        // model prices of Forward Cms Leg corrisponding to mid spreads
        Matrix modelForwardCmsLegValues_;
        // Differences between modelForwardCmsLegValues and marketMidCmsLegValues 
        Matrix forwardPriceErrors_;



        Matrix meanReversions_;
        boost::shared_ptr<VanillaCMSCouponPricer> pricer_;
        std::vector< boost::shared_ptr<SwapIndex> > swapIndices_;

        std::vector< std::vector< boost::shared_ptr<Swap> > > swaps_;

        Handle<YieldTermStructure> yieldTermStructure_;
        Handle<SwaptionVolatilityStructure> volStructure_;
 
     };

     class SmileAndCmsCalibrationBySabr{
        
      public:
        
        enum CalibrationType {OnSpread, OnPrice, OnForwardCmsPrice };

        SmileAndCmsCalibrationBySabr(
            Handle<SwaptionVolatilityStructure>& volCube,
            boost::shared_ptr<CmsMarket>& cmsMarket,
            const Matrix& weights,
            CalibrationType calibrationType);
        
        Handle<SwaptionVolatilityStructure> volCube_;
        boost::shared_ptr<CmsMarket> cmsMarket_;
        Matrix weights_;
        CalibrationType calibrationType_;

        Array calibration();
        Real error(){return error_;};
        EndCriteria::Type endCriteria(){ return endCriteria_; };

      private:

        class ParametersConstraint : public Constraint {
              private:
                class Impl : public Constraint::Impl {
                  public:
                    bool test(const Array& params) const {
                        return params[0]>=0.0 && params[0]<=1.0 // beta
                            && params[1]>0.0 && params[1]<2.0;   // mean reversion
                    }
                };
              public:
                ParametersConstraint()
                : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl)) {}
            };
  
        class ObjectiveFunction : public CostFunction {
          public:
            ObjectiveFunction(SmileAndCmsCalibrationBySabr* smileAndCms)
                :smileAndCms_(smileAndCms),
                volCube_(smileAndCms->volCube_),
                cmsMarket_(smileAndCms->cmsMarket_),
                weights_(smileAndCms->weights_),
                calibrationType_(smileAndCms->calibrationType_){};

                Real value(const Array& x) const;
          private:
            SmileAndCmsCalibrationBySabr* smileAndCms_;
            Handle<SwaptionVolatilityStructure> volCube_;
            boost::shared_ptr<CmsMarket> cmsMarket_;
            Matrix weights_;
            CalibrationType calibrationType_;
        };
        Real error_; 
		EndCriteria::Type endCriteria_;
  
    };

}

#endif
