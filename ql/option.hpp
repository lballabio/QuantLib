
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file option.hpp
    \brief Base option class

    \fullpath
    ql/%option.hpp
*/

// $Id$

#ifndef quantlib_option_h
#define quantlib_option_h

#include <ql/instrument.hpp>
#include <ql/pricingengine.hpp>

namespace QuantLib {

    //! base option class
    class Option : public Instrument {
      public:
        enum Type { Call, Put, Straddle };
        Option(const Handle<PricingEngine>& engine,
               const std::string& isinCode = "",
               const std::string& description = "");
        virtual ~Option();
        double errorEstimate() const;
        void setPricingEngine(const Handle<PricingEngine>&);
      protected:
        mutable double errorEstimate_;
        virtual void setupEngine() const = 0;
        /*! \warning this method simply launches the engine and copies the 
                returned value into NPV_. It does <b>not</b> set isExpired_. 
                This should be taken care of by redefining this method in
                derived classes and calling this implementation after 
                checking for validity and only if the check succeeded.
        */
        virtual void performCalculations() const;
        Handle<PricingEngine> engine_;
    };

}


#endif
