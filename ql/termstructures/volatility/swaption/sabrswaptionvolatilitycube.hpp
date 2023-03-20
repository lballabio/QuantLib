/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Copyright (C) 2014, 2015 Peter Caspers
 Copyright (C) 2023 Ignacio Anguita

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

/*! \file sabrswaptionvolatilitycube.hpp
    \brief Swaption volatility cube, fit-early-interpolate-later approach
           The provided types are
           SabrSwaptionVolatilityCube using the classic Hagan 2002 Sabr formula
           NoArbSabrSwaptionVolatilityCube using the No Arbitrage Sabr model (Doust)
*/

#ifndef quantlib_sabr_swaption_volatility_cube_hpp
#define quantlib_sabr_swaption_volatility_cube_hpp

#include <ql/math/interpolations/backwardflatlinearinterpolation.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/flatextrapolation2d.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <utility>


#ifndef SWAPTIONVOLCUBE_VEGAWEIGHTED_TOL
    #define SWAPTIONVOLCUBE_VEGAWEIGHTED_TOL 15.0e-4
#endif
#ifndef SWAPTIONVOLCUBE_TOL
    #define SWAPTIONVOLCUBE_TOL 100.0e-4
#endif

namespace QuantLib {    

    class Interpolation2D;
    class EndCriteria;
    class OptimizationMethod;

    //! XABR Swaption Volatility Cube
    /*! This class implements the XABR Swaption Volatility Cube
        which is a generic for different SABR, ZABR and 
        different smile models that can be used to instantiate concrete cubes.
    */
    template<class Model>
    class XabrSwaptionVolatilityCube : public SwaptionVolatilityCube {
        class Cube {
          public:
            Cube() = default;
            Cube(const std::vector<Date>& optionDates,
                 const std::vector<Period>& swapTenors,
                 const std::vector<Time>& optionTimes,
                 const std::vector<Time>& swapLengths,
                 Size nLayers,
                 bool extrapolation = true,
                 bool backwardFlat = false);
            Cube& operator=(const Cube& o);
            Cube(const Cube&);
            virtual ~Cube() = default;
            void setElement(Size IndexOfLayer,
                            Size IndexOfRow,
                            Size IndexOfColumn,
                            Real x);
            void setPoints(const std::vector<Matrix>& x);
            void setPoint(const Date& optionDate,
                          const Period& swapTenor,
                          Time optionTime,
                          Time swapLength,
                          const std::vector<Real>& point);
            void setLayer(Size i,
                          const Matrix& x);
            void expandLayers(Size i,
                              bool expandOptionTimes,
                              Size j,
                              bool expandSwapLengths);
            const std::vector<Date>& optionDates() const {
                return optionDates_;
            }
            const std::vector<Period>& swapTenors() const {
                return swapTenors_;
            }
            const std::vector<Time>& optionTimes() const;
            const std::vector<Time>& swapLengths() const;
            const std::vector<Matrix>& points() const;
            std::vector<Real> operator()(Time optionTime, Time swapLengths) const;
            void updateInterpolators()const;
            Matrix browse() const;
          private:
            std::vector<Time> optionTimes_, swapLengths_;
            std::vector<Date> optionDates_;
            std::vector<Period> swapTenors_;
            Size nLayers_;
            std::vector<Matrix> points_;
            mutable std::vector<Matrix> transposedPoints_;
            bool extrapolation_;
            bool backwardFlat_;
            mutable std::vector< ext::shared_ptr<Interpolation2D> > interpolators_;
         };
      public:
        XabrSwaptionVolatilityCube(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& optionTenors,
            const std::vector<Period>& swapTenors,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volSpreads,
            const ext::shared_ptr<SwapIndex>& swapIndexBase,
            const ext::shared_ptr<SwapIndex>& shortSwapIndexBase,
            bool vegaWeightedSmileFit,
            std::vector<std::vector<Handle<Quote> > > parametersGuess,
            std::vector<bool> isParameterFixed,
            bool isAtmCalibrated,
            ext::shared_ptr<EndCriteria> endCriteria = ext::shared_ptr<EndCriteria>(),
            Real maxErrorTolerance = Null<Real>(),
            ext::shared_ptr<OptimizationMethod> optMethod = ext::shared_ptr<OptimizationMethod>(),
            Real errorAccept = Null<Real>(),
            bool useMaxError = false,
            Size maxGuesses = 50,
            bool backwardFlat = false,
            Real cutoffStrike = 0.0001);
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
        //! \name SwaptionVolatilityCube interface
        //@{
        ext::shared_ptr<SmileSection> smileSectionImpl(Time optionTime,
                                                       Time swapLength) const override;
        //@}
        //! \name Other inspectors
        //@{
        const Matrix& marketVolCube(Size i) const {
            return marketVolCube_.points()[i];
        }
        Matrix sparseSabrParameters()const;
        Matrix denseSabrParameters() const;
        Matrix marketVolCube() const;
        Matrix volCubeAtmCalibrated() const;
        //@}
        void sabrCalibrationSection(const Cube& marketVolCube,
                                    Cube& parametersCube,
                                    const Period& swapTenor) const;
        void recalibration(Real beta,
                           const Period& swapTenor);
        void recalibration(const std::vector<Real> &beta,
                           const Period& swapTenor);
        void recalibration(const std::vector<Period> &swapLengths,
                           const std::vector<Real> &beta,
                           const Period& swapTenor);
        void updateAfterRecalibration();
     protected:
        void registerWithParametersGuess();
        void setParameterGuess() const;
        ext::shared_ptr<SmileSection> smileSection(
                                    Time optionTime,
                                    Time swapLength,
                                    const Cube& sabrParametersCube) const;
        Cube sabrCalibration(const Cube &marketVolCube) const;
        void fillVolatilityCube() const;
        void createSparseSmiles() const;
        std::vector<Real> spreadVolInterpolation(const Date& atmOptionDate,
                                                 const Period& atmSwapTenor) const;
      private:
        Size requiredNumberOfStrikes() const override { return 1; }
        mutable Cube marketVolCube_;
        mutable Cube volCubeAtmCalibrated_;
        mutable Cube sparseParameters_;
        mutable Cube denseParameters_;
        mutable std::vector< std::vector<ext::shared_ptr<SmileSection> > >
                                                                sparseSmiles_;
        std::vector<std::vector<Handle<Quote> > > parametersGuessQuotes_;
        mutable Cube parametersGuess_;
        std::vector<bool> isParameterFixed_;
        bool isAtmCalibrated_;
        const ext::shared_ptr<EndCriteria> endCriteria_;
        Real maxErrorTolerance_;
        const ext::shared_ptr<OptimizationMethod> optMethod_;
        Real errorAccept_;
        const bool useMaxError_;
        const Size maxGuesses_;
        const bool backwardFlat_;
        const Real cutoffStrike_;
        VolatilityType volatilityType_;

