/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file markovfunctional.hpp
    \brief Markov Functional 1 Factor Model
*/

// uncomment to enable NTL support
//#define MF_ENABLE_NTL 

#ifndef quantlib_markovfunctional_hpp
#define quantlib_markovfunctional_hpp

#include <ql/mathconstants.hpp>
#include <ql/models/model.hpp>
#include <ql/models/parameter.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/date.hpp>
#include <ql/time/period.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/utilities/null.hpp>
#include <ql/patterns/lazyobject.hpp>

#include <ql/experimental/models/mfstateprocess.hpp>
#include <ql/experimental/models/kahalesmilesection.hpp>
#include <ql/experimental/models/atmadjustedsmilesection.hpp>
#include <ql/experimental/models/atmsmilesection.hpp>

#ifdef MF_ENABLE_NTL
    #include <boost/math/bindings/rr.hpp>
#endif

namespace QuantLib {

    //! One factor Markov Functional model class. Some documentation is available here http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2183721
    /*! \warning The state variable $y$ is the standardized version of the original state variable $x$, i.e. $y(t) = ( \int_0^t (exp(at)\sigma(u))^2 du )^{-1/2} x(t)$.
        \warning In deflatedZerobond() it is assumed that the variance of the state process $x$ is not dependent on $x$. If you want to use a state process with local variance
                 then you will have to adjust deflatedZerobond() accordingly.
        \warning If you use the KahaleExtrapolation for the smile pretreatment then this implies zero density for negative underlying rates. This means that in this case the market
                 yield term structure must imply positive underlying atm forward rates. In principle the mf model is able to produce negative rates. To make this work the smileSection
                 provided as input must have an digitalOptionPrice (or an optionPrice) implementation that is consistent with such a yield term structure and the model setting lowerRateBound
                 must be set appropriately as a lower limit for the underlying rates.
        \warning If you do not use a smile pretreatment you should ensure that the input smileSection is arbitrage free
        \warning If you do not use a smile pretreatment you should ensure that the input smileSection covers the strikes from lowerRateBound to upperRateBound
        \warning The Kahale pretreatment only checks arbitrage freeness on a discrete strike grid and extrapolates an arbitrage free C^1 call price function outside the identified boundaries.
                 Inside these boundaries the provided smile should interpolate arbitrage free and smooth (C^2 or at least C^1 call price function).
        \warning The model assumes a monocurve setup. Workarounds for spreads between different forward curves or forward and discounting curve are at the moment left to be implemented
                 in pricing engines. Note that this implies slightly different atm forward levels compared to a multicurve setup with e.g. OIS discounting and Ibor forwarding curves.
                 The volatility structures feeded into the model should therefore be consistent with the monocurve assumption of the model.
        \warning The model uses a simplified formula for the npv of a swaps floating leg namely $P(t,T_0)-P(t,T_1)$ with $T_0$ being the start date of the leg and $T_1% being the last payment date,
                 which is an approximation to the true npv.
        \warning The model calibrates to slightly modified market options in the sense that the start date is set equal to the fixing date, i.e. there is not delay. The model outputs
                 refer to this modified instrument. This modification can be switched on and off in the models pricing functions via the flat zeroFixingDays. In general the actual market
                 instrument including the delay is still matched very well though the calibration is done on a slightly different instrument in fact.
        \warning AdjustYts and AdjustDigitals are experimental options. Specifying AdjustYts may have a negative impact on the volatility smile match, so it should be used with special care.
                 For long term calibration it seems an interesting option though.
        \warning NTL support must be enabled by defining MF_ENABLE_NTL in this file. For details on NTL see http://www.shoup.net/ntl/
    */

    class MarkovFunctional : public TermStructureConsistentModel, public CalibratedModel, public LazyObject  {

      public:

        struct ModelSettings {

            enum SmilePretreatment { NoPretreatment, KahaleExtrapolation };
            
            // NoPayoffExtrapolation overrides ExtrapolatePayoffFlat
            enum Adjustments { AdjustNone = 0, AdjustDigitals = 1<<0, AdjustYts = 1<<1, ExtrapolatePayoffFlat = 1<<2, NoPayoffExtrapolation = 1<<3 };

