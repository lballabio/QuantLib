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

#ifndef quantlib_base_correl_structure_hpp
#define quantlib_base_correl_structure_hpp

#include <ql/quote.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>

#include <ql/experimental/credit/correlationstructure.hpp>

namespace QuantLib {

    // Between this class and its parent I might need to include more classes
    //   allowing for more structure.

    /*! Matrix based Base Correlation Term Structure 
    Loss level versus time interpolated scalar copula type parametric 
    correlation term structure. Represents the correlation for the credit loss 
    level of a given portfolio at a given loss level and time.
    \todo:
    - Consider moving to a matrix data structure. A matrix might make some
    computations heavy, template specialization on the dimension might be an
    alternative to having two classes, one for scalars and another for matrices.
    - Rethink all the data structure with a basket where current losses are not
    zero.
    - In principle the 2D interpolator is left optional since there are 
    arbitrage issues on the interpolator type to be used. However one has to be
    careful when using non local interpolators like CubicSplines which have an
    effect on the past (calibrated) coupons of previous tenors.
    */
    template<class Interpolator2D_T> // template? alternatively have a ref to Interpolator2D
    class BaseCorrelationTermStructure : public CorrelationTermStructure {
    public:
/*
Inspiration from:
    SwaptionVolatilityDiscrete::SwaptionVolatilityDiscrete(
                                    const std::vector<Period>& optionTenors,
                                    const std::vector<Period>& swapTenors,
                                    Natural settlementDays,
                                    const Calendar& cal,
                                    BusinessDayConvention bdc,
                                    const DayCounter& dc)
    : SwaptionVolatilityStructure(settlementDays, cal, bdc, dc),
      nOptionTenors_(optionTenors.size()),
      optionTenors_(optionTenors),
      optionDates_(nOptionTenors_),
      optionTimes_(nOptionTenors_),
      optionDatesAsReal_(nOptionTenors_),
      nSwapTenors_(swapTenors.size()),
      swapTenors_(swapTenors),
      swapLengths_(nSwapTenors_) {

        checkOptionTenors();
        initializeOptionDatesAndTimes();

        checkSwapTenors();
        initializeSwapLengths();

        optionInterpolator_= LinearInterpolation(optionTimes_.begin(),
                                                 optionTimes_.end(),
                                                 optionDatesAsReal_.begin());
        optionInterpolator_.update();
        optionInterpolator_.enableExtrapolation();

        registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
    }

*/
        BaseCorrelationTermStructure(
            Natural settlementDays,
            const Calendar& cal,
            BusinessDayConvention bdc,
            const std::vector<Period>& tenors,// sorted
            const std::vector<Real>& lossLevel,//sorted
            const std::vector<std::vector<Handle<Quote> > >& correls,
            const DayCounter& dc = DayCounter()
            )
        : CorrelationTermStructure(settlementDays, cal, bdc, dc),
          correlHandles_(correls),
          correlations_(correls.size(), correls.front().size()),
          nTrancheTenors_(tenors.size()),
          nLosses_(lossLevel.size()),
          tenors_(tenors),
          lossLevel_(lossLevel),
          trancheTimes_(tenors.size(), 0.) {
              checkTrancheTenors();
                
              // INIT TRANCHE DATES--- move to update?????????? we are reg with instance date???
              for(Size i=0; i<tenors_.size(); i++)
                  trancheDates_.push_back(
                      calendar().advance(referenceDate(),
                                         tenors_[i],
                                         businessDayConvention())
                                          );

              initializeTrancheTimes();// have to set the size-- see the vol classes

              ////checkTrancheTenors();//???? loss rather
              ////initializeLengths();//????

              checkInputs(correlations_.rows(), correlations_.columns());
        updateMatrix();
              registerWithMarketData();
              /*
              interpolation_ = // so far all constructors match this...
                  Interpolator2D_T(lossLevel_.begin(), lossLevel_.end(),
                                 trancheTimes_.begin(), trancheTimes_.end(),
                                 correlations_);
                                 */
              // call factory
              setupInterpolation();
        }
    private:
        /* Notice that while this is calling a virtual method in the constructor

        1.- The virtual function is ('should be coded do that is') only accessing members from the base class not from the (unallocated) derived class.
        */
  virtual void setupInterpolation() ;//////// = 0;;
         ///OR:       virtual Interpolator2D_T& setupInterpolation() const;//////// = 0;;
    public:
        Size correlationSize() const {return 1;}
        //! Implicit correlation for the given loss interval.
        Real ImplicitCorrelation(Real, Real);

        void checkTrancheTenors() const ;
        void checkLosses() const;
        void initializeTrancheTimes() const;
        void checkInputs(Size volRows, Size volsColumns) const;
        void registerWithMarketData();

        void update();
        void updateMatrix() const;

        // TermStructure interface
        Date maxDate() const {
            return trancheDates_.back();
        }
// DESTRUCTOR?????????????-----------------------------------------
        //---------------------------------------
        Real correlation(const Date& d, Real lossLevel, bool extrapolate = false) const {
            return correlation(timeFromReference(d), lossLevel, extrapolate);
        }
        Real correlation(Time t, Real lossLevel, bool extrapolate = false) const {
            return interpolation_(t, lossLevel, true);// in correlImpl?????????
        }
    private:
        std::vector<std::vector<Handle<Quote> > > correlHandles_;
        mutable Matrix correlations_;
        Interpolation2D interpolation_;
        //Interpolator2D_T interpolation_;// because its factory constructed it should be a ptr otherwise it will be default constructed 

