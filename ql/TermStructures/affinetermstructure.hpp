/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file affinetermstructure.hpp
    \brief Affine term structure

    \fullpath
    ql/TermStructures/%affinetermstructure.hpp
*/

// $Id$

#ifndef quantlib_term_structures_affine_h
#define quantlib_term_structures_affine_h

#include <ql/dataformatters.hpp>
#include <ql/ShortRateModels/model.hpp>
#include <ql/Optimization/method.hpp>
#include <ql/TermStructures/ratehelpers.hpp>

namespace QuantLib {

    namespace TermStructures {

        //! Term-structure implied by an affine model
        /*! This class defines a term-structure that is based on an affine
            model, e.g. Vasicek or Cox-Ingersoll-Ross. It either be instanced
            using a model with defined parameters, or the model can be 
            calibrated to a set of rate helpers. Of course, there is no point
            in using a term-structure consistent affine model, since the implied
            term-structure will just be the initial term-structure on which the
            model is based.
        */
        class AffineTermStructure : public DiscountStructure,
                                    public Patterns::Observer {
          public:
            //! constructor using a fixed model
            AffineTermStructure(
                Currency currency,
                const DayCounter& dayCounter,
                const Date& todaysDate,
                const Date& settlementDate,
                const Handle<ShortRateModels::AffineModel>& model);
            //! constructor using a model that has to be calibrated
            AffineTermStructure(
                Currency currency,
                const DayCounter& dayCounter,
                const Date& todaysDate,
                const Date& settlementDate,
                const Handle<ShortRateModels::AffineModel>& model,
                const std::vector<Handle<RateHelper> >& instruments,
                const Handle<Optimization::Method>& method);

            // inspectors
            Currency currency() const;
            DayCounter dayCounter() const;
            Date todaysDate() const;
            Date settlementDate() const;
            Date maxDate() const;
            Date minDate() const;
            Time maxTime() const;
            Time minTime() const;

            void update();
          protected:
            DiscountFactor discountImpl(Time,
                bool extrapolate = false) const;
          private:
            class CalibrationFunction;

            void calibrate() const;

            Currency currency_;
            DayCounter dayCounter_;
            Date todaysDate_;
            Date settlementDate_;

            mutable bool needsRecalibration_;
            Handle<ShortRateModels::AffineModel> model_;
            std::vector<Handle<RateHelper> > instruments_;
            Handle<Optimization::Method> method_;
        };

        inline Currency AffineTermStructure::currency() const {
            return currency_;
        }

        inline DayCounter AffineTermStructure::dayCounter() const {
            return dayCounter_;
        }

        inline Date AffineTermStructure::todaysDate() const {
            return todaysDate_;
        }

        inline Date AffineTermStructure::settlementDate() const {
            return settlementDate_;
        }

        inline Date AffineTermStructure::maxDate() const {
            return Date::maxDate();
        }

        inline Date AffineTermStructure::minDate() const {
            return settlementDate_;
        }

        inline Time AffineTermStructure::maxTime() const {
            return dayCounter().yearFraction(
                settlementDate(), Date::maxDate());
        }

        inline Time AffineTermStructure::minTime() const {
            return 0.0;
        }

        inline void AffineTermStructure::update() {
            needsRecalibration_ = true;
            notifyObservers();
        }

        inline 
        DiscountFactor AffineTermStructure::discountImpl(Time t, bool) const {
            if (needsRecalibration_) calibrate();
            QL_REQUIRE(t >= 0.0,
                "AffineTermStructure: discount requested for negative time (" +
                DoubleFormatter::toString(t) + ")");
            return model_->discount(t);
        }

    }

}


#endif