            ModelSettings() : yGridPoints_(64), yStdDevs_(7.0), gaussHermitePoints_(32), digitalGap_(1E-5), marketRateAccuracy_(1E-7),
                upperRateBound_(2.0), lowerRateBound_(0.0), smilePretreatment_(KahaleExtrapolation), adjustments_(AdjustNone),
                smileMoneynessCheckpoints_(std::vector<Real>()), enableNtl_(false) {}
            
            void validate() const {
                QL_REQUIRE(yGridPoints_>0,"At least one grid point (" << yGridPoints_ << ") for the state process discretization must be given");
                QL_REQUIRE(yStdDevs_>0.0,"Multiple of standard deviations covered by state process discretization (" << yStdDevs_ << ") must be positive");
                QL_REQUIRE(gaussHermitePoints_>0,"Number of gauss hermite integration points (" << gaussHermitePoints_ << ") must be positive");
                QL_REQUIRE(digitalGap_>0.0,"Digital gap (" << digitalGap_ << ") must be positive");
                QL_REQUIRE(marketRateAccuracy_>0.0,"Market rate accuracy (" << marketRateAccuracy_ << ") must be positive");
                QL_REQUIRE(smilePretreatment_ != KahaleExtrapolation || lowerRateBound_ == 0.0,"If Kahale extrapolation is used, the lower rate bound (" << lowerRateBound_ << ") must be zero.");
                QL_REQUIRE(lowerRateBound_ < upperRateBound_,"Lower rate bound (" << lowerRateBound_ << ") must be strictly less than upper rate bound (" << upperRateBound_ << ")");
                #ifndef MF_ENABLE_NTL
                    QL_REQUIRE(!enableNtl_,"High precision computation using NTL can not be enabled since MF_ENABLE_NTL is not defined.");
                #endif
            }

            ModelSettings& withYGridPoints(Size n) { yGridPoints_ = n; return *this; }
            ModelSettings& withYStdDevs(Real s) { yStdDevs_ = s; return *this; }
            ModelSettings& withGaussHermitePoints(Size n) { gaussHermitePoints_ = n; return *this; }
            ModelSettings& withDigitalGap(Real d) { digitalGap_ = d; return *this; }
            ModelSettings& withMarketRateAccuracy(Real a) { marketRateAccuracy_ = a; return *this; }
            ModelSettings& withUpperRateBound(Real u) { upperRateBound_ = u; return *this; }
            ModelSettings& withLowerRateBound(Real l) { lowerRateBound_ = l; return *this; }
            ModelSettings& withSmilePretreatment(SmilePretreatment s) { smilePretreatment_ = s; return *this; }
            ModelSettings& withAdjustments(int a) { adjustments_ = a; return *this; }
            ModelSettings& withSmileMoneynessCheckpoints(std::vector<Real> m) { smileMoneynessCheckpoints_ = m; return *this; }
            ModelSettings& withEnableNtl(bool e) { enableNtl_ = e; return *this; }

            Size yGridPoints_;
            Real yStdDevs_;
            Size gaussHermitePoints_;
            Real digitalGap_, marketRateAccuracy_;
            Real lowerRateBound_,upperRateBound_;
            SmilePretreatment smilePretreatment_;
            int adjustments_;
            std::vector<Real> smileMoneynessCheckpoints_;
            bool enableNtl_;
        };

        struct CalibrationPoint {
            bool isCaplet_;
            Period tenor_;
            std::vector<Date> paymentDates_;
            std::vector<Real> yearFractions_;
            Real atm_;
            Real annuity_;
            boost::shared_ptr<SmileSection> smileSection_;
            boost::shared_ptr<SmileSection> rawSmileSection_;
            Real minRateDigital_;
            Real maxRateDigital_;
        };

        // utility macro to write messages to the model outputs

#define QL_MFMESSAGE(o,message) \
        {std::ostringstream os; \
        os << message; \
        o.messages_.push_back(os.str());}

