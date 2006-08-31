/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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


#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Math/sabrinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>

#include <fstream>
#include <string>
             
namespace QuantLib {

    namespace {
        double ALPHAGUESS = .02;
        double BETAGUESS = .36;
        double NUGUESS = .4;
        double RHOGUESS = .2;
    }

     SwaptionVolatilityCube::SwaptionVolatilityCube(
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
        const boost::shared_ptr<Xibor>& iborIndexShortTenor)
    : atmVolStructure_(atmVolStructure),
      exerciseDates_(expiries.size()), exerciseTimes_(expiries.size()),
      exerciseDatesAsReal_(expiries.size()),
      lengths_(lengths), timeLengths_(lengths.size()),
      nStrikes_(strikeSpreads.size()), strikeSpreads_(strikeSpreads),
      volSpreads_(nStrikes_, Matrix(expiries.size(), lengths.size(), 0.0)),
      volSpreadsInterpolator_(nStrikes_),
      localStrikes_(nStrikes_), localSmile_(nStrikes_),
      calendar_(calendar), swapSettlementDays_(swapSettlementDays),
	  fixedLegFrequency_(fixedLegFrequency),
      fixedLegConvention_(fixedLegConvention),
      fixedLegDayCounter_(fixedLegDayCounter),
      iborIndex_(iborIndex), shortTenor_(shortTenor),
      iborIndexShortTenor_(iborIndexShortTenor)
    {
        Size i, nExercise = expiries.size();
        exerciseDates_[0] = calendar_.advance(referenceDate(),
                                              expiries[0],
                                              Unadjusted); //FIXME
        exerciseDatesAsReal_[0] =
            static_cast<Real>(exerciseDates_[0].serialNumber());
        exerciseTimes_[0] = timeFromReference(exerciseDates_[0]);
        QL_REQUIRE(0.0<exerciseTimes_[0],
                   "first exercise time is negative");
        for (i=1; i<nExercise; i++) {
            exerciseDates_[i] = calendar_.advance(referenceDate(),
                                                  expiries[i],
                                                  Unadjusted); //FIXME
            exerciseDatesAsReal_[i] =
                static_cast<Real>(exerciseDates_[i].serialNumber());
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
            QL_REQUIRE(exerciseTimes_[i-1]<exerciseTimes_[i],
                       "non increasing exercise times");
        }

        exerciseInterpolator_ = LinearInterpolation(exerciseTimes_.begin(),
                                                    exerciseTimes_.end(),
                                                    exerciseDatesAsReal_.begin());
        exerciseInterpolator_.enableExtrapolation();

        Size nlengths = lengths_.size();
        Date startDate = exerciseDates_[0]; // as good as any
        Date endDate = startDate + lengths_[0];
        timeLengths_[0] = dayCounter().yearFraction(startDate,endDate);
        QL_REQUIRE(0.0<timeLengths_[0],
                   "first time length is negative");
        for (i=1; i<nlengths; i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter().yearFraction(startDate,endDate);
            QL_REQUIRE(timeLengths_[i-1]<timeLengths_[i],
                       "non increasing time length");
        }

        QL_REQUIRE(nStrikes_>1, "too few strikes (" << nStrikes_ << ")");
        for (i=1; i<nStrikes_; i++) {
            QL_REQUIRE(strikeSpreads_[i-1]<strikeSpreads_[i],
                "non increasing strike spreads");
        }

        QL_REQUIRE(nStrikes_==volSpreads.columns(),
                   "nStrikes_!=volSpreads.columns()");
        QL_REQUIRE(nExercise*nlengths==volSpreads.rows(),
                   "nExercise*nlengths!=volSpreads.rows()");
        for (i=0; i<nStrikes_; i++)
        {
            for (Size j=0; j<nExercise; j++) {
                for (Size k=0; k<nlengths; k++) {
                    volSpreads_[i][j][k]=volSpreads[j*nlengths+k][i];
                }
            }
            volSpreadsInterpolator_[i] = BilinearInterpolation(
                timeLengths_.begin(), timeLengths_.end(),
                exerciseTimes_.begin(), exerciseTimes_.end(),
                volSpreads_[i]);
            volSpreadsInterpolator_[i].enableExtrapolation();
        }

        //registerWith(atmVolMatrix_);
    }

