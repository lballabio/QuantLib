
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/14 17:09:47  lballabio
    Went for simplicity and removed Observer-Observable relationships from Instrument

    Revision 1.1  2001/04/09 14:03:54  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.2  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/

/*! \file instrument.hpp
    \brief Abstract instrument class
*/

#ifndef quantlib_financial_instrument_h
#define quantlib_financial_instrument_h

#include "ql/qldefines.hpp"
#include "ql/termstructure.hpp"
#include "ql/swaptionvolsurface.hpp"
#include "ql/forwardvolsurface.hpp"
#include "ql/null.hpp"

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
        : theISINCode(isinCode), theDescription(description),
          theSettlementDate(Date()), theNPV(0.0),
          expired(false) {}
        virtual ~Instrument() {}
        //! \name Modifiers
        //@{
        //! sets the price for instruments which allow to do so.
        virtual void setPrice(double price) = 0;
        //! sets the term structure to be used for pricing.
        virtual void setTermStructure(const Handle<TermStructure>&);
        //! sets the swaption volatility surface to be used for pricing.
        virtual void setSwaptionVolatility(
            const Handle<SwaptionVolatilitySurface>&);
        //! sets the forward volatility surface to be used for pricing.
        virtual void setForwardVolatility(
            const Handle<ForwardVolatilitySurface>&);
        //@}

        //! \name Inspectors
        //@{
        //! returns the ISIN code of the instrument.
        std::string isinCode() const;
        //! returns a brief textual description of the instrument.
        std::string description() const;
        //! checks whether a term structure is needed for pricing.
        virtual bool useTermStructure() const = 0;
        //! returns the term structure used for pricing.
        Handle<TermStructure> termStructure() const;
        //! checks whether a swaption volatility surface is needed for pricing.
        virtual bool useSwaptionVolatility() const = 0;
        //! returns the swaption volatility surface used for pricing.
        Handle<SwaptionVolatilitySurface> swaptionVolatility() const;
        //! checks whether a forward volatility surface is needed for pricing.
        virtual bool useForwardVolatility() const = 0;
        //! returns the forward volatility surface used for pricing.
        Handle<ForwardVolatilitySurface> forwardVolatility() const;
        //! returns the net present value of the instrument.
        double NPV() const;
        //! returns the price of the instrument.
        virtual double price() const = 0;
        //@}
      protected:
        /*! \name Calculations
            These methods do not modify the structure of the instrument and are
            therefore declared as <tt>const</tt>. Temporary variables will be
            declared as mutable.
        */
        //@{
        /*! This method performs all needed calculations by calling
            <b>performTermStructureCalculations</b>,
            <b>performSwaptionVolCalculations</b>,
            <b>performForwardVolCalculations</b>, and
            <b>performFinalCalculations</b> in turn while checking that the 
            needed structures are not null. 
            It should not be redefined in derived classes.
        */
        void calculate() const;
        /*! This method must implement any calculations which must be (re)done
            in case the term structure is set or changes. A default is supplied
            with a null body.
        */
        virtual void performTermStructureCalculations() const {}
        /*! This method must implement any calculations which must be (re)done
            in case the swaption volatility surface is set or changes. A default
            is supplied with a null body.
        */
        virtual void performSwaptionVolCalculations() const {}
        /*! This method must implement any calculations which must be (re)done
            in case the forward volatility surface is set or changes. A default
            is supplied with a null body.
        */
        virtual void performForwardVolCalculations() const {}
        /*! This method must implement any calculations which are needed besides
            the ones implemented in <b>performTermStructureCalculations</b>,
            <b>performSwaptionVolCalculations</b>, and
            <b>performForwardVolCalculations</b>.
            A default is supplied with a null body.
        */
        virtual void performFinalCalculations() const {}
        //@}

        //! \name Results
        //@{
        /*! The value of this attribute must be set by the instrument
            constructor.
        */
        Date theSettlementDate;
        /*! The value of this attribute must be set by either of the
            <b>performXxxCalculations</b> methods.
        */
        mutable double theNPV;
        /*! The value of this attribute must be set to <tt>true</tt> by either
            of the <b>performXxxCalculations</b> methods if the instrument is
            expired.
        */
        mutable bool expired;
        //@}
      private:
        // data members
        std::string theISINCode, theDescription;
        Handle<TermStructure> theTermStructure;
        Handle<SwaptionVolatilitySurface> theSwaptionVol;
        Handle<ForwardVolatilitySurface> theForwardVol;
    };

    bool operator==(const Handle<Instrument>&, const Handle<Instrument>&);
    bool operator!=(const Handle<Instrument>&, const Handle<Instrument>&);

    // derived classes

    //! Priced instrument class
    /*! It implements the Instrument interface for instruments
        whose prices are available on the market.
    */
    class PricedInstrument : public Instrument {
      public:
        PricedInstrument(const std::string& isinCode = "",
            const std::string& description = "")
        : Instrument(isinCode,description), priceIsSet(false) {}
        void setPrice(double price) { thePrice = price; priceIsSet = true; }
        /*! \pre The price must have been set with <B><I>setPrice()</B></I> */
        double price() const {
            QL_REQUIRE(priceIsSet, "price not set");
            return thePrice;
        }
        bool useTermStructure() const { return false; }
        bool useSwaptionVolatility() const { return false; }
        bool useForwardVolatility() const { return false; }
      private:
        /* this method will throw an exception if not set, thus acting as a
           check
        */
        void performFinalCalculations() const { theNPV = price(); }
        bool priceIsSet;
        double thePrice;
    };

    //! Over-the-counter instrument class
    /*! It inhibits the <b>setPrice</b> method and redirects the <b>price</b>
        method to <b>NPV</b> for over-the-counter instruments.
    */
    class OTCInstrument : public Instrument { // over the counter
      public:
        OTCInstrument(const std::string& isinCode = "",
            const std::string& description = "")
        : Instrument(isinCode,description) {}
        void setPrice(double price) { throw Error("Cannot set price"); }
        double price() const { return NPV(); }
    };


    // inline definitions

    inline void Instrument::setTermStructure(
      const Handle<TermStructure>& termStructure) {
        if (useTermStructure()) {
            theTermStructure = termStructure;
            theSettlementDate = termStructure->settlementDate();
        }
    }

    inline void Instrument::setSwaptionVolatility(
      const Handle<SwaptionVolatilitySurface>& vol) {
        if (useSwaptionVolatility()) {
            theSwaptionVol = vol;
        }
    }

    inline void Instrument::setForwardVolatility(
      const Handle<ForwardVolatilitySurface>& vol) {
        if (useForwardVolatility()) {
            theForwardVol = vol;
        }
    }

    inline std::string Instrument::isinCode() const {
        return theISINCode;
    }

    inline std::string Instrument::description() const {
        return theDescription;
    }

    inline double Instrument::NPV() const {
        calculate();
        return (expired ? 0.0 : theNPV);
    }

    /*! \pre The term structure must have been set */
    inline Handle<TermStructure> Instrument::termStructure() const {
        QL_REQUIRE(!theTermStructure.isNull(),
            "term structure not set");
        return theTermStructure;
    }

    /*! \pre The swaption volatility surface must have been set */
    inline Handle<SwaptionVolatilitySurface> Instrument::swaptionVolatility()
    const {
        QL_REQUIRE(!theSwaptionVol.isNull(),
            "swaption volatility surface not set");
        return theSwaptionVol;
    }

    /*! \pre The forward volatility surface must have been set */
    inline Handle<ForwardVolatilitySurface> Instrument::forwardVolatility()
    const {
        QL_REQUIRE(!theForwardVol.isNull(),
            "forward volatility surface not set");
        return theForwardVol;
    }

    inline void Instrument::calculate() const {
        if (useTermStructure()) {
            QL_REQUIRE(!theTermStructure.isNull(),
                "term structure not set");
            performTermStructureCalculations();
        }
        if (useSwaptionVolatility()) {
            QL_REQUIRE(!theSwaptionVol.isNull(),
                "swaption volatility surface not set");
            performSwaptionVolCalculations();
        }
        if (useForwardVolatility()) {
            QL_REQUIRE(!theForwardVol.isNull(),
                "forward volatility surface not set");
            performForwardVolCalculations();
        }
        performFinalCalculations();
    }

    // comparisons

    /*! \relates Instrument
        \brief returns <tt>true</tt> iff two instruments have the same ISIN code
    */
    inline bool operator==(const Handle<Instrument>& i,
        const Handle<Instrument>& j) {
            return (i->isinCode() == j->isinCode());
    }

    /*! \relates Instrument */
    inline bool operator!=(const Handle<Instrument>& i,
        const Handle<Instrument>& j) {
            return (i->isinCode() != j->isinCode());
    }

}


#endif
