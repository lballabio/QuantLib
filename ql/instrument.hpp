
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

/*! \file instrument.hpp
    \brief Abstract instrument class
*/

// $Id$

#ifndef quantlib_instrument_h
#define quantlib_instrument_h

#include <ql/Patterns/lazyobject.hpp>

/*! \namespace QuantLib::Instruments
    \brief Concrete implementations of the Instrument interface

    See sect. \ref instruments
*/

namespace QuantLib {

    //! Abstract instrument class
    /*! This class is purely abstract and defines the interface of concrete
        instruments which will be derived from this one.
    */
    class Instrument : public Patterns::LazyObject {
      public:
        Instrument(const std::string& isinCode = "",
                   const std::string& description = "");
        virtual ~Instrument() {}

        //! \name Inspectors
        //@{
        //! returns the ISIN code of the instrument, when given.
        std::string isinCode() const;
        //! returns a brief textual description of the instrument.
        std::string description() const;
        //! returns the net present value of the instrument.
        double NPV() const;
        //! returns whether the instrument is still tradable.
        bool isExpired() const;
        //@}
      protected:
        /*! \name Results
            The value of these attributes must be set in the body of the
            <b>performCalculations</b> method.
        */
        //@{
        mutable double NPV_;
        mutable bool isExpired_;
        //@}
      private:
        std::string isinCode_, description_;
    };

    // inline definitions

    inline Instrument::Instrument(const std::string& isinCode,
                                  const std::string& description)
    : NPV_(0.0), isExpired_(false), isinCode_(isinCode),
      description_(description) {}

    inline std::string Instrument::isinCode() const {
        return isinCode_;
    }

    inline std::string Instrument::description() const {
        return description_;
    }

    inline double Instrument::NPV() const {
        calculate();
        return (isExpired_ ? 0.0 : NPV_);
    }

    inline bool Instrument::isExpired() const {
        calculate();
        return isExpired_;
    }

}


#endif
