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


#include <ql/Volatilities/swaptionvolcubebysabr.hpp>
#include <ql/Math/sabrinterpolation.hpp>
// NO!!!!!!!!!
#include <ql/Volatilities/swaptionvolmatrix.hpp>

namespace QuantLib {

    //=======================================================================//
    //                        SwaptionVolatilityCubeBySabr                   //
    //=======================================================================//

    SwaptionVolatilityCubeBySabr::SwaptionVolatilityCubeBySabr(
                const Handle<SwaptionVolatilityStructure>& atmVolStructure,
                const std::vector<Period>& expiries,
                const std::vector<Period>& lengths,
                const std::vector<Spread>& strikeSpreads,
                const std::vector<std::vector<Handle<Quote> > >& volSpreads,
                const boost::shared_ptr<SwapIndex>& swapIndexBase,
                bool vegaWeightedSmileFit,
                const Matrix& parametersGuess,
                std::vector<bool> isParameterFixed,
                bool isAtmCalibrated)
    : SwaptionVolatilityCube(atmVolStructure, expiries, lengths,
                             strikeSpreads, volSpreads, swapIndexBase,
                             vegaWeightedSmileFit),
      isParameterFixed_(isParameterFixed), isAtmCalibrated_(isAtmCalibrated)
    {

        parametersGuess_ = Cube(exerciseDates_, lengths_,
                                exerciseTimes_, timeLengths_, 4);
        Size i;
        for (i=0; i<4; i++) {
            for (Size j=0; j<nExercise_ ; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    parametersGuess_.setElement(
                                 i, j, k, parametersGuess[j+k*nExercise_][i]);
                }
            }
        }
        parametersGuess_.updateInterpolators();

        // ???
        atmVol_.currentLink()->enableExtrapolation();

