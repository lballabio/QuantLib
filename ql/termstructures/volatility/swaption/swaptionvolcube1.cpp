/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Giorgio Facchinetti

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

#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/math/interpolations/flatextrapolation2d.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/quote.hpp>

#ifndef SWAPTIONVOLCUBE_VEGAWEIGHTED_TOL
    #define SWAPTIONVOLCUBE_VEGAWEIGHTED_TOL 15.0e-4
#endif
#ifndef SWAPTIONVOLCUBE_TOL
    #define SWAPTIONVOLCUBE_TOL 100.0e-4
#endif

#define MINSTRIKE 0.0001

namespace QuantLib {

    //=======================================================================//
    //                        SwaptionVolCube1                   //
    //=======================================================================//

    SwaptionVolCube1::SwaptionVolCube1(
                const Handle<SwaptionVolatilityStructure>& atmVolStructure,
                const std::vector<Period>& optionTenors,
                const std::vector<Period>& swapTenors,
                const std::vector<Spread>& strikeSpreads,
                const std::vector<std::vector<Handle<Quote> > >& volSpreads,
                const boost::shared_ptr<SwapIndex>& swapIndexBase,
                const boost::shared_ptr<SwapIndex>& shortSwapIndexBase,
                bool vegaWeightedSmileFit,
                const std::vector<std::vector<Handle<Quote> > >& parametersGuess,
                const std::vector<bool>& isParameterFixed,
                bool isAtmCalibrated,
                const boost::shared_ptr<EndCriteria>& endCriteria,
                Real maxErrorTolerance,
                const boost::shared_ptr<OptimizationMethod>& optMethod,
                const Real errorAccept,
                const bool useMaxError,
                const Size maxGuesses)
    : SwaptionVolatilityCube(atmVolStructure, optionTenors, swapTenors,
                             strikeSpreads, volSpreads, swapIndexBase,
                             shortSwapIndexBase,
                             vegaWeightedSmileFit),
      parametersGuessQuotes_(parametersGuess),
      isParameterFixed_(isParameterFixed), isAtmCalibrated_(isAtmCalibrated),
      endCriteria_(endCriteria), optMethod_(optMethod), errorAccept_(errorAccept), useMaxError_(useMaxError), maxGuesses_(maxGuesses)
    {
        if (maxErrorTolerance != Null<Rate>()) {
            maxErrorTolerance_ = maxErrorTolerance;
        } else{
            maxErrorTolerance_ = SWAPTIONVOLCUBE_TOL;
            if (vegaWeightedSmileFit_) maxErrorTolerance_ =  SWAPTIONVOLCUBE_VEGAWEIGHTED_TOL;
        }
       registerWithParametersGuess();
    }

    void SwaptionVolCube1::registerWithParametersGuess()
    {
        for (Size i=0; i<4; i++)
            for (Size j=0; j<nOptionTenors_; j++)
                for (Size k=0; k<nSwapTenors_; k++)
                    registerWith(parametersGuessQuotes_[j+k*nOptionTenors_][i]);
    }