    boost::shared_ptr<Interpolation>
    SwaptionVolatilityCube::smile(Time start, Time length) const
    {
        const Rate atmForward = atmStrike(start, length);

        const Volatility atmVol = atmVolStructure_->volatility(start, length, atmForward);
        for (Size i=0; i<nStrikes_; i++) {
            localStrikes_[i] = atmForward + strikeSpreads_[i];
            localSmile_[i]   = atmVol + volSpreadsInterpolator_[i](length, start);
        }
        return boost::shared_ptr<Interpolation>(new
            //SABRInterpolation(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin(), start, atmForward, Null<Real>(), Null<Real>(), Null<Real>(), Null<Real>())
            LinearInterpolation(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin())
            //NaturalCubicSpline(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin())
            );
    }

    VarianceSmileSection SwaptionVolatilityCube::smileSection(Time start, Time length) const {

        std::vector<Real> strikes, volatilities;

        const Rate atmForward = atmStrike(start, length);

        const Volatility atmVol =
            atmVolStructure_->volatility(start, length, atmForward);
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(atmForward + strikeSpreads_[i]);
            volatilities.push_back(atmVol + volSpreadsInterpolator_[i](length, start));
        }
        ////add points to force flat extrapolation
        //strikes.insert(strikes.begin(), strikes.front()-.000000001);
        //strikes.insert(strikes.end(), strikes.back()+.1);

        //volatilities.insert(volatilities.begin(),volatilities.front());
        //volatilities.insert(volatilities.end(),volatilities.back());

