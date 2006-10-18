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
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/Volatilities/smilesection.hpp>

#include <fstream>
#include <string>

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
        const Calendar& calendar,
        Integer swapSettlementDays,
        Frequency fixedLegFrequency,
        BusinessDayConvention fixedLegConvention,
        const DayCounter& fixedLegDayCounter,
        const boost::shared_ptr<Xibor>& iborIndex,
        Time shortTenor,
        const boost::shared_ptr<Xibor>& iborIndexShortTenor,
        const Matrix& parametersGuess,
        std::vector<bool> isParameterFixed,
        bool isAtmCalibrated):
        SwaptionVolatilityCube(
            atmVolStructure,
            expiries,
            lengths,
            strikeSpreads,
            calendar,
            swapSettlementDays,
            fixedLegFrequency,
            fixedLegConvention,
            fixedLegDayCounter,
            iborIndex,
            shortTenor,
            iborIndexShortTenor),
        volSpreads_(volSpreads),
        isParameterFixed_(isParameterFixed),
        isAtmCalibrated_(isAtmCalibrated) {

        QL_REQUIRE(!volSpreads_.empty(), "empty vol spreads matrix");

        QL_REQUIRE(nStrikes_== volSpreads_[0].size(),
                   "mismatch between number of strikes ("
                   << nStrikes_ << ") and number of columns ("
                   << volSpreads_[0].size() << ").");

        QL_REQUIRE(nExercise_*nlengths_==volSpreads_.size(),
                 "mismatch between number of option expiries * swap tenors ("
                 << nExercise_*nlengths_ << ") and number of rows ("
                 << volSpreads_.size() <<")");

        parametersGuess_ = Cube(exerciseDates_, lengths_,
                                exerciseTimes_, timeLengths_, 4);
        for (Size i=0; i<4; i++) {
            for (Size j=0; j<nExercise_ ; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    parametersGuess_.setElement(
                                 i, j, k, parametersGuess[j+k*nExercise_][i]);
                }
            }
        }
        parametersGuess_.updateInterpolators();

        atmVolStructure_.currentLink()->enableExtrapolation();
        marketVolCube_ = Cube(exerciseDates_, lengths_,
                              exerciseTimes_, timeLengths_, nStrikes_);
        for (Size i=0; i<nStrikes_; i++) {
            for (Size j=0; j<nExercise_; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    const Rate atmForward =
                        atmStrike(exerciseDates_[j], lengths_[k]);
                    const Volatility atmVol =
                        atmVolStructure_->volatility(exerciseDates_[j],
                                                     lengths_[k], atmForward);
                    const Volatility vol =
                        atmVol + volSpreads_[j*nlengths_+k][i]->value();
                    registerWith(volSpreads_[j*nlengths_+k][i]);
                    marketVolCube_.setElement(i, j, k, vol);
                }
            }
        }
        marketVolCube_.updateInterpolators();
    }

    void SwaptionVolatilityCubeBySabr::performCalculations() const{
        sparseParameters_ = sabrCalibration(marketVolCube_);
        sparseParameters_.updateInterpolators();
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

        const std::vector<Matrix>& tmpMarketVolCube = marketVolCube.points();

        for (Size j=0; j<exerciseTimes.size(); j++) {
            for (Size k=0; k<timeLengths.size(); k++) {
                const Rate atmForward =
                    atmStrike(exerciseDates[j], swapTenors[k]);
                std::vector<Real> strikes, volatilities;
                for (Size i=0; i<nStrikes_; i++){
                    strikes.push_back(atmForward+strikeSpreads_[i]);
                    volatilities.push_back(tmpMarketVolCube[i][j][k]);
                }

                const std::vector<Real>& guess = parametersGuess_.operator()(
                    exerciseTimes[j], timeLengths[k]);

                const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
                    boost::shared_ptr<SABRInterpolation>(
                        new SABRInterpolation(
                                   strikes.begin(), strikes.end(),
                                   volatilities.begin(),
                                   exerciseTimes[j], atmForward,
                                   guess[0], guess[1], guess[2], guess[3],
                                   isParameterFixed_[0], isParameterFixed_[1],
                                   isParameterFixed_[2], isParameterFixed_[3],
                                   boost::shared_ptr<OptimizationMethod>()));

                const Real interpolationError =
                    sabrInterpolation->interpolationError();
                alphas[j][k]= sabrInterpolation->alpha();
                betas[j][k]= sabrInterpolation->beta();
                nus[j][k]= sabrInterpolation->nu();
                rhos[j][k]= sabrInterpolation->rho();
                forwards[j][k]= atmForward;
                errors[j][k]= interpolationError;
                maxErrors[j][k]= sabrInterpolation->interpolationMaxError();

/*                QL_REQUIRE(sabrInterpolation->endCriteria() != EndCriteria::maxIter,
                    "SwaptionVolatilityCubeBySabr::sabrCalibration: end criteria is max iteration"); */   /*                QL_REQUIRE(sabrInterpolation->endCriteria != EndCriteria::maxIter,
                    "SwaptionVolatilityCubeBySabr::sabrCalibration: end criteria is max iteration");*/

            }
        }
        Cube sabrParametersCube(exerciseDates, swapTenors,
                                exerciseTimes, timeLengths, 7);
        sabrParametersCube.setLayer(0, alphas);
        sabrParametersCube.setLayer(1, betas);
        sabrParametersCube.setLayer(2, nus);
        sabrParametersCube.setLayer(3, rhos);
        sabrParametersCube.setLayer(4, forwards);
        sabrParametersCube.setLayer(5, errors);
        sabrParametersCube.setLayer(6, maxErrors);

        return sabrParametersCube;

    }

    void SwaptionVolatilityCubeBySabr::fillVolatilityCube() const{

        const boost::shared_ptr<SwaptionVolatilityMatrix> atmVolStructure =
            boost::dynamic_pointer_cast<SwaptionVolatilityMatrix>(
                                              atmVolStructure_.currentLink());

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
                const bool expandExpiries =
                    !(std::binary_search(exerciseTimes.begin(),
                                         exerciseTimes.end(),
                                         atmExerciseTimes[j]));
                const bool expandLengths =
                    !(std::binary_search(timeLengths.begin(),
                                         timeLengths.end(),
                                         atmTimeLengths[k]));
                if(expandExpiries || expandLengths){
                    const Rate atmForward = atmStrike(atmExerciseDates[j],
                                                      atmSwapTenors[k]);
                    const Volatility atmVol =
                        atmVolStructure_->volatility(atmExerciseDates[j],
                                                     atmSwapTenors[k],
                                                     atmForward);
                    const std::vector<Real> spreadVols =
                        spreadVolInterpolation(atmExerciseDates[j],
                                               atmSwapTenors[k]);
                    std::vector<Real> volAtmCalibrated;
                    for (Size i=0; i<nStrikes_; i++){
                        volAtmCalibrated.push_back(atmVol + spreadVols[i]);
                    }
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
            std::vector<boost::shared_ptr<SmileSection> > tmp;
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
        std::vector<Time> exerciseTimes(sparseParameters_.expiries());
        std::vector<Time> timeLengths(sparseParameters_.lengths());
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

        std::vector< std::vector<boost::shared_ptr<SmileSection> > > smiles;
        std::vector<boost::shared_ptr<SmileSection> >  smilesOnPreviousExpiry;
        std::vector<boost::shared_ptr<SmileSection> >  smilesOnNextExpiry;

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

    Volatility SwaptionVolatilityCubeBySabr::volatilityImpl(
                            Time expiry, Time length, Rate strike) const {
            return smileSection(expiry, length)->volatility(strike);
    }

    boost::shared_ptr<SmileSection>
    SwaptionVolatilityCubeBySabr::smileSection(Time expiry,
                                               Time length) const {
        if (isAtmCalibrated_)
            return smileSection(expiry, length, denseParameters_);
        else
            return smileSection(expiry, length, sparseParameters_);
    }

    boost::shared_ptr<SmileSection>
    SwaptionVolatilityCubeBySabr::smileSection(
                                    Time expiry, Time length,
                                    const Cube& sabrParametersCube) const {
        
        calculate();        
        const std::vector<Real> sabrParameters =
            sabrParametersCube(expiry, length);
        return boost::shared_ptr<SmileSection>(
            new SmileSection(sabrParameters, expiry));
    }

    Volatility SwaptionVolatilityCubeBySabr::volatilityImpl(
        const Date& exerciseDate, const Period& length, Rate strike) const {
            return smileSection(exerciseDate, length)->volatility(strike);
    }

    boost::shared_ptr<SmileSection>
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

    void SwaptionVolatilityCubeBySabr::recalibration(Real beta){
        Matrix newBetaGuess(nExercise_, nlengths_, beta);
        parametersGuess_.setLayer(1, newBetaGuess);
        parametersGuess_.updateInterpolators();

        sparseParameters_ = sabrCalibration(marketVolCube_);
        sparseParameters_.updateInterpolators();

        if(isAtmCalibrated_){
            volCubeAtmCalibrated_= marketVolCube_;
            fillVolatilityCube();
            denseParameters_ = sabrCalibration(volCubeAtmCalibrated_);
            denseParameters_.updateInterpolators();
        }
    }

    //======================================================================//
    //                      SwaptionVolatilityCubeBySabr::Cube              //
    //======================================================================//


    SwaptionVolatilityCubeBySabr::Cube::Cube(
                                    const std::vector<Date>& exerciseDates,
                                    const std::vector<Period>& swapTenors,
                                    const std::vector<Real>& expiries,
                                    const std::vector<Real>& lengths,
                                    Size nLayers,
                                    bool extrapolation)
    : expiries_(expiries), lengths_(lengths),
      exerciseDates_(exerciseDates), swapTenors_(swapTenors),
      nLayers_(nLayers), extrapolation_(extrapolation) {

        QL_REQUIRE(expiries.size()>1,"Cube::Cube(...): wrong input expiries");
        QL_REQUIRE(lengths.size()>1,"Cube::Cube(...): wrong input lengths");

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
        for(Size k=0;k<nLayers_;k++){
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
                            const Real expiry, const Real length,
                            const std::vector<Real>& point) {

        const bool expandExpiries =
            !(std::binary_search(expiries_.begin(),expiries_.end(),expiry));
        const bool expandLengths =
            !(std::binary_search(lengths_.begin(),lengths_.end(),length));

        std::vector<Real>::const_iterator expiriesPreviousNode,
                                          lengthsPreviousNode;

        expiriesPreviousNode =
            std::lower_bound(expiries_.begin(),expiries_.end(),expiry);
        std::vector<Real>::iterator::difference_type
            expiriesIndex = expiriesPreviousNode - expiries_.begin();

        lengthsPreviousNode =
            std::lower_bound(lengths_.begin(),lengths_.end(),length);
        std::vector<Real>::iterator::difference_type
            lengthsIndex = lengthsPreviousNode - lengths_.begin();

        if (expandExpiries || expandLengths)
            expandLayers(expiriesIndex, expandExpiries,
                         lengthsIndex, expandLengths);

        for(Size k=0;k<nLayers_;k++)
            points_[k][expiriesIndex][lengthsIndex]= point[k];

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

        for (Size k=0;k<nLayers_;k++) {
            for (Size u=0;u<points_[k].rows();u++) {
                 Size indexOfRow = u;
                 if (u>=i && expandExpiries) indexOfRow = u+1;
                 for (Size v=0;v<points_[k].columns();v++) {
                      Size indexOfCol = v;
                      if(v>=j && expandLengths) indexOfCol = v+1;
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
        const Real expiry, const Real length) const {
        std::vector<Real> result;
        for(Size k=0;k<nLayers_;k++){
            result.push_back(interpolators_[k]->operator()(expiry, length));
        }
        return result;
    }

    const std::vector<Real>&
    SwaptionVolatilityCubeBySabr::Cube::expiries() const {
        return expiries_;
    }

    const std::vector<Real>&
    SwaptionVolatilityCubeBySabr::Cube::lengths() const {
        return lengths_;
    }

    void SwaptionVolatilityCubeBySabr::Cube::updateInterpolators() const {
        for (Size k=0;k<nLayers_;k++) {
            transposedPoints_[k]=transpose(points_[k]);
            interpolators_[k] =
                boost::shared_ptr<BilinearInterpolation>(
                 new BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                           lengths_.begin(), lengths_.end(),
                                           transposedPoints_[k]));

            interpolators_[k]->enableExtrapolation();
        }
    }

    Matrix SwaptionVolatilityCubeBySabr::Cube::browse() const{
        Matrix result(lengths_.size()*expiries_.size(),nLayers_+2,0.);
        for (Size i=0;i<lengths_.size();i++) {
            for (Size j=0;j<expiries_.size();j++) {
                result[i*expiries_.size()+j][0]= lengths_[i];
                result[i*expiries_.size()+j][1]= expiries_[j];
                for (Size k=0;k<nLayers_;k++) {
                    result[i*expiries_.size()+j][2+k]= points_[k][j][i];
                }
            }
        }
        return result;
    }

}

