
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file calibrationhelper.hpp
    \brief Calibration helper class
*/

#ifndef quantlib_interest_rate_modelling_calibration_helper_h
#define quantlib_interest_rate_modelling_calibration_helper_h

#include <ql/grid.hpp>
#include <ql/pricingengine.hpp>
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib {

    //! liquid market instrument used during calibration
    class CalibrationHelper : public Observer, public Observable {
      public:
        CalibrationHelper(const RelinkableHandle<Quote>& volatility,
                          const RelinkableHandle<TermStructure>& termStructure)
        : volatility_(volatility), termStructure_(termStructure) {
            blackModel_ = boost::shared_ptr<BlackModel>(
                                  new BlackModel(volatility_,termStructure_));
            registerWith(volatility_);
            registerWith(termStructure_);
        }
        void update() { 
            marketValue_ = blackPrice(volatility_->value());
            notifyObservers(); 
        }

        //! returns the actual price of the instrument (from volatility)
        double marketValue() { return marketValue_; }

        //! returns the price of the instrument according to the model
        virtual double modelValue() = 0;

        //! returns the error resulting from the model valuation
        virtual double calibrationError() {
            return QL_FABS(marketValue() - modelValue())/marketValue();
        }

        virtual void addTimesTo(std::list<Time>& times) const = 0;

        //! Black volatility implied by the model
        double impliedVolatility(double targetValue,
                                 double accuracy,
                                 Size maxEvaluations,
                                 double minVol,
                                 double maxVol) const;

        //! Black price given a volatility
        virtual double blackPrice(double volatility) const = 0;

        void setPricingEngine(const boost::shared_ptr<PricingEngine>& engine) {
            engine_ = engine;
        }

      protected:
        double marketValue_;
        RelinkableHandle<Quote> volatility_;
        RelinkableHandle<TermStructure> termStructure_;
        boost::shared_ptr<BlackModel> blackModel_;
        boost::shared_ptr<PricingEngine> engine_;

      private:
        class ImpliedVolatilityHelper;
    };

    #ifndef QL_DISABLE_DEPRECATED
    //! Set of calibration instruments
    /*! For the moment, this is just here to facilitate the assignment of a
        pricing engine to a set of calibration helpers

        \deprecated use a std::vector of CalibrationHelpers instead
    */
    class CalibrationSet 
        : public std::vector<boost::shared_ptr<CalibrationHelper> > {
      public:
        void setPricingEngine(const boost::shared_ptr<PricingEngine>& engine) {
            for (Size i=0; i<size(); i++)
                (*this)[i]->setPricingEngine(engine);
        }
    };
    #endif

}


#endif