        marketVolCube_ = Cube(exerciseDates_, lengths_,
                              exerciseTimes_, timeLengths_, nStrikes_);
        Rate atmForward;
        Volatility vol;
        for (Size i=0; i<nStrikes_; i++) {
            for (Size j=0; j<nExercise_; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    atmForward = atmStrike(exerciseDates_[j], lengths_[k]);
                    vol = volSpreads_[j*nlengths_+k][i]->value() +
                        atmVol_->volatility(exerciseDates_[j], lengths_[k],
                                                               atmForward);
                    marketVolCube_.setElement(i, j, k, vol);
                }
            }
        }
        marketVolCube_.updateInterpolators();
    }

    void SwaptionVolatilityCubeBySabr::performCalculations() const{
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

    SwaptionVolatilityCubeBySabr::Cube
    SwaptionVolatilityCubeBySabr::sabrCalibration(
                                            const Cube& marketVolCube) const {

        const std::vector<Time>& exerciseTimes = marketVolCube.expiries();
        const std::vector<Time>& timeLengths = marketVolCube.lengths();
        const std::vector<Date>& exerciseDates = marketVolCube.exerciseDates();
        const std::vector<Period>& swapTenors = marketVolCube.swapTenors();
        Matrix alphas(exerciseTimes.size(), timeLengths.size(),0.);
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

        for (Size j=0; j<exerciseTimes.size(); j++) {
            for (Size k=0; k<timeLengths.size(); k++) {
                Rate atmForward = atmStrike(exerciseDates[j], swapTenors[k]);
                for (Size i=0; i<nStrikes_; i++){
                    strikes[i] = atmForward+strikeSpreads_[i];
                    volatilities[i] = tmpMarketVolCube[i][j][k];
                }

                const std::vector<Real>& guess = parametersGuess_.operator()(
                    exerciseTimes[j], timeLengths[k]);

                const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
                    boost::shared_ptr<SABRInterpolation>(new
                        SABRInterpolation(strikes.begin(), strikes.end(),
                                          volatilities.begin(),
                                          exerciseTimes[j], atmForward,
                                          guess[0], guess[1],
                                          guess[2], guess[3],
                                          isParameterFixed_[0],
                                          isParameterFixed_[1],
                                          isParameterFixed_[2],
                                          isParameterFixed_[3],
                                          vegaWeightedSmileFit_,
                                          boost::shared_ptr<OptimizationMethod>()));

                Real interpolationError =
                    sabrInterpolation->interpolationError();
                alphas     [j][k]=sabrInterpolation->alpha();
                betas      [j][k]=sabrInterpolation->beta();
                nus        [j][k]=sabrInterpolation->nu();
                rhos       [j][k]=sabrInterpolation->rho();
                forwards   [j][k]=atmForward;
                errors     [j][k]=interpolationError;
                maxErrors  [j][k]=sabrInterpolation->interpolationMaxError();
                endCriteria[j][k]=sabrInterpolation->endCriteria();
                //QL_ENSURE(endCriteria[j][k]!=EndCriteria::maxIter,
                //          "option tenor " << exerciseDates[j] <<
                //          ", swap tenor " << swapTenors[k] <<
                //          ": max iteration");
                //QL_ENSURE(maxErrors[j][k]<15e-4,
                //          "option tenor " << exerciseDates[j] <<
                //          ", swap tenor " << swapTenors[k] <<
                //          ": max error " << maxErrors[j][k]);
            }
        }
        Cube sabrParametersCube(exerciseDates, swapTenors,
                                exerciseTimes, timeLengths, 8);
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
    void SwaptionVolatilityCubeBySabr::sabrCalibrationSection(
                                            const Cube& marketVolCube,
                                            const Period& swapTenor) const {

        const std::vector<Time>& exerciseTimes = marketVolCube.expiries();
        const std::vector<Time>& timeLengths = marketVolCube.lengths();
        const std::vector<Date>& exerciseDates = marketVolCube.exerciseDates();
        const std::vector<Period>& swapTenors = marketVolCube.swapTenors();

        Size k = std::find(swapTenors.begin(), swapTenors.end(),
                           swapTenor) - swapTenors.begin();
        QL_REQUIRE(k != swapTenors.size(), "swap tenor not found");

        std::vector<Real> calibrationResult(8,0.);
        const std::vector<Matrix>& tmpMarketVolCube = marketVolCube.points();

        std::vector<Real> strikes(strikeSpreads_.size());
        std::vector<Real> volatilities(strikeSpreads_.size());

        for (Size j=0; j<exerciseTimes.size(); j++) {
            Rate atmForward = atmStrike(exerciseDates[j], swapTenors[k]);
            for (Size i=0; i<nStrikes_; i++){
                strikes[i] = atmForward+strikeSpreads_[i];
                volatilities[i] = tmpMarketVolCube[i][j][k];
            }

            const std::vector<Real>& guess = parametersGuess_.operator()(
                exerciseTimes[j], timeLengths[k]);

            const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
                boost::shared_ptr<SABRInterpolation>(new
                    SABRInterpolation(strikes.begin(), strikes.end(),
                                        volatilities.begin(),
                                        exerciseTimes[j], atmForward,
                                        guess[0], guess[1],
                                        guess[2], guess[3],
                                        isParameterFixed_[0],
                                        isParameterFixed_[1],
                                        isParameterFixed_[2],
                                        isParameterFixed_[3],
                                        vegaWeightedSmileFit_,
                                        boost::shared_ptr<OptimizationMethod>()));

            Real interpolationError =
                sabrInterpolation->interpolationError();
            calibrationResult[0]=sabrInterpolation->alpha();
            calibrationResult[1]=sabrInterpolation->beta();
            calibrationResult[2]=sabrInterpolation->nu();
            calibrationResult[3]=sabrInterpolation->rho();
            calibrationResult[4]=atmForward;
            calibrationResult[5]=interpolationError;
            calibrationResult[6]=sabrInterpolation->interpolationMaxError();
            calibrationResult[7]=sabrInterpolation->endCriteria();
            
            QL_ENSURE(calibrationResult[7]!=EndCriteria::maxIter,
                      "option tenor " << exerciseDates[j] <<
                      ", swap tenor " << swapTenors[k] <<
                      ": max iteration");
            //QL_ENSURE(maxErrors[j][k]<15e-4,
            //          "option tenor " << exerciseDates[j] <<
            //          ", swap tenor " << swapTenors[k] <<
            //          ": max error " << maxErrors[j][k]);

            sparseParameters_.setPoint(exerciseDates[j], swapTenors[k],
                                    exerciseTimes[j], timeLengths[k],
                                    calibrationResult);
            sparseParameters_.updateInterpolators();
            if(isAtmCalibrated_){
                denseParameters_.setPoint(exerciseDates[j], swapTenors[k],
                                        exerciseTimes[j], timeLengths[k],
                                        calibrationResult);
                denseParameters_.updateInterpolators();
            }

        }

    }

    void SwaptionVolatilityCubeBySabr::fillVolatilityCube() const{

        // NO!!!!!!
        const boost::shared_ptr<SwaptionVolatilityMatrix> atmVolStructure =
            boost::dynamic_pointer_cast<SwaptionVolatilityMatrix>(
                                                    atmVol_.currentLink());
        //const boost::shared_ptr<SwaptionVolatilityStructure> atmVolStructure = atmVol_;

        std::vector<Time> atmExerciseTimes(atmVolStructure->exerciseTimes());
        std::vector<Time> exerciseTimes(volCubeAtmCalibrated_.expiries());
        atmExerciseTimes.insert(atmExerciseTimes.end(),
                                exerciseTimes.begin(), exerciseTimes.end());
        std::sort(atmExerciseTimes.begin(),atmExerciseTimes.end());

        std::vector<Time> atmTimeLengths(atmVolStructure->timeLengths());
        std::vector<Time> timeLengths(volCubeAtmCalibrated_.lengths());
        atmTimeLengths.insert(atmTimeLengths.end(),
                              timeLengths.begin(), timeLengths.end());
        std::sort(atmTimeLengths.begin(),atmTimeLengths.end());

        std::vector<Date> atmExerciseDates = atmVolStructure->exerciseDates();
        std::vector<Date> exerciseDates(volCubeAtmCalibrated_.exerciseDates());
        atmExerciseDates.insert(atmExerciseDates.end(),
                                exerciseDates.begin(), exerciseDates.end());
        std::sort(atmExerciseDates.begin(),atmExerciseDates.end());

        std::vector<Period> atmSwapTenors = atmVolStructure->lengths();
        std::vector<Period> swapTenors(volCubeAtmCalibrated_.swapTenors());
        atmSwapTenors.insert(atmSwapTenors.end(),
                             swapTenors.begin(), swapTenors.end());
        std::sort(atmSwapTenors.begin(),atmSwapTenors.end());

        createSparseSmiles();

        for (Size j=0; j<atmExerciseTimes.size(); j++) {

            for (Size k=0; k<atmTimeLengths.size(); k++) {
                bool expandExpiries =
                    !(std::binary_search(exerciseTimes.begin(),
                                         exerciseTimes.end(),
                                         atmExerciseTimes[j]));
                bool expandLengths =
                    !(std::binary_search(timeLengths.begin(),
                                         timeLengths.end(),
                                         atmTimeLengths[k]));
                if(expandExpiries || expandLengths){
                    Rate atmForward = atmStrike(atmExerciseDates[j],
                                                atmSwapTenors[k]);
                    Volatility atmVol = atmVol_->volatility(
                        atmExerciseDates[j], atmSwapTenors[k], atmForward);
                    std::vector<Real> spreadVols =
                        spreadVolInterpolation(atmExerciseDates[j],
                                               atmSwapTenors[k]);
                    std::vector<Real> volAtmCalibrated;
                    for (Size i=0; i<nStrikes_; i++)
                        volAtmCalibrated.push_back(atmVol + spreadVols[i]);
                    volCubeAtmCalibrated_.setPoint(
                                    atmExerciseDates[j], atmSwapTenors[k],
                                    atmExerciseTimes[j], atmTimeLengths[k],
                                    volAtmCalibrated);
                }
            }
        }
        volCubeAtmCalibrated_.updateInterpolators();
    }


    void SwaptionVolatilityCubeBySabr::createSparseSmiles() const {

        std::vector<Time> exerciseTimes(sparseParameters_.expiries());
        std::vector<Time> timeLengths(sparseParameters_.lengths());

        for (Size j=0; j<exerciseTimes.size(); j++) {
            std::vector<boost::shared_ptr<SmileSectionInterface> > tmp;
            for (Size k=0; k<timeLengths.size(); k++) {
                tmp.push_back(smileSection(exerciseTimes[j], timeLengths[k],
                                           sparseParameters_));
            }
            sparseSmiles_.push_back(tmp);
        }
    }


    std::vector<Real> SwaptionVolatilityCubeBySabr::spreadVolInterpolation(
        const Date& atmExerciseDate, const Period& atmSwapTenor) const {

        const std::pair<Time, Time> p =
            convertDates(atmExerciseDate, atmSwapTenor);
        Time atmExerciseTime = p.first, atmTimeLength = p.second;

        std::vector<Real> result;
        const std::vector<Time>& exerciseTimes(sparseParameters_.expiries());
        const std::vector<Time>& timeLengths(sparseParameters_.lengths());
        const std::vector<Date>& exerciseDates =
            sparseParameters_.exerciseDates();
        const std::vector<Period>& swapTenors = sparseParameters_.swapTenors();

        std::vector<Real>::const_iterator expiriesPreviousNode,
                                          lengthsPreviousNode;

        expiriesPreviousNode = std::lower_bound(exerciseTimes.begin(),
                                                exerciseTimes.end(),
                                                atmExerciseTime);
        Size expiriesPreviousIndex
            = expiriesPreviousNode - exerciseTimes.begin();
        if (expiriesPreviousIndex >= exerciseTimes.size()-1)
            expiriesPreviousIndex = exerciseTimes.size()-2;

        lengthsPreviousNode = std::lower_bound(timeLengths.begin(),
                                               timeLengths.end(),
                                               atmTimeLength);
        Size lengthsPreviousIndex = lengthsPreviousNode - timeLengths.begin();
        if (lengthsPreviousIndex >= timeLengths.size()-1)
            lengthsPreviousIndex = timeLengths.size()-2;

        std::vector< std::vector<boost::shared_ptr<SmileSectionInterface> > > smiles;
        std::vector<boost::shared_ptr<SmileSectionInterface> >  smilesOnPreviousExpiry;
        std::vector<boost::shared_ptr<SmileSectionInterface> >  smilesOnNextExpiry;

        QL_REQUIRE(expiriesPreviousIndex+1 < sparseSmiles_.size(),
                   "expiriesPreviousIndex+1 >= sparseSmiles_.size()");
        QL_REQUIRE(lengthsPreviousIndex+1 < sparseSmiles_[0].size(),
                   "lengthsPreviousIndex+1 >= sparseSmiles_[0].size()");
        smilesOnPreviousExpiry.push_back(
              sparseSmiles_[expiriesPreviousIndex][lengthsPreviousIndex]);
        smilesOnPreviousExpiry.push_back(
              sparseSmiles_[expiriesPreviousIndex][lengthsPreviousIndex+1]);
        smilesOnNextExpiry.push_back(
              sparseSmiles_[expiriesPreviousIndex+1][lengthsPreviousIndex]);
        smilesOnNextExpiry.push_back(
              sparseSmiles_[expiriesPreviousIndex+1][lengthsPreviousIndex+1]);

        smiles.push_back(smilesOnPreviousExpiry);
        smiles.push_back(smilesOnNextExpiry);

        std::vector<Real> exercisesNodes(2);
        exercisesNodes[0] = exerciseTimes[expiriesPreviousIndex];
        exercisesNodes[1] = exerciseTimes[expiriesPreviousIndex+1];

        std::vector<Date> exercisesDateNodes(2);
        exercisesDateNodes[0] = exerciseDates[expiriesPreviousIndex];
        exercisesDateNodes[1] = exerciseDates[expiriesPreviousIndex+1];

        std::vector<Real> lengthsNodes(2);
        lengthsNodes[0] = timeLengths[lengthsPreviousIndex];
        lengthsNodes[1] = timeLengths[lengthsPreviousIndex+1];

        std::vector<Period> swapTenorNodes(2);
        swapTenorNodes[0] = swapTenors[lengthsPreviousIndex];
        swapTenorNodes[1] = swapTenors[lengthsPreviousIndex+1];

        const Rate atmForward = atmStrike(atmExerciseDate, atmSwapTenor);

        Matrix atmForwards(2, 2, 0.0);
        Matrix atmVols(2, 2, 0.0);
        for (Size i=0; i<2; i++) {
            for (Size j=0; j<2; j++) {
                atmForwards[i][j] = atmStrike(exercisesDateNodes[i],
                                              swapTenorNodes[j]);
                atmVols[i][j] = smiles[i][j]->volatility(atmForwards[i][j]);
            }
        }

        for (Size k=0; k<nStrikes_; k++){
            const Real strike = atmForward + strikeSpreads_[k];
            const Real moneyness = atmForward/strike;

            Matrix strikes(2,2,0.);
            Matrix spreadVols(2,2,0.);
            for (Size i=0; i<2; i++){
                for (Size j=0; j<2; j++){
                    strikes[i][j] = atmForwards[i][j]/moneyness;
                    spreadVols[i][j] =
                        smiles[i][j]->volatility(strikes[i][j])-atmVols[i][j];
                }
            }
           Cube localInterpolator(exercisesDateNodes, swapTenorNodes,
                                  exercisesNodes, lengthsNodes, 1);
           localInterpolator.setLayer(0, spreadVols);
           localInterpolator.updateInterpolators();

           result.push_back(
                        localInterpolator(atmExerciseTime, atmTimeLength)[0]);
        }
        return result;
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeBySabr::smileSection(Time expiry,
                                               Time length) const {
        if (isAtmCalibrated_)
            return smileSection(expiry, length, denseParameters_);
        else
            return smileSection(expiry, length, sparseParameters_);
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeBySabr::smileSection(
                                    Time expiry, Time length,
                                    const Cube& sabrParametersCube) const {

        calculate();
        const std::vector<Real> sabrParameters =
            sabrParametersCube(expiry, length);
        return boost::shared_ptr<SmileSectionInterface>(new
            SabrSmileSection(expiry, sabrParameters));
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeBySabr::smileSection(
        const Date& exerciseDate, const Period& swapTenor) const {
        const std::pair<Time, Time> p = convertDates(exerciseDate, swapTenor);
        if (isAtmCalibrated_)
            return smileSection(p.first, p.second, denseParameters_);
        else
            return smileSection(p.first, p.second, sparseParameters_);
    }

    Matrix SwaptionVolatilityCubeBySabr::sparseSabrParameters() const {
        calculate();
        return sparseParameters_.browse();
    }

    Matrix SwaptionVolatilityCubeBySabr::denseSabrParameters() const {
        calculate();
        return denseParameters_.browse();
    }

    Matrix SwaptionVolatilityCubeBySabr::marketVolCube() const {
        calculate();
        return marketVolCube_.browse();
    }
    Matrix SwaptionVolatilityCubeBySabr::volCubeAtmCalibrated() const {
        calculate();
        return volCubeAtmCalibrated_.browse();
    }

    void SwaptionVolatilityCubeBySabr::recalibration(Real beta,
                                                     const Period& swapTenor){
        Matrix newBetaGuess(nExercise_, nlengths_, beta);
        parametersGuess_.setLayer(1, newBetaGuess);
        parametersGuess_.updateInterpolators();

        sabrCalibrationSection(marketVolCube_,swapTenor);

        if(isAtmCalibrated_){
            fillVolatilityCube();
            sabrCalibrationSection(volCubeAtmCalibrated_,swapTenor);
        }
    }

    //======================================================================//
    //                      SwaptionVolatilityCubeBySabr::Cube              //
    //======================================================================//


    SwaptionVolatilityCubeBySabr::Cube::Cube(
                                    const std::vector<Date>& exerciseDates,
                                    const std::vector<Period>& swapTenors,
                                    const std::vector<Time>& expiries,
                                    const std::vector<Time>& lengths,
                                    Size nLayers,
                                    bool extrapolation)
    : expiries_(expiries), lengths_(lengths),
      exerciseDates_(exerciseDates), swapTenors_(swapTenors),
      nLayers_(nLayers), extrapolation_(extrapolation) {

        QL_REQUIRE(expiries.size()>1,"Cube::Cube(...): expiries.size()<2");
        QL_REQUIRE(lengths.size()>1,"Cube::Cube(...): lengths.size()<2");

        QL_REQUIRE(expiries.size()==exerciseDates.size(),
                   "Cube::Cube(...): expiries/exerciseDates mismatch");
        QL_REQUIRE(swapTenors.size()==lengths.size(),
                   "Cube::Cube(...): swapTenors/lengths mismatch");

        std::vector<Matrix> points(nLayers_, Matrix(expiries_.size(),
                                                    lengths_.size(), 0.0));

        for (Size k=0;k<nLayers_;k++) {
            transposedPoints_.push_back(transpose(points[k]));
            interpolators_.push_back(boost::shared_ptr<BilinearInterpolation>(
                new BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(),
                                        lengths_.end(),transposedPoints_[k])));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(points);
     }

    SwaptionVolatilityCubeBySabr::Cube::Cube(const Cube& o) {
        expiries_ = o.expiries_;
        lengths_ = o.lengths_;
        exerciseDates_ = o.exerciseDates_;
        swapTenors_ = o.swapTenors_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        transposedPoints_ = o.transposedPoints_;
        for (Size k=0; k<nLayers_; ++k) {
            interpolators_.push_back(boost::shared_ptr<BilinearInterpolation>(
                new BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                          lengths_.begin(), lengths_.end(),
                                          transposedPoints_[k])));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(o.points_);
    }

    SwaptionVolatilityCubeBySabr::Cube&
    SwaptionVolatilityCubeBySabr::Cube::operator=(const Cube& o) {
        expiries_ = o.expiries_;
        lengths_ = o.lengths_;
        exerciseDates_ = o.exerciseDates_;
        swapTenors_ = o.swapTenors_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        transposedPoints_ = o.transposedPoints_;
        for(Size k=0;k<nLayers_;k++){
            interpolators_.push_back(boost::shared_ptr<BilinearInterpolation>(
                new BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                          lengths_.begin(), lengths_.end(),
                                          transposedPoints_[k])));
            interpolators_[k]->enableExtrapolation();
        }
        setPoints(o.points_);
        return *this;
    }

    void SwaptionVolatilityCubeBySabr::Cube::setElement(Size IndexOfLayer,
                                                        Size IndexOfRow,
                                                        Size IndexOfColumn,
                                                        Real x) {
        QL_REQUIRE(IndexOfLayer<nLayers_,
                   "incompatible IndexOfLayer ");
        QL_REQUIRE(IndexOfRow<expiries_.size(),
                   "incompatible IndexOfRow");
        QL_REQUIRE(IndexOfColumn<lengths_.size(),
                   "incompatible IndexOfColumn");
        points_[IndexOfLayer][IndexOfRow][IndexOfColumn] = x;
    }

    void SwaptionVolatilityCubeBySabr::Cube::setPoints(
                                               const std::vector<Matrix>& x) {
        QL_REQUIRE(x.size()==nLayers_,
                   "incompatible number of layers ");
        QL_REQUIRE(x[0].rows()==expiries_.size(),
                   "incompatible size 1");
        QL_REQUIRE(x[0].columns()==lengths_.size(),
                   "incompatible size 2");

        points_ = x;
    }

    void SwaptionVolatilityCubeBySabr::Cube::setLayer(Size i,
                                                      const Matrix& x) {
        QL_REQUIRE(i<nLayers_,
                   "incompatible number of layer ");
        QL_REQUIRE(x.rows()==expiries_.size(),
                   "incompatible size 1");
        QL_REQUIRE(x.columns()==lengths_.size(),
                   "incompatible size 2");

        points_[i] = x;
    }

    void SwaptionVolatilityCubeBySabr::Cube::setPoint(
                            const Date& exerciseDate, const Period& swapTenor,
                            const Real expiry, const Time length,
                            const std::vector<Real>& point)
    {
        const bool expandExpiries =
            !(std::binary_search(expiries_.begin(),expiries_.end(),expiry));
        const bool expandLengths =
            !(std::binary_search(lengths_.begin(),lengths_.end(),length));

        std::vector<Real>::const_iterator expiriesPreviousNode,
                                          lengthsPreviousNode;

        expiriesPreviousNode =
            std::lower_bound(expiries_.begin(),expiries_.end(),expiry);
        Size expiriesIndex = expiriesPreviousNode - expiries_.begin();

        lengthsPreviousNode =
            std::lower_bound(lengths_.begin(),lengths_.end(),length);
        Size lengthsIndex = lengthsPreviousNode - lengths_.begin();

        if (expandExpiries || expandLengths)
            expandLayers(expiriesIndex, expandExpiries,
                         lengthsIndex, expandLengths);

        for (Size k=0; k<nLayers_; ++k)
            points_[k][expiriesIndex][lengthsIndex] = point[k];

        expiries_[expiriesIndex] = expiry;
        lengths_[lengthsIndex] = length;
        exerciseDates_[expiriesIndex] = exerciseDate;
        swapTenors_[lengthsIndex] = swapTenor;
    }

    void SwaptionVolatilityCubeBySabr::Cube::expandLayers(
                                                 Size i, bool expandExpiries,
                                                 Size j, bool expandLengths) {
        QL_REQUIRE(i<=expiries_.size(),"incompatible size 1");
        QL_REQUIRE(j<=lengths_.size(),"incompatible size 2");

        if (expandExpiries) {
            expiries_.insert(expiries_.begin()+i,0.);
            exerciseDates_.insert(exerciseDates_.begin()+i, Date());
        }
        if (expandLengths) {
            lengths_.insert(lengths_.begin()+j,0.);
            swapTenors_.insert(swapTenors_.begin()+j, Period());
        }

        std::vector<Matrix> newPoints(nLayers_,Matrix(expiries_.size(),
                                                      lengths_.size(), 0.));

        for (Size k=0; k<nLayers_; ++k) {
            for (Size u=0; u<points_[k].rows(); ++u) {
                 Size indexOfRow = u;
                 if (u>=i && expandExpiries) indexOfRow = u+1;
                 for (Size v=0; v<points_[k].columns(); ++v) {
                      Size indexOfCol = v;
                      if (v>=j && expandLengths) indexOfCol = v+1;
                      newPoints[k][indexOfRow][indexOfCol]=points_[k][u][v];
                 }
            }
        }
        setPoints(newPoints);
    }

    const std::vector<Matrix>&
    SwaptionVolatilityCubeBySabr::Cube::points() const {
        return points_;
    }

    std::vector<Real> SwaptionVolatilityCubeBySabr::Cube::operator()(
                            const Time expiry, const Time length) const {
        std::vector<Real> result;
        for (Size k=0; k<nLayers_; ++k)
            result.push_back(interpolators_[k]->operator()(expiry, length));
        return result;
    }

    const std::vector<Time>&
    SwaptionVolatilityCubeBySabr::Cube::expiries() const {
        return expiries_;
    }

    const std::vector<Time>&
    SwaptionVolatilityCubeBySabr::Cube::lengths() const {
        return lengths_;
    }

    void SwaptionVolatilityCubeBySabr::Cube::updateInterpolators() const {
        for (Size k=0; k<nLayers_; ++k) {
            transposedPoints_[k] = transpose(points_[k]);
            interpolators_[k] =
                boost::shared_ptr<BilinearInterpolation>(new
                    BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                          lengths_.begin(), lengths_.end(),
                                          transposedPoints_[k]));
            interpolators_[k]->enableExtrapolation();
        }
    }

    Matrix SwaptionVolatilityCubeBySabr::Cube::browse() const {
        Matrix result(lengths_.size()*expiries_.size(), nLayers_+2, 0.0);
        for (Size i=0; i<lengths_.size(); ++i) {
            for (Size j=0; j<expiries_.size(); ++j) {
                result[i*expiries_.size()+j][0] = lengths_[i];
                result[i*expiries_.size()+j][1] = expiries_[j];
                for (Size k=0; k<nLayers_; ++k)
                    result[i*expiries_.size()+j][2+k] = points_[k][j][i];
            }
        }
        return result;
    }

}