        //return VarianceSmileSection(start, strikes, volatilities);
        return VarianceSmileSection(start, atmForward, strikes, volatilities);
    }


    Volatility SwaptionVolatilityCube::
        volatilityImpl(Time start, Time length, Rate strike) const {
            return smile(start, length)->operator()(strike, true);
        }

     VarianceSmileSection::VarianceSmileSection(Time timeToExpiry,
                        const std::vector<Rate>& strikes,
                        const std::vector<Rate>& volatilities) :
     timeToExpiry_(timeToExpiry),
         strikes_(strikes),
         volatilities_(volatilities) {
             interpolation_ = boost::shared_ptr<Interpolation>(new
                 LinearInterpolation(strikes_.begin(),
                 strikes_.end(), volatilities_.begin())
                 );
         }

      VarianceSmileSection::VarianceSmileSection(Time timeToExpiry,
          Rate forwardValue,
          const std::vector<Rate>& strikes,
          const std::vector<Rate>& volatilities) :
      timeToExpiry_(timeToExpiry), strikes_(strikes),
          volatilities_(volatilities) {

                boost::shared_ptr<LineSearch> lineSearch(new ArmijoLineSearch(1e-12, 0.15, 0.55));
                boost::shared_ptr<OptimizationMethod> method = boost::shared_ptr<OptimizationMethod>(
                    new ConjugateGradient(lineSearch));
                method->setEndCriteria(EndCriteria(100000, 1e-12));
                Array guess(4);
                guess[0] = ALPHAGUESS; 
                guess[1] = BETAGUESS;  
                guess[2] = NUGUESS; 
                guess[3] = RHOGUESS; 
                method->setInitialValue(guess);

              interpolation_ = boost::shared_ptr<Interpolation>(new
                  SABRInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin(),
                  timeToExpiry, forwardValue, Null<Real>(), Null<Real>(), Null<Real>(),
                  Null<Real>(), method));

               const boost::shared_ptr<SABRInterpolation> sabrInterpolation =
               boost::dynamic_pointer_cast<SABRInterpolation>(interpolation_);
                


              //std::stringstream s;
              //ALPHAGUESS = sabrInterpolation->alpha();
              //BETAGUESS = sabrInterpolation->beta();
              //NUGUESS = sabrInterpolation->nu();
              //RHOGUESS = sabrInterpolation->rho();

              //s << "1:*********************\n";
              //for(int i=0;i<strikes_.size();i++){
              //s <<strikes_[i] << "\t";
              //}
              //s <<"\n";
              //for(int i=0;i<volatilities_.size();i++){
              //s <<volatilities_[i] << "\t";
              //}
              //s <<"\n";
              //s << ALPHAGUESS << "\n";
              //s << BETAGUESS << "\n";
              //s << NUGUESS << "\n";
              //s << RHOGUESS << "\n";
              //s << sabrInterpolation->interpolationError() << "\n";
              //s << sabrInterpolation->interpolationMaxError() << "\n";
              //s << sabrInterpolation->endCriteria() << "\n";
              //s << "2:*********************\n";

              //static std::ofstream log("C:\\QL_LOG.txt");

              //log << s.str();
                 QL_ENSURE(sabrInterpolation->interpolationError()<1e-4, 
                   "VarianceSmileSection::VarianceSmileSection: accuracy not reached");

          }

    Volatility VarianceSmileSection::operator ()(const Real& strike) const {
        const Real v = interpolation_->operator()(strike, true);
        return v*v*timeToExpiry_;
    }

    Rate SwaptionVolatilityCube::atmStrike(Time start, Time length) const {

        Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));

        // vanilla swap's parameters
        Integer swapFixingDays = 2; // FIXME
        Date startDate = calendar_.advance(exerciseDate,swapFixingDays,Days);

        Rounding rounder(0);
        Date endDate = NullCalendar().advance(startDate,rounder(length),Years);

        // (lenght<shortTenor_, iborIndexShortTenor_, iborIndex_);

        Schedule fixedSchedule = Schedule(calendar_, startDate, endDate,
            fixedLegFrequency_, fixedLegConvention_, Date(), true, false);
        Frequency floatingLegFrequency_ = iborIndex_->frequency();
        BusinessDayConvention floatingLegBusinessDayConvention_ =
            iborIndex_->businessDayConvention();
        Schedule floatSchedule = Schedule(calendar_, startDate, endDate,
            floatingLegFrequency_, floatingLegBusinessDayConvention_,
            Date(), true, false);
        Real nominal_= 1.0;
        Rate fixedRate_= 0.0;
        Spread spread_= 0.0;
        Handle<YieldTermStructure> termStructure;
        termStructure.linkTo(iborIndex_->termStructure());
        VanillaSwap swap(true, nominal_,
            fixedSchedule, fixedRate_, fixedLegDayCounter_,
            floatSchedule, iborIndex_,
            iborIndex_->settlementDays(), spread_, iborIndex_->dayCounter(),
            termStructure);

        return swap.fairRate();
    }

