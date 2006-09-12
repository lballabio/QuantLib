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

    //===========================================================================//
    //                          SwaptionVolatilityCubeBySabr                     //
    //===========================================================================//

    SwaptionVolatilityCubeBySabr::SwaptionVolatilityCubeBySabr(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const Matrix& volSpreads,
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
        isParameterFixed_(isParameterFixed),
        isAtmCalibrated_(isAtmCalibrated),
        volSpreads_(volSpreads) {

        for (Size i=0; i<nStrikes_; i++) {
            fictitiousStrikes_.push_back(0.05*i+.01);
        }

        QL_REQUIRE(nStrikes_==volSpreads_.columns(),
                   "nStrikes_!=marketVolCube.columns()");
        QL_REQUIRE(nExercise_*nlengths_==volSpreads_.rows(),
                   "nExercise*nlengths!=marketVolCube.rows()");
        
        parametersGuess_ = Cube(exerciseTimes_, timeLengths_, 4);
        for (Size i=0; i<4; i++) {
            for (Size j=0; j<nExercise_ ; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    parametersGuess_.setElement(i, j, k, parametersGuess[j+k*nExercise_][i]);
                }
            }
        }
        parametersGuess_.updateInterpolators();

        atmVolStructure_.currentLink()->enableExtrapolation();
        marketVolCube_ = Cube(exerciseTimes_, timeLengths_, nStrikes_);
        for (Size i=0; i<nStrikes_; i++) {
            for (Size j=0; j<nExercise_; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    const Rate atmForward = atmStrike(exerciseTimes_[j], timeLengths_[k]);        
                    const Volatility atmVol = 
                        atmVolStructure_->volatility(exerciseTimes_[j], timeLengths_[k], atmForward);
                    const Volatility vol = atmVol + volSpreads_[j*nlengths_+k][i];
                    marketVolCube_.setElement(i, j, k, vol);
                }
            }
        }
        marketVolCube_.updateInterpolators();

        sparseParameters_ = sabrCalibration(marketVolCube_);
        sparseParameters_.updateInterpolators();
        volCubeAtmCalibrated_= marketVolCube_;
       
        if(isAtmCalibrated_){
            fillVolatilityCube();
            denseParameters_ = sabrCalibration(volCubeAtmCalibrated_);
            denseParameters_.updateInterpolators();
        } 

    }

    SwaptionVolatilityCubeBySabr::Cube  SwaptionVolatilityCubeBySabr::sabrCalibration(const Cube& marketVolCube) const {
        
        std::vector<Time> exerciseTimes(marketVolCube.expiries());
        std::vector<Time> timeLengths(marketVolCube.lengths());
        Matrix alphas(exerciseTimes.size(), timeLengths.size(),0.);
        Matrix betas(alphas);
        Matrix nus(alphas);
        Matrix rhos(alphas);
        Matrix forwards(alphas);
        Matrix errors(alphas);
        Matrix maxErrors(alphas);

        const std::vector<Matrix> tmpMarketVolCube = marketVolCube.points(); 
        
        for (Size j=0; j<exerciseTimes.size(); j++) {
            for (Size k=0; k<timeLengths.size(); k++) {
                const Rate atmForward = atmStrike(exerciseTimes[j], timeLengths[k]);
                std::vector<Real> strikes, volatilities;
                for (Size i=0; i<nStrikes_; i++){
                    strikes.push_back(atmForward+strikeSpreads_[i]);
                    volatilities.push_back(tmpMarketVolCube[i][j][k]);
                }

                const std::vector<Real> guess = parametersGuess_.operator ()(exerciseTimes[j], timeLengths[k]);

                const boost::shared_ptr<SABRInterpolation> sabrInterpolation = 
                    boost::shared_ptr<SABRInterpolation>(
                  new SABRInterpolation(strikes.begin(), strikes.end(), volatilities.begin(),
                    exerciseTimes[j], atmForward, 
                    guess[0],
                    guess[1], 
                    guess[2],
                    guess[3],
                    isParameterFixed_[0],
                    isParameterFixed_[1], 
                    isParameterFixed_[2],
                    isParameterFixed_[3],
                    boost::shared_ptr<OptimizationMethod>()));

                const Real interpolationError = sabrInterpolation->interpolationError();
                alphas[j][k]= sabrInterpolation->alpha();
                betas[j][k]= sabrInterpolation->beta();
                nus[j][k]= sabrInterpolation->nu();
                rhos[j][k]= sabrInterpolation->rho();
                forwards[j][k]= atmForward;
                errors[j][k]= interpolationError;
                maxErrors[j][k]= sabrInterpolation->interpolationMaxError();

            }
        }
        Cube sabrParametersCube(exerciseTimes, timeLengths, 7);
        sabrParametersCube.setLayer(0, alphas);
        sabrParametersCube.setLayer(1, betas);
        sabrParametersCube.setLayer(2, nus);
        sabrParametersCube.setLayer(3, rhos);
        sabrParametersCube.setLayer(4, forwards);
        sabrParametersCube.setLayer(5, errors);
        sabrParametersCube.setLayer(6, maxErrors);

        return sabrParametersCube;

    }

    Rate SwaptionVolatilityCubeBySabr::atmStrike(Time start, Time length) const {

        const Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));

        // vanilla swap's parameters
        const Date startDate = calendar_.advance(exerciseDate,swapSettlementDays_,Days);

        const Rounding rounder(0);
        const Date endDate = NullCalendar().advance(startDate,rounder(length),Years);
        boost::shared_ptr<Xibor> iborIndexEffective(iborIndex_);
		if (length<=shortTenor_) {
			iborIndexEffective = iborIndexShortTenor_;
		} 

        const Schedule fixedSchedule(startDate, endDate,
            Period(fixedLegFrequency_), calendar_,
            fixedLegConvention_, fixedLegConvention_,
            true, true);
        //Frequency floatingLegFrequency_ = iborIndexEffective->frequency();
        const BusinessDayConvention floatingLegBusinessDayConvention_ =
            iborIndexEffective->businessDayConvention();
        const Schedule floatSchedule(startDate, endDate,
            iborIndexEffective->tenor(), calendar_,
            floatingLegBusinessDayConvention_, floatingLegBusinessDayConvention_,
            true, true);
        const Real nominal_= 1.0;
        const Rate fixedRate_= 0.0;
        const Spread spread_= 0.0;
        Handle<YieldTermStructure> termStructure;
        termStructure.linkTo(iborIndexEffective->termStructure());
        const VanillaSwap swap(true, nominal_,
            fixedSchedule, fixedRate_, fixedLegDayCounter_,
            floatSchedule, iborIndexEffective,
            iborIndexEffective->settlementDays(), spread_, 
			iborIndexEffective->dayCounter(), termStructure);

        return swap.fairRate();
    }

    void SwaptionVolatilityCubeBySabr::fillVolatilityCube( ){
        
        const boost::shared_ptr<SwaptionVolatilityMatrix> atmVolStructure =
            boost::dynamic_pointer_cast<SwaptionVolatilityMatrix>(atmVolStructure_.currentLink());
                
        std::vector<Time> atmExerciseTimes(atmVolStructure->exerciseTimes());
        std::vector<Time> atmTimeLengths(atmVolStructure->timeLengths());
        
        std::vector<Time> exerciseTimes(volCubeAtmCalibrated_.expiries());
        std::vector<Time> timeLengths(volCubeAtmCalibrated_.lengths());
        
        atmExerciseTimes.insert( atmExerciseTimes.end(), exerciseTimes.begin(), exerciseTimes.end() );
        atmTimeLengths.insert( atmTimeLengths.end(), timeLengths.begin(), timeLengths.end() );
        std::sort(atmExerciseTimes.begin(),atmExerciseTimes.end());
        std::sort(atmTimeLengths.begin(),atmTimeLengths.end()); 

        createSparseSmiles();
            
        for (Size j=0; j<atmExerciseTimes.size(); j++) {

            for (Size k=0; k<atmTimeLengths.size(); k++) {
                const bool expandExpiries = 
                    !(std::binary_search(exerciseTimes.begin(),exerciseTimes.end(),atmExerciseTimes[j]));
                const bool expandLengths = 
                    !(std::binary_search(timeLengths.begin(),timeLengths.end(),atmTimeLengths[k]));  
                if(expandExpiries || expandLengths){
                    const Rate atmForward = atmStrike(atmExerciseTimes[j], atmTimeLengths[k]);
                    const Volatility atmVol = 
                        atmVolStructure_->volatility(atmExerciseTimes[j], atmTimeLengths[k], atmForward);
                    const std::vector<Real> spreadVols = spreadVolInterpolation(atmExerciseTimes[j],
                                                    atmTimeLengths[k]);
                    std::vector<Real> volAtmCalibrated;
                    for (Size i=0; i<nStrikes_; i++){
                        volAtmCalibrated.push_back(atmVol + spreadVols[i]);
                    }
                    volCubeAtmCalibrated_.setPoint(atmExerciseTimes[j], atmTimeLengths[k],volAtmCalibrated);
                }
            }
        }
        volCubeAtmCalibrated_.updateInterpolators();
    }


    void SwaptionVolatilityCubeBySabr::createSparseSmiles( ){
        
        std::vector<Time> exerciseTimes(sparseParameters_.expiries());
        std::vector<Time> timeLengths(sparseParameters_.lengths());
   
        for (Size j=0; j<exerciseTimes.size(); j++) {
            std::vector<boost::shared_ptr<SmileSection> > tmp;
            for (Size k=0; k<timeLengths.size(); k++) {
                tmp.push_back(smileSection(exerciseTimes[j], timeLengths[k], sparseParameters_)); 
            }
            sparseSmiles_.push_back(tmp);
        }
    }


    std::vector<Real> SwaptionVolatilityCubeBySabr::spreadVolInterpolation(
        double atmExerciseTime, double atmTimeLength){
        std::vector<Real> result;
        std::vector<Time> exerciseTimes(sparseParameters_.expiries());
        std::vector<Time> timeLengths(sparseParameters_.lengths());
         
        std::vector<Real>::const_iterator expiriesPreviousNode, lengthsPreviousNode;

        expiriesPreviousNode = std::lower_bound(exerciseTimes.begin(),
            exerciseTimes.end(), 
            atmExerciseTime);      
        Size expiriesPreviousIndex 
            = expiriesPreviousNode - exerciseTimes.begin();
        if(expiriesPreviousIndex >= exerciseTimes.size()-1) {
            expiriesPreviousIndex = exerciseTimes.size()-2;
        }
  
        lengthsPreviousNode = std::lower_bound(timeLengths.begin(),timeLengths.end(),atmTimeLength);
        Size lengthsPreviousIndex 
            = lengthsPreviousNode - timeLengths.begin();
        if(lengthsPreviousIndex >= timeLengths.size()-1) {
            lengthsPreviousIndex = timeLengths.size()-2; 
        }

        std::vector< std::vector<boost::shared_ptr<SmileSection> > > smiles;
        std::vector<boost::shared_ptr<SmileSection> >  smilesOnPreviousExpiry;
        std::vector<boost::shared_ptr<SmileSection> >  smilesOnNextExpiry;

        QL_REQUIRE(expiriesPreviousIndex+1 < sparseSmiles_.size(),
            "SwaptionVolatilityCubeBySabr::spreadVolInterpolation: expiriesPreviousIndex+1 >= sparseSmiles_.size()");
        QL_REQUIRE(lengthsPreviousIndex+1 < sparseSmiles_[0].size(),
            "SwaptionVolatilityCubeBySabr::spreadVolInterpolation: lengthsPreviousIndex+1 >= sparseSmiles_[0].size()");       
        smilesOnPreviousExpiry.push_back(sparseSmiles_[expiriesPreviousIndex][lengthsPreviousIndex]);
        smilesOnPreviousExpiry.push_back(sparseSmiles_[expiriesPreviousIndex][lengthsPreviousIndex+1]);
        smilesOnNextExpiry.push_back(sparseSmiles_[expiriesPreviousIndex+1][lengthsPreviousIndex]);
        smilesOnNextExpiry.push_back(sparseSmiles_[expiriesPreviousIndex+1][lengthsPreviousIndex+1]);

        smiles.push_back(smilesOnPreviousExpiry);
        smiles.push_back(smilesOnNextExpiry);

        std::vector<Real> exercisesNodes(2);
        exercisesNodes[0] = exerciseTimes[expiriesPreviousIndex];
        exercisesNodes[1] = exerciseTimes[expiriesPreviousIndex+1];

        std::vector<Real> lengthsNodes(2);
        lengthsNodes[0] = timeLengths[lengthsPreviousIndex];
        lengthsNodes[1] = timeLengths[lengthsPreviousIndex+1];

       
        const Rate atmForward = atmStrike(atmExerciseTime, atmTimeLength);
        const Volatility atmVol = atmVolStructure_->volatility(atmExerciseTime, atmTimeLength, atmForward);

        Matrix atmForwards(2,2,0.); 
        Matrix atmVols(2,2,0.); 
        for (Size i=0; i<2; i++){
            for (Size j=0; j<2; j++){
                atmForwards[i][j] = atmStrike(exercisesNodes[i], lengthsNodes[j]);
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
                    spreadVols[i][j] = smiles[i][j]->volatility(strikes[i][j])- atmVols[i][j];
                }
            }
           Cube localInterpolator(exercisesNodes, lengthsNodes, 1);
           localInterpolator.setLayer(0, spreadVols);
           localInterpolator.updateInterpolators();
            
           result.push_back(localInterpolator.operator ()(atmExerciseTime, atmTimeLength)[0]);    
        }
        return result;
    }

    Volatility SwaptionVolatilityCubeBySabr::
        volatilityImpl(Time expiry, Time length, Rate strike) const {
            return smileSection(expiry, length)->volatility(strike);
    }

    boost::shared_ptr<SmileSection> 
        SwaptionVolatilityCubeBySabr::smileSection(Time expiry, 
        Time length,                        
        const Cube& sabrParametersCube) const {
        const std::vector<Real> sabrParameters = sabrParametersCube.operator ()(expiry, length);
        return boost::shared_ptr<SmileSection>(
            new SmileSection(sabrParameters, fictitiousStrikes_, expiry));
    }


    boost::shared_ptr<SmileSection> 
        SwaptionVolatilityCubeBySabr::smileSection(Time expiry, Time length) const {
        if(isAtmCalibrated_){
            return smileSection(expiry, length, denseParameters_ );
        } 
        else{
            return smileSection(expiry, length, sparseParameters_ );
        }
    }

    Matrix SwaptionVolatilityCubeBySabr::sparseSabrParameters() const {
        return sparseParameters_.browse();
    }

    Matrix SwaptionVolatilityCubeBySabr::denseSabrParameters() const {
        return denseParameters_.browse();
    }

    Matrix SwaptionVolatilityCubeBySabr::marketVolCube() const {
        return marketVolCube_.browse();
    }
    Matrix SwaptionVolatilityCubeBySabr::volCubeAtmCalibrated() const {
        return volCubeAtmCalibrated_.browse();
    }

    void SwaptionVolatilityCubeBySabr::recalibration(Real beta){
        Matrix newBetaGuess(nExercise_, nlengths_, beta);
        parametersGuess_.setLayer(1, newBetaGuess);
        parametersGuess_.updateInterpolators();
        
        sparseParameters_ = sabrCalibration(marketVolCube_);
        sparseParameters_.updateInterpolators();
        volCubeAtmCalibrated_= marketVolCube_;
       
        if(isAtmCalibrated_){
            fillVolatilityCube();
            denseParameters_ = sabrCalibration(volCubeAtmCalibrated_);
            denseParameters_.updateInterpolators();
        }  
    }

    //===========================================================================//
    //                      SwaptionVolatilityCubeBySabr::Cube                   //
    //===========================================================================//


    SwaptionVolatilityCubeBySabr::Cube::Cube(const std::vector<Real>& expiries, const std::vector<Real>& lengths, 
                                    Size nLayers, bool extrapolation) : 
        expiries_(expiries), 
        lengths_(lengths), 
        nLayers_(nLayers),
        extrapolation_(extrapolation){

	    QL_REQUIRE(expiries.size()>1,"Cube::Cube(...): wrong input expiries");
	    QL_REQUIRE(lengths.size()>1,"Cube::Cube(...): wrong input lengths");

	    std::vector<Matrix> points(nLayers_, Matrix(expiries_.size(), lengths_.size(), 0.0));
         
        for(Size k=0;k<nLayers_;k++){
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
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        transposedPoints_ = o.transposedPoints_;
        for(Size k=0;k<nLayers_;k++){
            interpolators_.push_back(boost::shared_ptr<BilinearInterpolation>( 
                new BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(), lengths_.end(),transposedPoints_[k])));
            interpolators_[k]->enableExtrapolation();
        }  
	    setPoints(o.points_);
    }

    SwaptionVolatilityCubeBySabr::Cube& 
        SwaptionVolatilityCubeBySabr::Cube::operator=(const Cube& o){
        expiries_ = o.expiries_;
	    lengths_ = o.lengths_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        transposedPoints_ = o.transposedPoints_;
        for(Size k=0;k<nLayers_;k++){
            interpolators_.push_back(boost::shared_ptr<BilinearInterpolation>( 
                new BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(), lengths_.end(),transposedPoints_[k])));
            interpolators_[k]->enableExtrapolation();
        }  
	    setPoints(o.points_);
	    return *this;
    }
    void SwaptionVolatilityCubeBySabr::Cube::setElement(Size IndexOfLayer, Size IndexOfRow,
                                                  Size IndexOfColumn, Real x) {
        QL_REQUIRE(IndexOfLayer<nLayers_,"Cube::setElement: incompatible IndexOfLayer ");
	    QL_REQUIRE(IndexOfRow<expiries_.size(),"Cube::setElement: incompatible IndexOfRow");
        QL_REQUIRE(IndexOfColumn<lengths_.size(),"Cube::setElement: incompatible IndexOfColumn");
	    points_[IndexOfLayer][IndexOfRow][IndexOfColumn] = x;
    }
    void SwaptionVolatilityCubeBySabr::Cube::setPoints(const std::vector<Matrix>& x) {
        QL_REQUIRE(x.size()==nLayers_,"Cube::setPoints: incompatible number of layers ");
	    QL_REQUIRE(x[0].rows()==expiries_.size(),"Cube::setPoints: incompatible size 1");
        QL_REQUIRE(x[0].columns()==lengths_.size(),"Cube::setPoints: incompatible size 2");

	    points_ = x;
    }
    void SwaptionVolatilityCubeBySabr::Cube::setLayer(Size i, const Matrix& x) {
        QL_REQUIRE(i<nLayers_,"Cube::setLayer: incompatible number of layer ");
	    QL_REQUIRE(x.rows()==expiries_.size(),"Cube::setLayer: incompatible size 1");
        QL_REQUIRE(x.columns()==lengths_.size(),"Cube::setLayer: incompatible size 2");

	    points_[i] = x;
    }

    void SwaptionVolatilityCubeBySabr::Cube::setPoint(const Real& expiry, const Real& length,
                                                const std::vector<Real> point){
            
        const bool expandExpiries = !(std::binary_search(expiries_.begin(),expiries_.end(),expiry));
        const bool expandLengths = !(std::binary_search(lengths_.begin(),lengths_.end(),length));
       
        std::vector<Real>::const_iterator expiriesPreviousNode, lengthsPreviousNode;
        
        expiriesPreviousNode = std::lower_bound(expiries_.begin(),expiries_.end(),expiry);
        std::vector<Real>::iterator::difference_type 
            expiriesIndex = expiriesPreviousNode - expiries_.begin();

        lengthsPreviousNode = std::lower_bound(lengths_.begin(),lengths_.end(),length);
        std::vector<Real>::iterator::difference_type 
            lengthsIndex = lengthsPreviousNode - lengths_.begin();

        if(expandExpiries || expandLengths ) {
            expandLayers(expiriesIndex, expandExpiries, lengthsIndex, expandLengths);
        }
        for(Size k=0;k<nLayers_;k++){
            points_[k][expiriesIndex][lengthsIndex]= point[k];
        }    
        expiries_[expiriesIndex] = expiry;
        lengths_[lengthsIndex] = length;
    }

    void SwaptionVolatilityCubeBySabr::Cube::expandLayers(Size i, bool expandExpiries, 
                                                          Size j, bool expandLengths){
	    QL_REQUIRE(i<=expiries_.size(),"Cube::expandLayers: incompatible size 1");
        QL_REQUIRE(j<=lengths_.size(),"Cube::expandLayers: incompatible size 2");
 
        if(expandExpiries) {
            expiries_.insert(expiries_.begin()+i,0.);
        }
        if(expandLengths) {
            lengths_.insert(lengths_.begin()+j,0.);
        }
        
        std::vector<Matrix> newPoints(nLayers_,Matrix(expiries_.size(), lengths_.size(), 0.));

        for(Size k=0;k<nLayers_;k++){
            for(Size u=0;u<points_[k].rows();u++){
                 Size indexOfRow = u;
                 if(u>=i && expandExpiries) indexOfRow = u+1;
                 for(Size v=0;v<points_[k].columns();v++){
                      Size indexOfCol = v;
                      if(v>=j && expandLengths) indexOfCol = v+1;
                      newPoints[k][indexOfRow][indexOfCol]=points_[k][u][v];
                 }
            }
        }
        setPoints(newPoints);
    }

    const std::vector<Matrix>& SwaptionVolatilityCubeBySabr::Cube::points() const {
	    return points_;
    }

    std::vector<Real> SwaptionVolatilityCubeBySabr::Cube::operator()(const Real& expiry, 
                                                                const Real& length) const {
        std::vector<Real> result;
        for(Size k=0;k<nLayers_;k++){
            result.push_back(interpolators_[k]->operator()(expiry, length));
        }   
	    return result;
    }

    const std::vector<Real>& SwaptionVolatilityCubeBySabr::Cube::expiries() const {
	    return expiries_;
    }

    const std::vector<Real>& SwaptionVolatilityCubeBySabr::Cube::lengths() const {
	    return lengths_;
    }	

    void SwaptionVolatilityCubeBySabr::Cube::updateInterpolators() const{
	    for(Size k=0;k<nLayers_;k++){
            transposedPoints_[k]=transpose(points_[k]);
            interpolators_[k] = boost::shared_ptr<BilinearInterpolation>( new BilinearInterpolation                       (expiries_.begin(), expiries_.end(),lengths_.begin(), lengths_.end(),transposedPoints_[k]));

            interpolators_[k]->enableExtrapolation();
        } 
    }	
    Matrix SwaptionVolatilityCubeBySabr::Cube::browse() const{
        Matrix result(lengths_.size()*expiries_.size(),nLayers_+2,0.);
	    for(Size i=0;i<lengths_.size();i++){
	        for(Size j=0;j<expiries_.size();j++){
                result[i*expiries_.size()+j][0]= lengths_[i];
                result[i*expiries_.size()+j][1]= expiries_[j];
	            for(Size k=0;k<nLayers_;k++){
                    result[i*expiries_.size()+j][2+k]= points_[k][j][i]; 
                }   
            }   
        }  
        return result;
    }

}