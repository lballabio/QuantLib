
/*
 Copyright (C) 2000, 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file affinetermstructure.hpp
    \brief Affine term structure
*/

#ifndef quantlib_term_structures_affine_h
#define quantlib_term_structures_affine_h

#include <ql/dataformatters.hpp>
#include <ql/ShortRateModels/model.hpp>
#include <ql/Optimization/method.hpp>
#include <ql/TermStructures/ratehelpers.hpp>

namespace QuantLib {

    //! Term-structure implied by an affine model
    /*! This class defines a term-structure that is based on an affine
        model, e.g. Vasicek or Cox-Ingersoll-Ross. It either be
        instanced using a model with defined arguments, or the model
        can be calibrated to a set of rate helpers. Of course, there
        is no point in using a term-structure consistent affine model,
        since the implied term-structure will just be the initial
        term-structure on which the model is based.
    */
    class AffineTermStructure : public DiscountStructure,
                                public Observer {
      public:
        //! constructor using a fixed model
        AffineTermStructure(const Date& todaysDate,
                            const Date& referenceDate,
                            const boost::shared_ptr<AffineModel>& model,
                            const DayCounter& dayCounter);
        //! constructor using a model that has to be calibrated
        AffineTermStructure(const Date& todaysDate,
                            const Date& referenceDate,
                            const boost::shared_ptr<AffineModel>& model,
                            const std::vector<boost::shared_ptr<RateHelper> >&,
                            const boost::shared_ptr<OptimizationMethod>&,
                            const DayCounter& dayCounter);

        // inspectors
        DayCounter dayCounter() const;
        Date todaysDate() const {return todaysDate_; }
        Date referenceDate() const;
        Date maxDate() const;

        void update();
      protected:
        DiscountFactor discountImpl(Time,
                                    bool extrapolate = false) const;
      private:
        class CalibrationFunction;

        void calibrate() const;

        DayCounter dayCounter_;
        Date todaysDate_, referenceDate_;

        mutable bool needsRecalibration_;
        boost::shared_ptr<AffineModel> model_;
        std::vector<boost::shared_ptr<RateHelper> > instruments_;
        boost::shared_ptr<OptimizationMethod> method_;
    };

    inline DayCounter AffineTermStructure::dayCounter() const {
        return dayCounter_;
    }

    inline Date AffineTermStructure::referenceDate() const {
        return referenceDate_;
    }

    inline Date AffineTermStructure::maxDate() const {
        return Date::maxDate();
    }

    inline void AffineTermStructure::update() {
        needsRecalibration_ = true;
        notifyObservers();
    }

    inline
    DiscountFactor AffineTermStructure::discountImpl(Time t, bool) const {
        if (needsRecalibration_) calibrate();
        QL_REQUIRE(t >= 0.0,
                   "discount undefined for time (" +
                   DoubleFormatter::toString(t) + ")");
        return model_->discount(t);
    }

}


#endif