        struct ModelOutputs {
            bool dirty_;
            ModelSettings settings_;
            std::vector<Date> expiries_;
            std::vector<Period> tenors_;
            std::vector<Real> atm_;
            std::vector<Real> annuity_;
            std::vector<Real> adjustmentFactors_;
            std::vector<Real> digitalsAdjustmentFactors_;
            std::vector<std::string> messages_;
            std::vector<std::vector<Real> > smileStrikes_;
            std::vector<std::vector<Real> > marketRawCallPremium_; // premium w.r.t. original smile section without pretreatment, but with atm adjustment if AdjustAtmSmile is active
            std::vector<std::vector<Real> > marketRawPutPremium_;
            std::vector<std::vector<Real> > marketCallPremium_;
            std::vector<std::vector<Real> > marketPutPremium_;
            std::vector<std::vector<Real> > modelCallPremium_;
            std::vector<std::vector<Real> > modelPutPremium_;
            std::vector<std::vector<Real> > marketVega_;
            std::vector<Real> marketZerorate_;
            std::vector<Real> modelZerorate_;
        };

        // Constructor for a swaption smile calibrated model
        MarkovFunctional(const Handle<YieldTermStructure>& termStructure,
                        const Real reversion,
                        const std::vector<Date>& volstepdates,
                        const std::vector<Real>& volatilities,
                        const Handle<SwaptionVolatilityStructure>& swaptionVol,
                        const std::vector<Date>& swaptionExpiries,
                        const std::vector<Period>& swaptionTenors,
                        const boost::shared_ptr<SwapIndex>& swapIndexBase,
                        const MarkovFunctional::ModelSettings& modelSettings = ModelSettings());

        // Constructor for a caplet smile calibrated model
        MarkovFunctional(const Handle<YieldTermStructure>& termStructure,
                        const Real reversion,
                        const std::vector<Date>& volstepdates,
                        const std::vector<Real>& volatilities,
                        const Handle<OptionletVolatilityStructure>& capletVol,
                        const std::vector<Date>& capletExpiries,
                        const boost::shared_ptr<IborIndex>& iborIndex,
                        const MarkovFunctional::ModelSettings& modelSettings = ModelSettings());

        const ModelSettings& modelSettings() const { return modelSettings_; }
        const ModelOutputs& modelOutputs() const;

        const Date& numeraireDate() const { return numeraireDate_; }
        const Time& numeraireTime() const { return numeraireTime_; }

        const boost::shared_ptr<StochasticProcess1D> stateProcess() const { return stateProcess_; }

        const Real numeraire(const Time t, const Real y=0.0) const;
        const Disposable<Array> numeraire(const Time t, const Array& y) const;
        const Real deflatedZerobond(const Time T, const Time t=0.0, const Real y=0.0) const;
        const Disposable<Array> deflatedZerobond(const Time T, const Time t, const Array& y) const;

        const Real zerobond(const Time T, const Time t=0.0, const Real y=0.0) const;
        const Real zerobond(const Date& maturity, const Date& referenceDate = Null<Date>(), const Real y=0.0) const;

        const Real zerobondOption(const Option::Type& type, const Date& expiry, const Date& maturity, const Rate strike, const Date& referenceDate = Null<Date>(), const Real y=0.0) const;

        const Real forwardRate(const Date& fixing, const Date& referenceDate = Null<Date>(), const Real y=0.0,const bool zeroFixingDays=false, boost::shared_ptr<IborIndex> iborIdx = boost::shared_ptr<IborIndex>()) const;
        const Real swapRate(const Date& fixing, const Period& tenor, const Date& referenceDate = Null<Date>(), const Real y=0.0,const bool zeroFixingDays=false, boost::shared_ptr<SwapIndex> swapIdx = boost::shared_ptr<SwapIndex>()) const;
        const Real swapAnnuity(const Date& fixing, const Period& tenor, const Date& referenceDate = Null<Date>(), const Real y=0.0,const bool zeroFixingDays=false, boost::shared_ptr<SwapIndex> swapIdx = boost::shared_ptr<SwapIndex>()) const;