        class PrivateObserver : public Observer {
          public:
            explicit PrivateObserver(XabrSwaptionVolatilityCube<Model> *v)
                : v_(v) {}
            void update() override {
                v_->setParameterGuess();
                v_->update();
            }

          private:
            XabrSwaptionVolatilityCube<Model> *v_;
        };

       ext::shared_ptr<PrivateObserver> privateObserver_;

    };

    //=======================================================================//
    //                        XabrSwaptionVolatilityCube                              //
    //=======================================================================//

    template <class Model>
    XabrSwaptionVolatilityCube<Model>::XabrSwaptionVolatilityCube(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& optionTenors,
        const std::vector<Period>& swapTenors,
        const std::vector<Spread>& strikeSpreads,
        const std::vector<std::vector<Handle<Quote> > >& volSpreads,
        const ext::shared_ptr<SwapIndex>& swapIndexBase,
        const ext::shared_ptr<SwapIndex>& shortSwapIndexBase,
        bool vegaWeightedSmileFit,
        std::vector<std::vector<Handle<Quote> > > parametersGuess,
        std::vector<bool> isParameterFixed,
        bool isAtmCalibrated,
        ext::shared_ptr<EndCriteria> endCriteria,
        Real maxErrorTolerance,
        ext::shared_ptr<OptimizationMethod> optMethod,
        const Real errorAccept,
        const bool useMaxError,
        const Size maxGuesses,
        const bool backwardFlat,
        const Real cutoffStrike)
    : SwaptionVolatilityCube(atmVolStructure,
                             optionTenors,
                             swapTenors,
                             strikeSpreads,
                             volSpreads,
                             swapIndexBase,
                             shortSwapIndexBase,
                             vegaWeightedSmileFit),
      parametersGuessQuotes_(std::move(parametersGuess)),
      isParameterFixed_(std::move(isParameterFixed)), isAtmCalibrated_(isAtmCalibrated),
      endCriteria_(std::move(endCriteria)), optMethod_(std::move(optMethod)),
      useMaxError_(useMaxError), maxGuesses_(maxGuesses), backwardFlat_(backwardFlat),
      cutoffStrike_(cutoffStrike), volatilityType_(atmVolStructure->volatilityType()) {

        if (maxErrorTolerance != Null<Rate>()) {
            maxErrorTolerance_ = maxErrorTolerance;
        } else{
            maxErrorTolerance_ = SWAPTIONVOLCUBE_TOL;
            if (vegaWeightedSmileFit_) maxErrorTolerance_ =  SWAPTIONVOLCUBE_VEGAWEIGHTED_TOL;
        }
        if (errorAccept != Null<Rate>()) {
            errorAccept_ = errorAccept;
        } else{
            errorAccept_ = maxErrorTolerance_ / 5.0;
        }

        privateObserver_ = ext::make_shared<PrivateObserver>(this);
        registerWithParametersGuess();
        setParameterGuess();
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::registerWithParametersGuess()
    {
        for (Size i=0; i<4; i++)
            for (Size j=0; j<nOptionTenors_; j++)
                for (Size k=0; k<nSwapTenors_; k++)
                    privateObserver_->registerWith(parametersGuessQuotes_[j+k*nOptionTenors_][i]);
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::setParameterGuess() const {

        //! set parametersGuess_ by parametersGuessQuotes_
        parametersGuess_ = Cube(optionDates_, swapTenors_,
                                optionTimes_, swapLengths_, 4,
                                true, backwardFlat_);
        Size i;
        for (i=0; i<4; i++)
            for (Size j=0; j<nOptionTenors_ ; j++)
                for (Size k=0; k<nSwapTenors_; k++) {
                    parametersGuess_.setElement(i, j, k,
                        parametersGuessQuotes_[j+k*nOptionTenors_][i]->value());
                }
        parametersGuess_.updateInterpolators();

    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::performCalculations() const {

        SwaptionVolatilityCube::performCalculations();

        //! set marketVolCube_ by volSpreads_ quotes
        marketVolCube_ = Cube(optionDates_, swapTenors_,
                              optionTimes_, swapLengths_, nStrikes_);
        Rate atmForward;
        Volatility atmVol, vol;
        for (Size j=0; j<nOptionTenors_; ++j) {
            for (Size k=0; k<nSwapTenors_; ++k) {
                atmForward = atmStrike(optionDates_[j], swapTenors_[k]);
                atmVol = atmVol_->volatility(optionDates_[j], swapTenors_[k],
                                                              atmForward);
                for (Size i=0; i<nStrikes_; ++i) {
                    vol = atmVol + volSpreads_[j*nSwapTenors_+k][i]->value();
                    marketVolCube_.setElement(i, j, k, vol);
                }
            }
        }
        marketVolCube_.updateInterpolators();

        sparseParameters_ = sabrCalibration(marketVolCube_);
        //parametersGuess_ = sparseParameters_;
        sparseParameters_.updateInterpolators();
        //parametersGuess_.updateInterpolators();
        volCubeAtmCalibrated_= marketVolCube_;

        if(isAtmCalibrated_){
            fillVolatilityCube();
            denseParameters_ = sabrCalibration(volCubeAtmCalibrated_);
            denseParameters_.updateInterpolators();
        }
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::updateAfterRecalibration() {
        volCubeAtmCalibrated_ = marketVolCube_;
        if(isAtmCalibrated_){
            fillVolatilityCube();
            denseParameters_ = sabrCalibration(volCubeAtmCalibrated_);
            denseParameters_.updateInterpolators();
        }
        notifyObservers();
    }

    template <class Model>
    typename XabrSwaptionVolatilityCube<Model>::Cube
    XabrSwaptionVolatilityCube<Model>::sabrCalibration(const Cube &marketVolCube) const {

        const std::vector<Time>& optionTimes = marketVolCube.optionTimes();
        const std::vector<Time>& swapLengths = marketVolCube.swapLengths();
        const std::vector<Date>& optionDates = marketVolCube.optionDates();
        const std::vector<Period>& swapTenors = marketVolCube.swapTenors();
        Matrix alphas(optionTimes.size(), swapLengths.size(),0.);
        Matrix betas(alphas);
        Matrix nus(alphas);
        Matrix rhos(alphas);
        Matrix forwards(alphas);
        Matrix errors(alphas);
        Matrix maxErrors(alphas);
        Matrix endCriteria(alphas);

        const std::vector<Matrix>& tmpMarketVolCube = marketVolCube.points();

        std::vector<Real> strikes(strikeSpreads_.size());
        std::vector<Real> volatilities(strikeSpreads_.size());

        for (Size j=0; j<optionTimes.size(); j++) {
            for (Size k=0; k<swapLengths.size(); k++) {
                Rate atmForward = atmStrike(optionDates[j], swapTenors[k]);
                Real shiftTmp = atmVol_->shift(optionTimes[j], swapLengths[k]);
                strikes.clear();
                volatilities.clear();
                for (Size i=0; i<nStrikes_; i++){
                    Real strike = atmForward+strikeSpreads_[i];
                    if(strike + shiftTmp >=cutoffStrike_) {
                        strikes.push_back(strike);
                        volatilities.push_back(tmpMarketVolCube[i][j][k]);
                    }
                }

                const std::vector<Real>& guess =
                    parametersGuess_(optionTimes[j], swapLengths[k]);

                const ext::shared_ptr<typename Model::Interpolation> sabrInterpolation =
                    ext::shared_ptr<typename Model::Interpolation>(new
                                          (typename Model::Interpolation)(strikes.begin(), strikes.end(),
                                          volatilities.begin(),
                                          optionTimes[j], atmForward,
                                          guess[0], guess[1],
                                          guess[2], guess[3],
                                          isParameterFixed_[0],
                                          isParameterFixed_[1],
                                          isParameterFixed_[2],
                                          isParameterFixed_[3],
                                          vegaWeightedSmileFit_,
                                          endCriteria_,
                                          optMethod_,
                                          errorAccept_,
                                          useMaxError_,
                                          maxGuesses_,
                                          shiftTmp,
                                          volatilityType_));
                sabrInterpolation->update();

                Real rmsError = sabrInterpolation->rmsError();
                Real maxError = sabrInterpolation->maxError();
                alphas     [j][k] = sabrInterpolation->alpha();
                betas      [j][k] = sabrInterpolation->beta();
                nus        [j][k] = sabrInterpolation->nu();
                rhos       [j][k] = sabrInterpolation->rho();
                forwards   [j][k] = atmForward;
                errors     [j][k] = rmsError;
                maxErrors  [j][k] = maxError;
                endCriteria[j][k] = sabrInterpolation->endCriteria();

                QL_ENSURE(endCriteria[j][k] != Integer(EndCriteria::MaxIterations),
                          "global swaptions calibration failed: "
                          "MaxIterations reached: " << "\n" <<
                          "option maturity = " << optionDates[j] << ", \n" <<
                          "swap tenor = " << swapTenors[k] << ", \n" <<
                          "rms error = " << io::rate(errors[j][k])  << ", \n" <<
                          "max error = " << io::rate(maxErrors[j][k]) << ", \n" <<
                          "   alpha = " <<  alphas[j][k] << "n" <<
                          "   beta = " <<  betas[j][k] << "\n" <<
                          "   nu = " <<  nus[j][k]   << "\n" <<
                          "   rho = " <<  rhos[j][k]  << "\n"
                          );

                QL_ENSURE((useMaxError_ ? maxError : rmsError) < maxErrorTolerance_,
                          "global swaptions calibration failed: "
                          "error tolerance exceeded: "
                              << "\n"
                              << "using " << (useMaxError_ ? "maxError" : "rmsError")
                              << " tolerance " << maxErrorTolerance_ << ", \n"
                              << "option maturity = " << optionDates[j] << ", \n"
                              << "swap tenor = " << swapTenors[k] << ", \n"
                              << "rms error = " << io::rate(errors[j][k]) << ", \n"
                              << "max error = " << io::rate(maxErrors[j][k]) << ", \n"
                              << "   alpha = " << alphas[j][k] << "n"
                              << "   beta = " << betas[j][k] << "\n"
                              << "   nu = " << nus[j][k] << "\n"
                              << "   rho = " << rhos[j][k] << "\n");
            }
        }
        Cube sabrParametersCube(optionDates, swapTenors,
                                optionTimes, swapLengths, 8,
                                true, backwardFlat_);
        sabrParametersCube.setLayer(0, alphas);
        sabrParametersCube.setLayer(1, betas);
        sabrParametersCube.setLayer(2, nus);
        sabrParametersCube.setLayer(3, rhos);
        sabrParametersCube.setLayer(4, forwards);
        sabrParametersCube.setLayer(5, errors);
        sabrParametersCube.setLayer(6, maxErrors);
        sabrParametersCube.setLayer(7, endCriteria);

        return sabrParametersCube;

    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::sabrCalibrationSection(
                                            const Cube& marketVolCube,
                                            Cube& parametersCube,
                                            const Period& swapTenor) const {

        const std::vector<Time>& optionTimes = marketVolCube.optionTimes();
        const std::vector<Time>& swapLengths = marketVolCube.swapLengths();
        const std::vector<Date>& optionDates = marketVolCube.optionDates();
        const std::vector<Period>& swapTenors = marketVolCube.swapTenors();

        Size k = std::find(swapTenors.begin(), swapTenors.end(),
                           swapTenor) - swapTenors.begin();
        QL_REQUIRE(k != swapTenors.size(), "swap tenor not found");

        std::vector<Real> calibrationResult(8,0.);
        const std::vector<Matrix>& tmpMarketVolCube = marketVolCube.points();

        std::vector<Real> strikes(strikeSpreads_.size());
        std::vector<Real> volatilities(strikeSpreads_.size());

        for (Size j=0; j<optionTimes.size(); j++) {
            Rate atmForward = atmStrike(optionDates[j], swapTenors[k]);
            Real shiftTmp = atmVol_->shift(optionTimes[j], swapLengths[k]);
            strikes.clear();
            volatilities.clear();
            for (Size i=0; i<nStrikes_; i++){
                Real strike = atmForward+strikeSpreads_[i];
                if(strike+shiftTmp>=cutoffStrike_) {
                    strikes.push_back(strike);
                    volatilities.push_back(tmpMarketVolCube[i][j][k]);
                }
            }

            const std::vector<Real>& guess =
                parametersGuess_(optionTimes[j], swapLengths[k]);

                const ext::shared_ptr<typename Model::Interpolation> sabrInterpolation =
                    ext::shared_ptr<typename Model::Interpolation>(new
                                          (typename Model::Interpolation)(strikes.begin(), strikes.end(),
                                      volatilities.begin(),
                                      optionTimes[j], atmForward,
                                      guess[0], guess[1],
                                      guess[2], guess[3],
                                      isParameterFixed_[0],
                                      isParameterFixed_[1],
                                      isParameterFixed_[2],
                                      isParameterFixed_[3],
                                      vegaWeightedSmileFit_,
                                      endCriteria_,
                                      optMethod_,
                                      errorAccept_,
                                      useMaxError_,
                                      maxGuesses_,
                                      shiftTmp));

            sabrInterpolation->update();
            Real interpolationError = sabrInterpolation->rmsError();
            calibrationResult[0]=sabrInterpolation->alpha();
            calibrationResult[1]=sabrInterpolation->beta();
            calibrationResult[2]=sabrInterpolation->nu();
            calibrationResult[3]=sabrInterpolation->rho();
            calibrationResult[4]=atmForward;
            calibrationResult[5]=interpolationError;
            calibrationResult[6]=sabrInterpolation->maxError();
            calibrationResult[7]=sabrInterpolation->endCriteria();

            QL_ENSURE(calibrationResult[7] != Integer(EndCriteria::MaxIterations),
                      "section calibration failed: "
                      "option tenor " << optionDates[j] <<
                      ", swap tenor " << swapTenors[k] <<
                      ": max iteration (" <<
                      endCriteria_->maxIterations() << ")" <<
                          ", alpha " <<  calibrationResult[0]<<
                          ", beta "  <<  calibrationResult[1] <<
                          ", nu "    <<  calibrationResult[2]   <<
                          ", rho "   <<  calibrationResult[3]  <<
                          ", max error " << calibrationResult[6] <<
                          ", error " <<  calibrationResult[5]
                          );

            QL_ENSURE((useMaxError_ ? calibrationResult[6] : calibrationResult[5]) < maxErrorTolerance_,
                      "section calibration failed: "
                      "option tenor " << optionDates[j] <<
                      ", swap tenor " << swapTenors[k] <<
                      (useMaxError_ ? ": max error " : ": error ") <<
                      (useMaxError_ ? calibrationResult[6] : calibrationResult[5]) <<
                          ", alpha " <<  calibrationResult[0] <<
                          ", beta "  <<  calibrationResult[1] <<
                          ", nu "    <<  calibrationResult[2] <<
                          ", rho "   <<  calibrationResult[3] <<
                      (useMaxError_ ? ": error" : ": max error ") <<
                      (useMaxError_ ? calibrationResult[5] : calibrationResult[6])
            );

            parametersCube.setPoint(optionDates[j], swapTenors[k],
                                    optionTimes[j], swapLengths[k],
                                    calibrationResult);
            parametersCube.updateInterpolators();
        }

    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::fillVolatilityCube() const {

        const ext::shared_ptr<SwaptionVolatilityDiscrete> atmVolStructure =
            ext::dynamic_pointer_cast<SwaptionVolatilityDiscrete>(*atmVol_);

        std::vector<Time> atmOptionTimes(atmVolStructure->optionTimes());
        std::vector<Time> optionTimes(volCubeAtmCalibrated_.optionTimes());
        atmOptionTimes.insert(atmOptionTimes.end(),
                              optionTimes.begin(), optionTimes.end());
        std::sort(atmOptionTimes.begin(),atmOptionTimes.end());
        auto new_end = std::unique(atmOptionTimes.begin(), atmOptionTimes.end());
        atmOptionTimes.erase(new_end, atmOptionTimes.end());

        std::vector<Time> atmSwapLengths(atmVolStructure->swapLengths());
        std::vector<Time> swapLengths(volCubeAtmCalibrated_.swapLengths());
        atmSwapLengths.insert(atmSwapLengths.end(),
                              swapLengths.begin(), swapLengths.end());
        std::sort(atmSwapLengths.begin(),atmSwapLengths.end());
        new_end = std::unique(atmSwapLengths.begin(), atmSwapLengths.end());
        atmSwapLengths.erase(new_end, atmSwapLengths.end());

        std::vector<Date> atmOptionDates = atmVolStructure->optionDates();
        std::vector<Date> optionDates(volCubeAtmCalibrated_.optionDates());
        atmOptionDates.insert(atmOptionDates.end(),
                                optionDates.begin(), optionDates.end());
        std::sort(atmOptionDates.begin(),atmOptionDates.end());
        auto new_end_1 = std::unique(atmOptionDates.begin(), atmOptionDates.end());
        atmOptionDates.erase(new_end_1, atmOptionDates.end());

        std::vector<Period> atmSwapTenors = atmVolStructure->swapTenors();
        std::vector<Period> swapTenors(volCubeAtmCalibrated_.swapTenors());
        atmSwapTenors.insert(atmSwapTenors.end(),
                             swapTenors.begin(), swapTenors.end());
        std::sort(atmSwapTenors.begin(),atmSwapTenors.end());
        auto new_end_2 = std::unique(atmSwapTenors.begin(), atmSwapTenors.end());
        atmSwapTenors.erase(new_end_2, atmSwapTenors.end());

        createSparseSmiles();

        for (Size j=0; j<atmOptionTimes.size(); j++) {

            for (Size k=0; k<atmSwapLengths.size(); k++) {
                bool expandOptionTimes =
                    !(std::binary_search(optionTimes.begin(),
                                         optionTimes.end(),
                                         atmOptionTimes[j]));
                bool expandSwapLengths =
                    !(std::binary_search(swapLengths.begin(),
                                         swapLengths.end(),
                                         atmSwapLengths[k]));
                if(expandOptionTimes || expandSwapLengths){
                    Rate atmForward = atmStrike(atmOptionDates[j],
                                                atmSwapTenors[k]);
                    Volatility atmVol = atmVol_->volatility(
                        atmOptionDates[j], atmSwapTenors[k], atmForward);
                    std::vector<Real> spreadVols =
                        spreadVolInterpolation(atmOptionDates[j],
                                               atmSwapTenors[k]);
                    std::vector<Real> volAtmCalibrated;
                    volAtmCalibrated.reserve(nStrikes_);
                    for (Size i=0; i<nStrikes_; i++)
                        volAtmCalibrated.push_back(atmVol + spreadVols[i]);
                    volCubeAtmCalibrated_.setPoint(
                                    atmOptionDates[j], atmSwapTenors[k],
                                    atmOptionTimes[j], atmSwapLengths[k],
                                    volAtmCalibrated);
                }
            }
        }
        volCubeAtmCalibrated_.updateInterpolators();
    }


    template<class Model> void XabrSwaptionVolatilityCube<Model>::createSparseSmiles() const {

        std::vector<Time> optionTimes(sparseParameters_.optionTimes());
        std::vector<Time> swapLengths(sparseParameters_.swapLengths());
        sparseSmiles_.clear();

        for (Real& optionTime : optionTimes) {
            std::vector<ext::shared_ptr<SmileSection> > tmp;
            Size n = swapLengths.size();
            tmp.reserve(n);
            for (Size k=0; k<n; ++k) {
                tmp.push_back(smileSection(optionTime, swapLengths[k], sparseParameters_));
            }
            sparseSmiles_.push_back(tmp);
        }
    }


    template<class Model> std::vector<Real> XabrSwaptionVolatilityCube<Model>::spreadVolInterpolation(
        const Date& atmOptionDate, const Period& atmSwapTenor) const {

        Time atmOptionTime = timeFromReference(atmOptionDate);
        Time atmTimeLength = swapLength(atmSwapTenor);

        std::vector<Real> result;
        const std::vector<Time>& optionTimes(sparseParameters_.optionTimes());
        const std::vector<Time>& swapLengths(sparseParameters_.swapLengths());
        const std::vector<Date>& optionDates =
            sparseParameters_.optionDates();
        const std::vector<Period>& swapTenors = sparseParameters_.swapTenors();

        std::vector<Real>::const_iterator optionTimesPreviousNode,
                                          swapLengthsPreviousNode;

        optionTimesPreviousNode = std::lower_bound(optionTimes.begin(),
                                                   optionTimes.end(),
                                                   atmOptionTime);
        Size optionTimesPreviousIndex =
            optionTimesPreviousNode - optionTimes.begin();
        if (optionTimesPreviousIndex >0)
            optionTimesPreviousIndex --;

        swapLengthsPreviousNode = std::lower_bound(swapLengths.begin(),
                                                   swapLengths.end(),
                                                   atmTimeLength);
        Size swapLengthsPreviousIndex = swapLengthsPreviousNode - swapLengths.begin();
        if (swapLengthsPreviousIndex >0)
            swapLengthsPreviousIndex --;

        std::vector< std::vector<ext::shared_ptr<SmileSection> > > smiles;
        std::vector<ext::shared_ptr<SmileSection> >  smilesOnPreviousExpiry;
        std::vector<ext::shared_ptr<SmileSection> >  smilesOnNextExpiry;

        QL_REQUIRE(optionTimesPreviousIndex+1 < sparseSmiles_.size(),
                   "optionTimesPreviousIndex+1 >= sparseSmiles_.size()");
        QL_REQUIRE(swapLengthsPreviousIndex+1 < sparseSmiles_[0].size(),
                   "swapLengthsPreviousIndex+1 >= sparseSmiles_[0].size()");
        smilesOnPreviousExpiry.push_back(
              sparseSmiles_[optionTimesPreviousIndex][swapLengthsPreviousIndex]);
        smilesOnPreviousExpiry.push_back(
              sparseSmiles_[optionTimesPreviousIndex][swapLengthsPreviousIndex+1]);
        smilesOnNextExpiry.push_back(
              sparseSmiles_[optionTimesPreviousIndex+1][swapLengthsPreviousIndex]);
        smilesOnNextExpiry.push_back(
              sparseSmiles_[optionTimesPreviousIndex+1][swapLengthsPreviousIndex+1]);

        smiles.push_back(smilesOnPreviousExpiry);
        smiles.push_back(smilesOnNextExpiry);

        std::vector<Real> optionsNodes(2);
        optionsNodes[0] = optionTimes[optionTimesPreviousIndex];
        optionsNodes[1] = optionTimes[optionTimesPreviousIndex+1];

        std::vector<Date> optionsDateNodes(2);
        optionsDateNodes[0] = optionDates[optionTimesPreviousIndex];
        optionsDateNodes[1] = optionDates[optionTimesPreviousIndex+1];

        std::vector<Real> swapLengthsNodes(2);
        swapLengthsNodes[0] = swapLengths[swapLengthsPreviousIndex];
        swapLengthsNodes[1] = swapLengths[swapLengthsPreviousIndex+1];

        std::vector<Period> swapTenorNodes(2);
        swapTenorNodes[0] = swapTenors[swapLengthsPreviousIndex];
        swapTenorNodes[1] = swapTenors[swapLengthsPreviousIndex+1];

        Rate atmForward = atmStrike(atmOptionDate, atmSwapTenor);
        Real shift = atmVol_->shift(atmOptionTime, atmTimeLength);

        Matrix atmForwards(2, 2, 0.0);
        Matrix atmShifts(2,2,0.0);
        Matrix atmVols(2, 2, 0.0);
        for (Size i=0; i<2; i++) {
            for (Size j=0; j<2; j++) {
                atmForwards[i][j] = atmStrike(optionsDateNodes[i],
                                              swapTenorNodes[j]);
                atmShifts[i][j] = atmVol_->shift(optionsNodes[i], swapLengthsNodes[j]);
                // atmVols[i][j] = smiles[i][j]->volatility(atmForwards[i][j]);
                atmVols[i][j] = atmVol_->volatility(
                    optionsDateNodes[i], swapTenorNodes[j], atmForwards[i][j]);
                /* With the old implementation the interpolated spreads on ATM
                   volatilities were null even if the spreads on ATM volatilities to be
                   interpolated were non-zero. The new implementation removes
                   this behaviour, but introduces a small ERROR in the cube:
                   even if no spreads are applied on any cube ATM volatility corresponding
                   to quoted smile sections (that is ATM volatilities in sparse cube), the
                   cube ATM volatilities corresponding to not quoted smile sections (that
                   is ATM volatilities in dense cube) are no more exactly the quoted values,
                   but that ones PLUS the linear interpolation of the fit errors on the ATM
                   volatilities in sparse cube whose spreads are used in the calculation.
                   A similar imprecision is introduced to the volatilities in dense cube
                   whith moneyness near to 1.
                   (See below how spreadVols are calculated).
                   The extent of this error depends on the quality of the fit: in case of
                   good fits it is negligibile.
                */
            }
        }

        for (Size k=0; k<nStrikes_; k++){
            const Real strike = std::max(atmForward + strikeSpreads_[k],cutoffStrike_-shift);
            const Real moneyness = (atmForward+shift)/(strike+shift);

            Matrix strikes(2,2,0.);
            Matrix spreadVols(2,2,0.);
            for (Size i=0; i<2; i++){
                for (Size j=0; j<2; j++){
                    strikes[i][j] = (atmForwards[i][j]+atmShifts[i][j])/moneyness - atmShifts[i][j];
                    spreadVols[i][j] =
                        smiles[i][j]->volatility(strikes[i][j]) - atmVols[i][j];
                }
            }
           Cube localInterpolator(optionsDateNodes, swapTenorNodes,
                                  optionsNodes, swapLengthsNodes, 1);
           localInterpolator.setLayer(0, spreadVols);
           localInterpolator.updateInterpolators();

           result.push_back(localInterpolator(atmOptionTime, atmTimeLength)[0]);
        }
        return result;
    }

    template<class Model> ext::shared_ptr<SmileSection>
    XabrSwaptionVolatilityCube<Model>::smileSection(Time optionTime, Time swapLength,
                                   const Cube& sabrParametersCube) const {

        calculate();
        const std::vector<Real> sabrParameters =
            sabrParametersCube(optionTime, swapLength);
        Real shiftTmp = atmVol_->shift(optionTime,swapLength);
        return ext::shared_ptr<SmileSection>(new (typename Model::SmileSection)(
                          optionTime, sabrParameters[4], sabrParameters,shiftTmp, volatilityType_));
    }

    template<class Model> ext::shared_ptr<SmileSection>
    XabrSwaptionVolatilityCube<Model>::smileSectionImpl(Time optionTime,
                                       Time swapLength) const {
        if (isAtmCalibrated_)
            return smileSection(optionTime, swapLength, denseParameters_);
        else
            return smileSection(optionTime, swapLength, sparseParameters_);
    }

    template<class Model> Matrix XabrSwaptionVolatilityCube<Model>::sparseSabrParameters() const {
        calculate();
        return sparseParameters_.browse();
    }

    template<class Model> Matrix XabrSwaptionVolatilityCube<Model>::denseSabrParameters() const {
        calculate();
        return denseParameters_.browse();
    }

    template<class Model> Matrix XabrSwaptionVolatilityCube<Model>::marketVolCube() const {
        calculate();
        return marketVolCube_.browse();
    }

    template<class Model> Matrix XabrSwaptionVolatilityCube<Model>::volCubeAtmCalibrated() const {
        calculate();
        return volCubeAtmCalibrated_.browse();
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::recalibration(Real beta,
                                         const Period& swapTenor) {

        std::vector<Real> betaVector(nOptionTenors_, beta);
        recalibration(betaVector,swapTenor);

    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::recalibration(const std::vector<Real> &beta,
                                         const Period& swapTenor) {

        QL_REQUIRE(beta.size() == nOptionTenors_,
                   "beta size ("
                       << beta.size()
                       << ") must be equal to number of option tenors ("
                       << nOptionTenors_ << ")");

        const std::vector<Period> &swapTenors = marketVolCube_.swapTenors();
        Size k = std::find(swapTenors.begin(), swapTenors.end(), swapTenor) -
                 swapTenors.begin();

        QL_REQUIRE(k != swapTenors.size(), "swap tenor (" << swapTenor
                                                          << ") not found");

        for (Size i = 0; i < nOptionTenors_; ++i) {
            parametersGuess_.setElement(1, i, k, beta[i]);
        }

        parametersGuess_.updateInterpolators();
        sabrCalibrationSection(marketVolCube_, sparseParameters_, swapTenor);

        volCubeAtmCalibrated_ = marketVolCube_;
        if (isAtmCalibrated_) {
            fillVolatilityCube();
            sabrCalibrationSection(volCubeAtmCalibrated_, denseParameters_,
                                   swapTenor);
        }
        notifyObservers();

    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::recalibration(const std::vector<Period> &swapLengths,
                                         const std::vector<Real> &beta,
                                         const Period &swapTenor) {

        QL_REQUIRE(beta.size() == swapLengths.size(),
                   "beta size ("
                       << beta.size()
                       << ") must be equal to number of swap lengths ("
                       << swapLengths.size() << ")");

        std::vector<Time> betaTimes;
        for (Size i = 0; i < beta.size(); i++)
            betaTimes.push_back(
                timeFromReference(optionDateFromTenor(swapLengths[i])));

        LinearInterpolation betaInterpolation(betaTimes.begin(),
                                              betaTimes.end(), beta.begin());

        std::vector<Real> cubeBeta;
        for (Size i = 0; i < optionTimes().size(); i++) {
            Real t = optionTimes()[i];
            // flat extrapolation ensures admissable values
            if (t < betaTimes.front())
                t = betaTimes.front();
            if (t > betaTimes.back())
                t = betaTimes.back();
            cubeBeta.push_back(betaInterpolation(t));
        }

        recalibration(cubeBeta, swapTenor);

    }

    //======================================================================//
    //                      XabrSwaptionVolatilityCube::Cube                         //
    //======================================================================//


    template<class Model> XabrSwaptionVolatilityCube<Model>::Cube::Cube(const std::vector<Date>& optionDates,
                                    const std::vector<Period>& swapTenors,
                                    const std::vector<Time>& optionTimes,
                                    const std::vector<Time>& swapLengths,
                                    Size nLayers,
                                    bool extrapolation,
                                    bool backwardFlat)
    : optionTimes_(optionTimes), swapLengths_(swapLengths),
      optionDates_(optionDates), swapTenors_(swapTenors),
        nLayers_(nLayers), extrapolation_(extrapolation),
        backwardFlat_(backwardFlat) {

        QL_REQUIRE(optionTimes.size()>1,"Cube::Cube(...): optionTimes.size()<2");
        QL_REQUIRE(swapLengths.size()>1,"Cube::Cube(...): swapLengths.size()<2");

        QL_REQUIRE(optionTimes.size()==optionDates.size(),
                   "Cube::Cube(...): optionTimes/optionDates mismatch");
        QL_REQUIRE(swapTenors.size()==swapLengths.size(),
                   "Cube::Cube(...): swapTenors/swapLengths mismatch");

        std::vector<Matrix> points(nLayers_, Matrix(optionTimes_.size(),
                                                    swapLengths_.size(), 0.0));
        for (Size k=0;k<nLayers_;k++) {
            ext::shared_ptr<Interpolation2D> interpolation;
            transposedPoints_.push_back(transpose(points[k]));
            if (k <= 4 && backwardFlat_)
                interpolation =
                    ext::make_shared<BackwardflatLinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            else
                interpolation =
                    ext::make_shared<BilinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            interpolators_.push_back(ext::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation)));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(points);
     }

    template<class Model> XabrSwaptionVolatilityCube<Model>::Cube::Cube(const Cube& o)
    : optionTimes_(o.optionTimes_), swapLengths_(o.swapLengths_),
      optionDates_(o.optionDates_), swapTenors_(o.swapTenors_),
      nLayers_(o.nLayers_), transposedPoints_(o.transposedPoints_),
      extrapolation_(o.extrapolation_), backwardFlat_(o.backwardFlat_) {
        for (Size k=0; k<nLayers_; ++k) {
            ext::shared_ptr<Interpolation2D> interpolation;
            if (k <= 4 && backwardFlat_)
                interpolation =
                    ext::make_shared<BackwardflatLinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            else
                interpolation =
                    ext::make_shared<BilinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            interpolators_.push_back(ext::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation)));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(o.points_);
    }

    template<class Model> typename XabrSwaptionVolatilityCube<Model>::Cube&
    XabrSwaptionVolatilityCube<Model>::Cube::operator=(const Cube& o) {
        optionTimes_ = o.optionTimes_;
        swapLengths_ = o.swapLengths_;
        optionDates_ = o.optionDates_;
        swapTenors_ = o.swapTenors_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        backwardFlat_ = o.backwardFlat_;
        transposedPoints_ = o.transposedPoints_;
        for(Size k=0;k<nLayers_;k++){
            ext::shared_ptr<Interpolation2D> interpolation;
            if (k <= 4 && backwardFlat_)
                interpolation =
                    ext::make_shared<BackwardflatLinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            else
                interpolation =
                    ext::make_shared<BilinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            interpolators_.push_back(ext::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation)));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(o.points_);
        return *this;
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::Cube::setElement(Size IndexOfLayer,
                                                        Size IndexOfRow,
                                                        Size IndexOfColumn,
                                                        Real x) {
        QL_REQUIRE(IndexOfLayer<nLayers_,
            "Cube::setElement: incompatible IndexOfLayer ");
        QL_REQUIRE(IndexOfRow<optionTimes_.size(),
            "Cube::setElement: incompatible IndexOfRow");
        QL_REQUIRE(IndexOfColumn<swapLengths_.size(),
            "Cube::setElement: incompatible IndexOfColumn");
        points_[IndexOfLayer][IndexOfRow][IndexOfColumn] = x;
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::Cube::setPoints(
                                               const std::vector<Matrix>& x) {
        QL_REQUIRE(x.size()==nLayers_,
            "Cube::setPoints: incompatible number of layers ");
        QL_REQUIRE(x[0].rows()==optionTimes_.size(),
            "Cube::setPoints: incompatible size 1");
        QL_REQUIRE(x[0].columns()==swapLengths_.size(),
            "Cube::setPoints: incompatible size 2");

        points_ = x;
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::Cube::setLayer(Size i,
                                                      const Matrix& x) {
        QL_REQUIRE(i<nLayers_,
            "Cube::setLayer: incompatible number of layer ");
        QL_REQUIRE(x.rows()==optionTimes_.size(),
            "Cube::setLayer: incompatible size 1");
        QL_REQUIRE(x.columns()==swapLengths_.size(),
            "Cube::setLayer: incompatible size 2");

        points_[i] = x;
    }

    template <class Model>
    void XabrSwaptionVolatilityCube<Model>::Cube::setPoint(const Date& optionDate,
                                                  const Period& swapTenor,
                                                  Time optionTime,
                                                  Time swapLength,
                                                  const std::vector<Real>& point)
    {
        const bool expandOptionTimes =
            !(std::binary_search(optionTimes_.begin(),optionTimes_.end(),optionTime));
        const bool expandSwapLengths =
            !(std::binary_search(swapLengths_.begin(),swapLengths_.end(),swapLength));

        std::vector<Real>::const_iterator optionTimesPreviousNode,
                                          swapLengthsPreviousNode;

        optionTimesPreviousNode =
            std::lower_bound(optionTimes_.begin(),optionTimes_.end(),optionTime);
        Size optionTimesIndex = optionTimesPreviousNode - optionTimes_.begin();

        swapLengthsPreviousNode =
            std::lower_bound(swapLengths_.begin(),swapLengths_.end(),swapLength);
        Size swapLengthsIndex = swapLengthsPreviousNode - swapLengths_.begin();

        if (expandOptionTimes || expandSwapLengths)
            expandLayers(optionTimesIndex, expandOptionTimes,
                         swapLengthsIndex, expandSwapLengths);

        for (Size k=0; k<nLayers_; ++k)
            points_[k][optionTimesIndex][swapLengthsIndex] = point[k];

        optionTimes_[optionTimesIndex] = optionTime;
        swapLengths_[swapLengthsIndex] = swapLength;
        optionDates_[optionTimesIndex] = optionDate;
        swapTenors_[swapLengthsIndex] = swapTenor;
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::Cube::expandLayers(Size i, bool expandOptionTimes,
                                              Size j, bool expandSwapLengths) {
        QL_REQUIRE(i<=optionTimes_.size(),"Cube::expandLayers: incompatible size 1");
        QL_REQUIRE(j<=swapLengths_.size(),"Cube::expandLayers: incompatible size 2");

        if (expandOptionTimes) {
            optionTimes_.insert(optionTimes_.begin()+i,0.);
            optionDates_.insert(optionDates_.begin()+i, Date());
        }
        if (expandSwapLengths) {
            swapLengths_.insert(swapLengths_.begin()+j,0.);
            swapTenors_.insert(swapTenors_.begin()+j, Period());
        }

        std::vector<Matrix> newPoints(nLayers_,Matrix(optionTimes_.size(),
                                                      swapLengths_.size(), 0.));

        for (Size k=0; k<nLayers_; ++k) {
            for (Size u=0; u<points_[k].rows(); ++u) {
                 Size indexOfRow = u;
                 if (u>=i && expandOptionTimes) indexOfRow = u+1;
                 for (Size v=0; v<points_[k].columns(); ++v) {
                      Size indexOfCol = v;
                      if (v>=j && expandSwapLengths) indexOfCol = v+1;
                      newPoints[k][indexOfRow][indexOfCol]=points_[k][u][v];
                 }
            }
        }
        setPoints(newPoints);
    }

    template<class Model> const std::vector<Matrix>&
    XabrSwaptionVolatilityCube<Model>::Cube::points() const {
        return points_;
    }

    template<class Model> std::vector<Real> XabrSwaptionVolatilityCube<Model>::Cube::operator()(
                            const Time optionTime, const Time swapLength) const {
        std::vector<Real> result;
        for (Size k=0; k<nLayers_; ++k)
            result.push_back((*interpolators_[k])(optionTime, swapLength));
        return result;
    }

    template<class Model> const std::vector<Time>&
    XabrSwaptionVolatilityCube<Model>::Cube::optionTimes() const {
        return optionTimes_;
    }

    template<class Model> const std::vector<Time>&
    XabrSwaptionVolatilityCube<Model>::Cube::swapLengths() const {
        return swapLengths_;
    }

    template<class Model> void XabrSwaptionVolatilityCube<Model>::Cube::updateInterpolators() const {
        for (Size k = 0; k < nLayers_; ++k) {
            transposedPoints_[k] = transpose(points_[k]);
            ext::shared_ptr<Interpolation2D> interpolation;
            if (k <= 4 && backwardFlat_)
                interpolation =
                    ext::make_shared<BackwardflatLinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            else
                interpolation =
                    ext::make_shared<BilinearInterpolation>(
                        optionTimes_.begin(), optionTimes_.end(),
                        swapLengths_.begin(), swapLengths_.end(),
                        transposedPoints_[k]);
            interpolators_[k] = ext::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation));
            interpolators_[k]->enableExtrapolation();
        }
    }

    template<class Model> Matrix XabrSwaptionVolatilityCube<Model>::Cube::browse() const {
        Matrix result(swapLengths_.size()*optionTimes_.size(), nLayers_+2, 0.0);
        for (Size i=0; i<swapLengths_.size(); ++i) {
            for (Size j=0; j<optionTimes_.size(); ++j) {
                result[i*optionTimes_.size()+j][0] = swapLengths_[i];
                result[i*optionTimes_.size()+j][1] = optionTimes_[j];
                for (Size k=0; k<nLayers_; ++k)
                    result[i*optionTimes_.size()+j][2+k] = points_[k][j][i];
            }
        }
        return result;
    }

    /*! \deprecated Renamed to XabrSwaptionVolatilityCube.
                    Deprecated in version 1.30.
    */    
    template <class Model>
    using SwaptionVolCube1x [[deprecated("renamed to XabrSwaptionVolatilityCube")]] = XabrSwaptionVolatilityCube<Model>;

    //======================================================================//
    //                      SabrSwaptionVolatilityCube                      //
    //======================================================================//

    //! Swaption Volatility Cube SABR 
    /*! This struct defines the types used by SABR Volatility cubes
        for interpolation (SABRInterpolation) and for modeling the
        smile (SabrSmileSection).
    */
    struct SwaptionVolCubeSabrModel {
        typedef SABRInterpolation Interpolation;
        typedef SabrSmileSection SmileSection;
    };


    //! SABR volatility cube for swaptions
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeSabrModel> SabrSwaptionVolatilityCube;

    /*! \deprecated Renamed to SabrSwaptionVolatilityCube.
                    Deprecated in version 1.30.
    */
    [[deprecated("renamed to SabrSwaptionVolatilityCube")]]
    typedef XabrSwaptionVolatilityCube<SwaptionVolCubeSabrModel> SwaptionVolCube1;

}

#endif