        // these mapped form SwaptionVolDiscrete......----------------
        Size nTrancheTenors_,
            nLosses_;
        std::vector<Period> tenors_;
        mutable std::vector<Real> lossLevel_; // used now
      //////////  std::vector<Period> optionTenors_;///>>??????????????????????????????????????
        mutable std::vector<Date> trancheDates_; // used now
        mutable std::vector<Time> trancheTimes_;// used now 
     ///////////////   mutable std::vector<Real> optionDatesAsReal_;///>>??????????????????????????????????????
     ////////////////   Interpolation optionInterpolator_;

   /////////     Size nSwapTenors_;///>>??????????????????????????????????????
        //! fractional loss of a given portfolio
///////////////////        Date evaluationDate_;///>>?????????????????????????????

    };

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::checkTrancheTenors() const {
        QL_REQUIRE(tenors_[0]>0*Days,
                   "first tranche tenor is negative (" <<
                   tenors_[0] << ")");
        for (Size i=1; i<nTrancheTenors_; ++i)
            QL_REQUIRE(tenors_[i]>tenors_[i-1],
                       "non increasing tranche tenor: " << io::ordinal(i) <<
                       " is " << tenors_[i-1] << ", " << io::ordinal(i+1) <<
                       " is " << tenors_[i]);
    }

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::checkLosses() const {
        // CHECK ZERO IS PRESENT AND INSERT IT IF NOT??????????????????????
        QL_REQUIRE(lossLevel_[0]>0.,
                   "first loss level is negative (" <<
                   lossLevel_[0] << ")");
        QL_REQUIRE(lossLevel_[0] <= 1.,
            "First loss level larger than 100% (" << lossLevel_[0] <<")");
        for (Size i=1; i<nLosses_; ++i) {
            QL_REQUIRE(lossLevel_[i]>lossLevel_[i-1],
                       "non increasing losses: " << io::ordinal(i) <<
                       " is " << lossLevel_[i-1] << ", " << io::ordinal(i+1) <<
                       " is " << lossLevel_[i]);
        QL_REQUIRE(lossLevel_[i] <= 1.,
            "Loss level " << i << " larger than 100% (" << lossLevel_[i] <<")");
        }
    }

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::initializeTrancheTimes() const {
        for (Size i=0; i<nTrancheTenors_; ++i)
            trancheTimes_[i] = timeFromReference(trancheDates_[i]);
    }



    ///------------

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::checkInputs(Size volRows,
                                               Size volsColumns) const {
        QL_REQUIRE(nLosses_==volRows,
                   "mismatch between number of loss levels (" <<
                   nLosses_ << ") and number of rows (" << volRows <<
                   ") in the correl matrix");
        QL_REQUIRE(nTrancheTenors_==volsColumns,
                   "mismatch between number of tranche tenors (" <<
                   nTrancheTenors_ << ") and number of columns (" << 
                   volsColumns << ") in the correl matrix");
        ///////////CHECK CORREL VALS MAKE SENSE/////////////////////////////
    }

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::registerWithMarketData()
    {
        for (Size i=0; i<correlHandles_.size(); ++i)
            for (Size j=0; j<correlHandles_.front().size(); ++j)
                registerWith(correlHandles_[i][j]);
    }

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::update() {
        // recalc dateS????? are we registerd with Today??? Can I use the 'moving' flag from TS for this?
        updateMatrix();
        TermStructure::update();
    }

    template <class I2D_T>
    void BaseCorrelationTermStructure<I2D_T>::updateMatrix() const {
        for (Size i=0; i<correlHandles_.size(); ++i)
            for (Size j=0; j<correlHandles_.front().size(); ++j)
                correlations_[i][j] = correlHandles_[i][j]->value();

    }

    // UPDATE ======UPDATE ======UPDATE ======UPDATE ======UPDATE ======UPDATE ======UPDATE ======UPDATE ======


    ////////void SwaptionVolatilityMatrix::performCalculations() const {

    ////////    SwaptionVolatilityDiscrete::performCalculations();

    ////////    // we might use iterators here...
    ////////    for (Size i=0; i<volatilities_.rows(); ++i)
    ////////        for (Size j=0; j<volatilities_.columns(); ++j)
    ////////            volatilities_[i][j] = volHandles_[i][j]->value();
    ////////}

/*
    //! Default Factories for some specific two dimensional interpolations through template specialization. The signature of the 2D interpolator constructor is not guaranteed.
    template<>
     void 
  //  BilinearInterpolation&
    BaseCorrelationTermStructure<BilinearInterpolation>::setupInterpolation() {
         interpolation_ =  
             BilinearInterpolation(trancheTimes_.begin(), 
            trancheTimes_.end(), lossLevel_.begin(), lossLevel_.end(), 
            correlations_);
    }



     // See that some interpolators might take you out of the [-1,1] correlation domain.

    template<>
     void 
  //  BilinearInterpolation&
    BaseCorrelationTermStructure<BicubicSpline>::setupInterpolation() {
         interpolation_ =  
             BicubicSpline(trancheTimes_.begin(), 
            trancheTimes_.end(), lossLevel_.begin(), lossLevel_.end(), 
            correlations_);
    }

*/
}

#endif