////////////////////////////////////

    SwaptionVolatilityCubeBySabr::SwaptionVolatilityCubeBySabr(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const Matrix& marketVolCube,
        const Calendar& calendar,
		Integer swapSettlementDays,
        Frequency fixedLegFrequency,
        BusinessDayConvention fixedLegConvention,
        const DayCounter& fixedLegDayCounter,
        const boost::shared_ptr<Xibor>& iborIndex,
        Time shortTenor,
        const boost::shared_ptr<Xibor>& iborIndexShortTenor)
    : atmVolStructure_(atmVolStructure),
      exerciseDates_(expiries.size()), exerciseTimes_(expiries.size()),
      exerciseDatesAsReal_(expiries.size()),
      lengths_(lengths), timeLengths_(lengths.size()),
      nStrikes_(strikeSpreads.size()), strikeSpreads_(strikeSpreads),
      volSpreadsInterpolator_(nStrikes_),
      localStrikes_(nStrikes_), localSmile_(nStrikes_),
      calendar_(calendar), swapSettlementDays_(swapSettlementDays),
	  fixedLegFrequency_(fixedLegFrequency),
      fixedLegConvention_(fixedLegConvention),
      fixedLegDayCounter_(fixedLegDayCounter),
      iborIndex_(iborIndex), shortTenor_(shortTenor),
      iborIndexShortTenor_(iborIndexShortTenor)
    {
        Size i, nExercise = expiries.size();
        exerciseDates_[0] = calendar_.advance(referenceDate(),
                                              expiries[0],
                                              Unadjusted); //FIXME
        exerciseDatesAsReal_[0] =
            static_cast<Real>(exerciseDates_[0].serialNumber());
        exerciseTimes_[0] = timeFromReference(exerciseDates_[0]);
        QL_REQUIRE(0.0<exerciseTimes_[0],
                   "first exercise time is negative");
        for (i=1; i<nExercise; i++) {
            exerciseDates_[i] = calendar_.advance(referenceDate(),
                                                  expiries[i],
                                                  Unadjusted); //FIXME
            exerciseDatesAsReal_[i] =
                static_cast<Real>(exerciseDates_[i].serialNumber());
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
            QL_REQUIRE(exerciseTimes_[i-1]<exerciseTimes_[i],
                       "non increasing exercise times");
        }

        exerciseInterpolator_ = LinearInterpolation(exerciseTimes_.begin(),
                                                    exerciseTimes_.end(),
                                                    exerciseDatesAsReal_.begin());
        exerciseInterpolator_.enableExtrapolation();

        Size nlengths = lengths_.size();
        Date startDate = exerciseDates_[0]; // as good as any
        Date endDate = startDate + lengths_[0];
        timeLengths_[0] = dayCounter().yearFraction(startDate,endDate);
        QL_REQUIRE(0.0<timeLengths_[0],
                   "first time length is negative");
        for (i=1; i<nlengths; i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter().yearFraction(startDate,endDate);
            QL_REQUIRE(timeLengths_[i-1]<timeLengths_[i],
                       "non increasing time length");
        }

        QL_REQUIRE(nStrikes_>1, "too few strikes (" << nStrikes_ << ")");
        for (i=1; i<nStrikes_; i++) {
            QL_REQUIRE(strikeSpreads_[i-1]<strikeSpreads_[i],
                "non increasing strike spreads");
        }

        QL_REQUIRE(nStrikes_==marketVolCube.columns(),
                   "nStrikes_!=marketVolCube.columns()");
        QL_REQUIRE(nExercise*nlengths==marketVolCube.rows(),
                   "nExercise*nlengths!=marketVolCube.rows()");
        
        marketVolCube_ = Cube(exerciseTimes_, timeLengths_, nStrikes_);
        for (i=0; i<nStrikes_; i++)
        {
            for (Size j=0; j<nExercise; j++) {
                for (Size k=0; k<nlengths; k++) {
                    marketVolCube_.setElement(i, j, k, marketVolCube[j*nlengths+k][i]);
                }
            }
        }
 
        sparseParameters_ = sabrCalibration(marketVolCube_);
        volCubeAtmCalibrated_= marketVolCube_;
        fillVolatilityCube();

    }

    SwaptionVolatilityCubeBySabr::Cube  SwaptionVolatilityCubeBySabr::sabrCalibration(Cube& marketVolCube) const {
           
        Matrix alphas(exerciseTimes_.size(), timeLengths_.size(),0.);
        Matrix betas(alphas);
        Matrix nus(alphas);
        Matrix rhos(alphas);
        Matrix forwards(alphas);

        const std::vector<Matrix> tmpMarketVolCube = marketVolCube_.points(); 

        for (Size j=0; j<exerciseTimes_.size(); j++) {
            for (Size k=0; k<timeLengths_.size(); k++) {
                const Rate atmForward = atmStrike(exerciseTimes_[j], timeLengths_[k]);
                std::vector<Real> strikes, volatilities;
                for (Size i=0; i<nStrikes_; i++){
                    strikes.push_back(atmForward+strikeSpreads_[i]);
                    volatilities.push_back(tmpMarketVolCube[i][j][k]);
                }

                const boost::shared_ptr<SABRInterpolation> sabrInterpolation = 
                    boost::shared_ptr<SABRInterpolation>(
                  new SABRInterpolation(strikes.begin(), strikes.end(), volatilities.begin(),
                  exerciseTimes_[j], atmForward, Null<Real>(), Null<Real>(), Null<Real>(),
                  Null<Real>()));
                QL_ENSURE(sabrInterpolation->interpolationError()<1e-4, 
                   "VarianceSmileSection::VarianceSmileSection: accuracy not reached");
                alphas[j][k]= sabrInterpolation->alpha();
                betas[j][k]= sabrInterpolation->beta();
                nus[j][k]= sabrInterpolation->nu();
                rhos[j][k]= sabrInterpolation->rho();
                forwards[j][k]= atmForward;

            }
        }
        Cube sabrParametersCube(exerciseTimes_, timeLengths_, 5);
        sabrParametersCube.setLayer(0, alphas);
        sabrParametersCube.setLayer(1, betas);
        sabrParametersCube.setLayer(2, nus);
        sabrParametersCube.setLayer(3, rhos);
        sabrParametersCube.setLayer(4, forwards);

        return sabrParametersCube;

    }

    Rate SwaptionVolatilityCubeBySabr::atmStrike(Time start, Time length) const {

        Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));

        // vanilla swap's parameters
        Date startDate = calendar_.advance(exerciseDate,swapSettlementDays_,Days);

        Rounding rounder(0);
        Date endDate = NullCalendar().advance(startDate,rounder(length),Years);
        boost::shared_ptr<Xibor> iborIndexEffective(iborIndex_);
		if (length<=shortTenor_) {
			iborIndexEffective = iborIndexShortTenor_;
		} 

        Schedule fixedSchedule = Schedule(calendar_, startDate, endDate,
            fixedLegFrequency_, fixedLegConvention_, Date(), true, false);
        Frequency floatingLegFrequency_ = iborIndexEffective->frequency();
        BusinessDayConvention floatingLegBusinessDayConvention_ =
            iborIndexEffective->businessDayConvention();
        Schedule floatSchedule = Schedule(calendar_, startDate, endDate,
            floatingLegFrequency_, floatingLegBusinessDayConvention_,
            Date(), true, false);
        Real nominal_= 1.0;
        Rate fixedRate_= 0.0;
        Spread spread_= 0.0;
        Handle<YieldTermStructure> termStructure;
        termStructure.linkTo(iborIndexEffective->termStructure());
        VanillaSwap swap(true, nominal_,
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
        
        for (Size j=0; j<atmExerciseTimes.size(); j++) {
//sparseParameters_
            for (Size k=0; k<atmTimeLengths.size(); k++) {
                bool expandExpiries = 
                    !(std::binary_search(exerciseTimes.begin(),exerciseTimes.end(),atmExerciseTimes[j]));
                bool expandLengths = 
                    !(std::binary_search(timeLengths.begin(),timeLengths.end(),atmTimeLengths[k]));  
                if(expandExpiries || expandLengths){
                    const Rate atmForward = atmStrike(atmExerciseTimes[j], atmTimeLengths[k]);
                    const Volatility atmVol = 
                        atmVolStructure_->volatility(atmExerciseTimes[j], atmTimeLengths[k], atmForward);
                    std::vector<Real>  volAtmCalibrated;
                    for (Size i=0; i<nStrikes_; i++){
                        Volatility spreadVol=spreadVolInterpolation(atmExerciseTimes[j],
                                                    atmTimeLengths[k], sparseParameters_);
                        volAtmCalibrated.push_back(atmVol + spreadVol);
                    }
                    volCubeAtmCalibrated_.setPoint(atmExerciseTimes[j], atmTimeLengths[k],volAtmCalibrated);
                }
            }
        }
    }

    double SwaptionVolatilityCubeBySabr::spreadVolInterpolation(double atmExerciseTime, 
      double atmTimeLengths, Cube& sabrParametersCube){
        //std::vector<Time> exerciseTimes(sabrParametersCube.expiries());
        //std::vector<Time> timeLengths(sabrParametersCube.lengths());
        // 
        //std::vector<Real>::const_iterator expiriesPreviousNode, lengthsPreviousNode;

        //expiriesPreviousNode = std::lower_bound(expiries_.begin(),expiries_.end(),expiry);       
        //std::vector<Real>::iterator::difference_type 
        //expiriesPreviousIndex = expiriesPreviousNode - expiries_.begin();

        //lengthsPreviousNode = std::lower_bound(lengths_.begin(),lengths_.end(),lengths);
        //std::vector<Real>::iterator::difference_type 
        //lengthsPreviousIndex = lengthsPreviousNode - lengths_.begin();

        //Size expiriesNextIndex, lengthsNextIndex;

        //                    std::vector<Real> strikes, volatilities, volAtmCalibrated;
        //            const boost::shared_ptr<SABRInterpolation> sabrInterpolation = 
        //                boost::shared_ptr<SABRInterpolation>(
        //            new SABRInterpolation(strikes.begin(), strikes.end(), volatilities.begin(),
        //            exerciseTimes_[j], atmForward, Null<Real>(), Null<Real>(), Null<Real>(),
        //            Null<Real>()));

                  return 0.;
    }
    //////
    Volatility SwaptionVolatilityCubeBySabr::
        volatilityImpl(Time expiry, Time length, Rate strike) const {
            return std::sqrt(smileSection(expiry, length).operator()(strike)/expiry);
        }
    VarianceSmileSection SwaptionVolatilityCubeBySabr::smileSection(Time expiry, Time length) const {

        std::vector<Real> strikes, volatilities, sabrParameters;
        sabrParameters = sparseParameters_.operator ()(expiry, length);
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(0.05*i+.01);
        }

        return VarianceSmileSection(sabrParameters, strikes, expiry);
    }
  
    VarianceSmileSection::VarianceSmileSection(
          const std::vector<Real>& sabrParameters,
          const std::vector<Rate>& strikes,
          const Time timeToExpiry) :
      timeToExpiry_(timeToExpiry), strikes_(strikes),volatilities_(strikes) {

           Real alpha = sabrParameters[0];
           Real beta = sabrParameters[1];
           Real nu = sabrParameters[2];
           Real rho = sabrParameters[3];
           Real forwardValue = sabrParameters[4];

           interpolation_ = boost::shared_ptr<Interpolation>(new
                  SABRInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin(),
                  timeToExpiry, forwardValue, alpha, beta, nu, rho,
                  boost::shared_ptr<OptimizationMethod>()));
      }

    /********************		SwaptionVolatilityCubeBySabr::Cube				*************************/

    SwaptionVolatilityCubeBySabr::Cube::Cube(const std::vector<Real>& expiries, const std::vector<Real>& lengths, 
                                    Size nLayers, bool extrapolation) : 
        expiries_(expiries), 
        lengths_(lengths), 
        nLayers_(nLayers),
        extrapolation_(extrapolation){

	    QL_REQUIRE(expiries.size()>1,"Cube::Cube(...): wrong input expiries");
	    QL_REQUIRE(lengths.size()>1,"Cube::Cube(...): wrong input lengths");

	    std::vector<Matrix> points(nLayers_,Matrix(expiries_.size(), lengths_.size(), 0.0));
        for(Size k=0;k<nLayers_;k++){
            interpolators_.push_back(BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(), lengths_.end(),points[k]));
            interpolators_[k].enableExtrapolation();
        }   
   	    setPoints(points); 
     }

    SwaptionVolatilityCubeBySabr::Cube::Cube(const Cube& o) {
        expiries_ = o.expiries_;
	    lengths_ = o.lengths_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        for(Size k=0;k<nLayers_;k++){
            interpolators_.push_back(BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(), lengths_.end(),o.points_[k]));
            interpolators_[k].enableExtrapolation();
        }  
	    setPoints(o.points_);
    }

    SwaptionVolatilityCubeBySabr::Cube& SwaptionVolatilityCubeBySabr::Cube::operator=(const Cube& o){
        expiries_ = o.expiries_;
	    lengths_ = o.lengths_;
        nLayers_ = o.nLayers_;
        extrapolation_ = o.extrapolation_;
        for(Size k=0;k<nLayers_;k++){
            interpolators_.push_back(BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(), lengths_.end(),o.points_[k]));
            interpolators_[k].enableExtrapolation();
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
        updateInterpolators();
    }
    void SwaptionVolatilityCubeBySabr::Cube::setPoints(const std::vector<Matrix>& x) {
        QL_REQUIRE(x.size()==nLayers_,"Cube::setPoints: incompatible number of layers ");
	    QL_REQUIRE(x[0].rows()==expiries_.size(),"Cube::setPoints: incompatible size 1");
        QL_REQUIRE(x[0].columns()==lengths_.size(),"Cube::setPoints: incompatible size 2");

	    points_ = x;
        updateInterpolators();
    }
    void SwaptionVolatilityCubeBySabr::Cube::setLayer(Size i, const Matrix& x) {
        QL_REQUIRE(i<nLayers_,"Cube::setLayer: incompatible number of layer ");
	    QL_REQUIRE(x.rows()==expiries_.size(),"Cube::setLayer: incompatible size 1");
        QL_REQUIRE(x.columns()==lengths_.size(),"Cube::setLayer: incompatible size 2");

	    points_[i] = x;
        updateInterpolators();
    }

    void SwaptionVolatilityCubeBySabr::Cube::setPoint(const Real& expiry, const Real& lengths,
                                                const std::vector<Real> point){
            bool expandExpiries = !(std::binary_search(expiries_.begin(),expiries_.end(),expiry));
            bool expandLengths = !(std::binary_search(lengths_.begin(),lengths_.end(),lengths));
           
            std::vector<Real>::const_iterator expiriesPreviousNode, lengthsPreviousNode;
            expiriesPreviousNode = std::lower_bound(expiries_.begin(),expiries_.end(),expiry);
            std::vector<Real>::iterator::difference_type 
                expiriesIndex = expiriesPreviousNode - expiries_.begin();

            lengthsPreviousNode = std::lower_bound(lengths_.begin(),lengths_.end(),lengths);
            std::vector<Real>::iterator::difference_type 
                lengthsIndex = lengthsPreviousNode - lengths_.begin();
            
            if(expandExpiries || expandLengths ) 
                expandLayers(expiriesIndex, expandExpiries, lengthsIndex, expandLengths);
            if(expandExpiries) expiriesIndex++;
            if(expandLengths) lengthsIndex++;
            for(Size k=0;k<nLayers_;k++){
                points_[k][expiriesIndex][lengthsIndex]= point[k];
            } 
            updateInterpolators();
  
    }
    void SwaptionVolatilityCubeBySabr::Cube::expandLayers(Size i, bool expandExpiries, 
                                                          Size j, bool expandLengths){
	    QL_REQUIRE(i<expiries_.size(),"Cube::expandLayers: incompatible size 1");
        QL_REQUIRE(j<lengths_.size(),"Cube::expandLayers: incompatible size 2");
 
        if(expandExpiries) expiries_.insert(expiries_.begin()+i,0.);
        if(expandLengths) lengths_.insert(lengths_.begin()+j,0.);
        
        std::vector<Matrix> newPoints(nLayers_,Matrix(expiries_.size(), lengths_.size(), 0.0));

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
    };
    const std::vector<Matrix>& SwaptionVolatilityCubeBySabr::Cube::points() const {
	    return points_;
    }

    std::vector<Real> SwaptionVolatilityCubeBySabr::Cube::operator()(const Real& expiry, 
                                                                const Real& length) const {
        std::vector<Real> result;
        for(Size k=0;k<nLayers_;k++){
            result.push_back(interpolators_[k](expiry, length));
        }   
	    return result;
    }

    const std::vector<Real>& SwaptionVolatilityCubeBySabr::Cube::expiries() const {
	    return expiries_;
    }

    const std::vector<Real>& SwaptionVolatilityCubeBySabr::Cube::lengths() const {
	    return lengths_;
    }	

    void SwaptionVolatilityCubeBySabr::Cube::updateInterpolators(){
	    for(Size k=0;k<nLayers_;k++){
            interpolators_[k] = BilinearInterpolation(expiries_.begin(), expiries_.end(),
                                        lengths_.begin(), lengths_.end(),points_[k]);
            interpolators_[k].enableExtrapolation();
        }    
    }	

}