    void SwaptionVolCube1::performCalculations() const{

        SwaptionVolatilityDiscrete::performCalculations();

        //! set parametersGuess_ by parametersGuessQuotes_
        parametersGuess_ = Cube(optionDates_, swapTenors_,
                                optionTimes_, swapLengths_, 4);
        Size i;
        for (i=0; i<4; i++)
            for (Size j=0; j<nOptionTenors_ ; j++)
                for (Size k=0; k<nSwapTenors_; k++) {
                    parametersGuess_.setElement(i, j, k,
                        parametersGuessQuotes_[j+k*nOptionTenors_][i]->value());
                }
        parametersGuess_.updateInterpolators();

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

    SwaptionVolCube1::Cube
    SwaptionVolCube1::sabrCalibration(const Cube& marketVolCube) const {

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
                strikes.clear();
                volatilities.clear();
                for (Size i=0; i<nStrikes_; i++){
                    Real strike = atmForward+strikeSpreads_[i];
                    if(strike>=MINSTRIKE) {
                        strikes.push_back(strike);
                        volatilities.push_back(tmpMarketVolCube[i][j][k]);
                    }
                }

                const std::vector<Real>& guess = parametersGuess_.operator()(
                    optionTimes[j], swapLengths[k]);

                const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
                    boost::shared_ptr<SABRInterpolation>(new
                        SABRInterpolation(strikes.begin(), strikes.end(),
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
                                          maxGuesses_));
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

                QL_ENSURE(endCriteria[j][k]!=EndCriteria::MaxIterations,
                          "global swaptions calibration failed: "
                          "MaxIterations reached: " << "\n" <<
                          "option maturity = " << optionDates[j] << ", \n" <<
                          "swap tenor = " << swapTenors[k] << ", \n" <<
                          "error = " << io::rate(errors[j][k])  << ", \n" <<
                          "max error = " << io::rate(maxErrors[j][k]) << ", \n" <<
                          "   alpha = " <<  alphas[j][k] << "n" <<
                          "   beta = " <<  betas[j][k] << "\n" <<
                          "   nu = " <<  nus[j][k]   << "\n" <<
                          "   rho = " <<  rhos[j][k]  << "\n"
                          );

                QL_ENSURE(useMaxError_ ? maxError : rmsError < maxErrorTolerance_,
                      "global swaptions calibration failed: "
                      "option tenor " << optionDates[j] <<
                      ", swap tenor " << swapTenors[k] <<
                      (useMaxError_ ? ": max error " : ": error") <<
                      (useMaxError_ ? maxError : rmsError) <<
                          "   alpha = " <<  alphas[j][k] << "n" <<
                          "   beta = " <<  betas[j][k] << "\n" <<
                          "   nu = " <<  nus[j][k]   << "\n" <<
                          "   rho = " <<  rhos[j][k]  << "\n" <<
                      (useMaxError_ ? ": error" : ": max error ") <<
                      (useMaxError_ ? rmsError :maxError)
                );

            }
        }
        Cube sabrParametersCube(optionDates, swapTenors,
                                optionTimes, swapLengths, 8);
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
    void SwaptionVolCube1::sabrCalibrationSection(
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
            strikes.clear();
            volatilities.clear();
            for (Size i=0; i<nStrikes_; i++){
                Real strike = atmForward+strikeSpreads_[i];
                if(strike>=MINSTRIKE) {
                    strikes.push_back(strike);
                    volatilities.push_back(tmpMarketVolCube[i][j][k]);
                }
            }

            const std::vector<Real>& guess = parametersGuess_.operator()(
                optionTimes[j], swapLengths[k]);

            const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
                boost::shared_ptr<SABRInterpolation>(new
                    SABRInterpolation(strikes.begin(), strikes.end(),
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
                                      maxGuesses_));

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

            QL_ENSURE(calibrationResult[7]!=EndCriteria::MaxIterations,
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

            QL_ENSURE(useMaxError_ ? calibrationResult[6] : calibrationResult[5] < maxErrorTolerance_,
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

    void SwaptionVolCube1::fillVolatilityCube() const {

        const boost::shared_ptr<SwaptionVolatilityDiscrete> atmVolStructure =
            boost::dynamic_pointer_cast<SwaptionVolatilityDiscrete>(*atmVol_);

        std::vector<Time> atmOptionTimes(atmVolStructure->optionTimes());
        std::vector<Time> optionTimes(volCubeAtmCalibrated_.optionTimes());
        atmOptionTimes.insert(atmOptionTimes.end(),
                              optionTimes.begin(), optionTimes.end());
        std::sort(atmOptionTimes.begin(),atmOptionTimes.end());
        std::vector<Time>::iterator new_end =
            std::unique(atmOptionTimes.begin(), atmOptionTimes.end());
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
        std::vector<Date>::iterator new_end_1 =
            std::unique(atmOptionDates.begin(), atmOptionDates.end());
        atmOptionDates.erase(new_end_1, atmOptionDates.end());

        std::vector<Period> atmSwapTenors = atmVolStructure->swapTenors();
        std::vector<Period> swapTenors(volCubeAtmCalibrated_.swapTenors());
        atmSwapTenors.insert(atmSwapTenors.end(),
                             swapTenors.begin(), swapTenors.end());
        std::sort(atmSwapTenors.begin(),atmSwapTenors.end());
        std::vector<Period>::iterator new_end_2 =
            std::unique(atmSwapTenors.begin(), atmSwapTenors.end());
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


    void SwaptionVolCube1::createSparseSmiles() const {

        std::vector<Time> optionTimes(sparseParameters_.optionTimes());
        std::vector<Time> swapLengths(sparseParameters_.swapLengths());
        sparseSmiles_.clear();

        for (Size j=0; j<optionTimes.size(); j++) {
            std::vector<boost::shared_ptr<SmileSection> > tmp;
            Size n = swapLengths.size();
            tmp.reserve(n);
            for (Size k=0; k<n; ++k) {
                tmp.push_back(smileSection(optionTimes[j], swapLengths[k],
                                           sparseParameters_));
            }
            sparseSmiles_.push_back(tmp);
        }
    }


    std::vector<Real> SwaptionVolCube1::spreadVolInterpolation(
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

        std::vector< std::vector<boost::shared_ptr<SmileSection> > > smiles;
        std::vector<boost::shared_ptr<SmileSection> >  smilesOnPreviousExpiry;
        std::vector<boost::shared_ptr<SmileSection> >  smilesOnNextExpiry;

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

        Matrix atmForwards(2, 2, 0.0);
        Matrix atmVols(2, 2, 0.0);
        for (Size i=0; i<2; i++) {
            for (Size j=0; j<2; j++) {
                atmForwards[i][j] = atmStrike(optionsDateNodes[i],
                                              swapTenorNodes[j]);                
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
            const Real strike = std::max(atmForward + strikeSpreads_[k],MINSTRIKE);
            const Real moneyness = atmForward/strike;

            Matrix strikes(2,2,0.);
            Matrix spreadVols(2,2,0.);
            for (Size i=0; i<2; i++){
                for (Size j=0; j<2; j++){
                    strikes[i][j] = atmForwards[i][j]/moneyness;
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

    boost::shared_ptr<SmileSection>
    SwaptionVolCube1::smileSection(Time optionTime, Time swapLength,
                                   const Cube& sabrParametersCube) const {

        calculate();
        const std::vector<Real> sabrParameters =
            sabrParametersCube(optionTime, swapLength);
        return boost::shared_ptr<SmileSection>(new
            SabrSmileSection(optionTime, sabrParameters[4], sabrParameters));
    }

    boost::shared_ptr<SmileSection>
    SwaptionVolCube1::smileSectionImpl(Time optionTime,
                                       Time swapLength) const {
        if (isAtmCalibrated_)
            return smileSection(optionTime, swapLength, denseParameters_);
        else
            return smileSection(optionTime, swapLength, sparseParameters_);
    }

    Matrix SwaptionVolCube1::sparseSabrParameters() const {
        calculate();
        return sparseParameters_.browse();
    }

    Matrix SwaptionVolCube1::denseSabrParameters() const {
        calculate();
        return denseParameters_.browse();
    }

    Matrix SwaptionVolCube1::marketVolCube() const {
        calculate();
        return marketVolCube_.browse();
    }
    Matrix SwaptionVolCube1::volCubeAtmCalibrated() const {
        calculate();
        return volCubeAtmCalibrated_.browse();
    }

    void SwaptionVolCube1::recalibration(Real beta,
                                         const Period& swapTenor){
        Matrix newBetaGuess(nOptionTenors_, nSwapTenors_, beta);
        parametersGuess_.setLayer(1, newBetaGuess);
        parametersGuess_.updateInterpolators();

        sabrCalibrationSection(marketVolCube_,sparseParameters_,swapTenor);

        if(isAtmCalibrated_){
            fillVolatilityCube();
            sabrCalibrationSection(volCubeAtmCalibrated_,denseParameters_,swapTenor);
        }
    }

    //======================================================================//
    //                      SwaptionVolCube1::Cube                          //
    //======================================================================//


    SwaptionVolCube1::Cube::Cube(
                                    const std::vector<Date>& optionDates,
                                    const std::vector<Period>& swapTenors,
                                    const std::vector<Time>& optionTimes,
                                    const std::vector<Time>& swapLengths,
                                    Size nLayers,
                                    bool extrapolation)
    : optionTimes_(optionTimes), swapLengths_(swapLengths),
      optionDates_(optionDates), swapTenors_(swapTenors),
      nLayers_(nLayers), extrapolation_(extrapolation) {

        QL_REQUIRE(optionTimes.size()>1,"Cube::Cube(...): optionTimes.size()<2");
        QL_REQUIRE(swapLengths.size()>1,"Cube::Cube(...): swapLengths.size()<2");

        QL_REQUIRE(optionTimes.size()==optionDates.size(),
                   "Cube::Cube(...): optionTimes/optionDates mismatch");
        QL_REQUIRE(swapTenors.size()==swapLengths.size(),
                   "Cube::Cube(...): swapTenors/swapLengths mismatch");

        std::vector<Matrix> points(nLayers_, Matrix(optionTimes_.size(),
                                                    swapLengths_.size(), 0.0));

        for (Size k=0;k<nLayers_;k++) {
            transposedPoints_.push_back(transpose(points[k]));

            boost::shared_ptr<Interpolation2D> interpolation (new
                BilinearInterpolation (optionTimes_.begin(), optionTimes_.end(),
                                       swapLengths_.begin(), swapLengths_.end(),
                                       transposedPoints_[k]));
            interpolators_.push_back(boost::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation)));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(points);
     }

    SwaptionVolCube1::Cube::Cube(const Cube& o) {
        optionTimes_ = o.optionTimes_;
        swapLengths_ = o.swapLengths_;
        optionDates_ = o.optionDates_;
        swapTenors_ = o.swapTenors_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        transposedPoints_ = o.transposedPoints_;
        for (Size k=0; k<nLayers_; ++k) {
            boost::shared_ptr<Interpolation2D> interpolation (
                new BilinearInterpolation (optionTimes_.begin(), optionTimes_.end(),
                                           swapLengths_.begin(), swapLengths_.end(),
                                           transposedPoints_[k]));
            interpolators_.push_back(boost::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation)));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(o.points_);
    }

    SwaptionVolCube1::Cube&
    SwaptionVolCube1::Cube::operator=(const Cube& o) {
        optionTimes_ = o.optionTimes_;
        swapLengths_ = o.swapLengths_;
        optionDates_ = o.optionDates_;
        swapTenors_ = o.swapTenors_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        transposedPoints_ = o.transposedPoints_;
        for(Size k=0;k<nLayers_;k++){
            boost::shared_ptr<Interpolation2D> interpolation (
                new BilinearInterpolation (optionTimes_.begin(), optionTimes_.end(),
                                           swapLengths_.begin(), swapLengths_.end(),
                                           transposedPoints_[k]));
            interpolators_.push_back(boost::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation)));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(o.points_);
        return *this;
    }

    void SwaptionVolCube1::Cube::setElement(Size IndexOfLayer,
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

    void SwaptionVolCube1::Cube::setPoints(
                                               const std::vector<Matrix>& x) {
        QL_REQUIRE(x.size()==nLayers_,
            "Cube::setPoints: incompatible number of layers ");
        QL_REQUIRE(x[0].rows()==optionTimes_.size(),
            "Cube::setPoints: incompatible size 1");
        QL_REQUIRE(x[0].columns()==swapLengths_.size(),
            "Cube::setPoints: incompatible size 2");

        points_ = x;
    }

    void SwaptionVolCube1::Cube::setLayer(Size i,
                                                      const Matrix& x) {
        QL_REQUIRE(i<nLayers_,
            "Cube::setLayer: incompatible number of layer ");
        QL_REQUIRE(x.rows()==optionTimes_.size(),
            "Cube::setLayer: incompatible size 1");
        QL_REQUIRE(x.columns()==swapLengths_.size(),
            "Cube::setLayer: incompatible size 2");

        points_[i] = x;
    }

    void SwaptionVolCube1::Cube::setPoint(
                            const Date& optionDate, const Period& swapTenor,
                            const Real optionTime, const Time swapLength,
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

    void SwaptionVolCube1::Cube::expandLayers(
                                                 Size i, bool expandOptionTimes,
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

    const std::vector<Matrix>&
    SwaptionVolCube1::Cube::points() const {
        return points_;
    }

    std::vector<Real> SwaptionVolCube1::Cube::operator()(
                            const Time optionTime, const Time swapLength) const {
        std::vector<Real> result;
        for (Size k=0; k<nLayers_; ++k)
            result.push_back(interpolators_[k]->operator()(optionTime, swapLength));
        return result;
    }

    const std::vector<Time>&
    SwaptionVolCube1::Cube::optionTimes() const {
        return optionTimes_;
    }

    const std::vector<Time>&
    SwaptionVolCube1::Cube::swapLengths() const {
        return swapLengths_;
    }

    void SwaptionVolCube1::Cube::updateInterpolators() const {
        for (Size k=0; k<nLayers_; ++k) {
            transposedPoints_[k] = transpose(points_[k]);
            boost::shared_ptr<Interpolation2D> interpolation (
                new BilinearInterpolation (optionTimes_.begin(), optionTimes_.end(),
                                           swapLengths_.begin(), swapLengths_.end(),
                                           transposedPoints_[k]));
            interpolators_[k] = boost::shared_ptr<Interpolation2D>(
                new FlatExtrapolator2D(interpolation));
            interpolators_[k]->enableExtrapolation();
        }
    }

    Matrix SwaptionVolCube1::Cube::browse() const {
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

}