        const Real capletPrice(const Option::Type& type, const Date& expiry, const Rate strike, const Date& referenceDate = Null<Date>(), const Real y=0.0, const bool zeroFixingDays=false, boost::shared_ptr<IborIndex> iborIdx = boost::shared_ptr<IborIndex>()) const;
        const Real swaptionPrice(const Option::Type& type, const Date& expiry, const Period& tenor, const Rate strike, const Date& referenceDate = Null<Date>(), const Real y=0.0, const bool zeroFixingDays=false, boost::shared_ptr<SwapIndex> swapIdx = boost::shared_ptr<SwapIndex>()) const;

        /*! Computes the integral
        \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
        with
        \f[ p(x) = ax^4+bx^3+cx^2+dx+e \f].
        */
        const Real gaussianPolynomialIntegral(const Real a, const Real b, const Real c, const Real d, const Real e, const Real x0, const Real x1) const;
        
        /*! Computes the integral
        \f[ {2\pi}^{-0.5} \int_{a}^{b} p(x) \exp{-0.5*x*x} \mathrm{d}x \f]
        with
        \f[ p(x) = a(x-h)^4+b(x-h)^3+c(x-h)^2+d(x-h)+e \f].
        */
        const Real gaussianShiftedPolynomialIntegral(const Real a, const Real b, const Real c, const Real d, const Real e, const Real h, const Real x0, const Real x1) const;

        const Disposable<Array> yGrid(const Real yStdDevs, const int gridPoints, const Real T=1.0, const Real t=0, const Real y=0) const;

        void update() {
            LazyObject::update();
        }

        void performCalculations() const {
            updateSmiles();
            updateNumeraireTabulation();
        }

      protected:
        
        void generateArguments() {
            calculate();
            updateNumeraireTabulation();
            notifyObservers();
        }

      private:

        void initialize();
        void updateSmiles() const;
        void updateNumeraireTabulation() const;

        void makeSwaptionCalibrationPoint(const Date& expiry, const Period& tenor);
        void makeCapletCalibrationPoint(const Date& expiry);
        
        const Real marketSwapRate(const Date& expiry, const CalibrationPoint& p, const Real digitalPrice, const Real guess = 0.03) const;
        const Real marketDigitalPrice(const Date& expiry, const CalibrationPoint& p, const Option::Type& type, const Real strike) const;

        class ZeroHelper;
        friend class ZeroHelper;
        class ZeroHelper {
            public:
            ZeroHelper(const MarkovFunctional *model,
                       const Date& expiry,
                       const CalibrationPoint& p,
                       const Real marketPrice)
            : model_(model), expiry_(expiry), p_(p), marketPrice_(marketPrice) {}
            double operator()(double strike) const {
                Real modelPrice = model_->marketDigitalPrice(expiry_,p_,Option::Call,strike);
                return modelPrice-marketPrice_;
            };
            const MarkovFunctional *model_;
            const Real marketPrice_;
            const Date& expiry_;
            const CalibrationPoint& p_;
        };

        ModelSettings modelSettings_;
        mutable ModelOutputs modelOutputs_;

        const bool capletCalibrated_;

        boost::shared_ptr<StochasticProcess1D> stateProcess_;

        boost::shared_ptr<Matrix> discreteNumeraire_;
        std::vector<boost::shared_ptr<Interpolation> > numeraire_; // vector of interpolated numeraires in y direction for all calibration times

        Parameter reversion_;
        Parameter& sigma_;
        
        std::vector<Date> volstepdates_;
        std::vector<Time> volsteptimes_;
        Array volsteptimesArray_; // FIXME this is redundant (just a copy of volsteptimes_)
        std::vector<Real> volatilities_;

        Date numeraireDate_;
        Time numeraireTime_;

        Handle<SwaptionVolatilityStructure> swaptionVol_;
        Handle<OptionletVolatilityStructure> capletVol_;

        std::vector<Date> swaptionExpiries_, capletExpiries_;
        std::vector<Period> swaptionTenors_;
        boost::shared_ptr<SwapIndex> swapIndexBase_;
        boost::shared_ptr<IborIndex> iborIndex_;

        mutable std::map<Date,CalibrationPoint> calibrationPoints_;
        std::vector<Real> times_;
        Array y_;

        Array normalIntegralX_;
        Array normalIntegralW_;
        
    };

    std::ostream& operator<<(std::ostream& out, const MarkovFunctional::ModelOutputs& m);

}


#endif

