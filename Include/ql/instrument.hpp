
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file instrument.hpp
    \brief Abstract instrument class

    $Id$
*/

// $Source$
// $Log$
// Revision 1.5  2001/05/28 12:52:58  lballabio
// Simplified Instrument interface
//
// Revision 1.4  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_instrument_h
#define quantlib_instrument_h

#include "ql/qldefines.hpp"
#include <string>

namespace QuantLib {

    //! Abstract instrument class
    /*! This class is purely abstract and defines the interface of concrete
        instruments which will be derived from this one.

        \todo Methods should be added for adding a spread to the term structure
        or volatility surface used to price the instrument.
    */
    class Instrument {
      public:
        Instrument(const std::string& isinCode = "",
            const std::string& description = "")
        : isinCode_(isinCode), description_(description),
          NPV_(0.0), isExpired_(false) {}
        virtual ~Instrument() {}
        //! \name Inspectors
        //@{
        //! returns the ISIN code of the instrument.
        std::string isinCode() const;
        //! returns a brief textual description of the instrument.
        std::string description() const;
        //! returns the net present value of the instrument.
        double NPV() const;
        //@}
      protected:
        /*! \name Calculations
            These methods do not modify the structure of the instrument and are
            therefore declared as <tt>const</tt>. Temporary variables will be
            declared as mutable.
        */
        //@{
        /*! This method performs all needed calculations by calling
            the <b>performCalculations</b> method.
            \note The current implementation of this method does nothing more 
            than calling <b>performCalculations</b> and might seem unnecessary. 
            However, it will eventually contain control code to check whether 
            the previous results are still valid, or a recalculation is needed.
            \warning This method should <b>not</b> be redefined in derived 
            classes.
        */
        void calculate() const;
        /*! This method must implement any calculations which must be (re)done
            in order to calculate the NPV of the instrument.
        */
        virtual void performCalculations() const = 0;
        //@}

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

    inline void Instrument::calculate() const {
        // eventually we will check whether previous calculations 
        // are still valid
        performCalculations();
    }

}


#endif
