
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file instrument.h
    \brief Abstract instrument class

    $Source$
    $Log$
    Revision 1.14  2001/03/12 17:35:09  lballabio
    Removed global IsNull function - could have caused very vicious loops

    Revision 1.13  2001/02/16 15:19:52  lballabio
    Used QL_DECLARE_TEMPLATE_SPECIFICATIONS macro

    Revision 1.12  2001/02/09 19:21:09  lballabio
    removed QL_DECLARE_TEMPLATE_SPECIALIZATION macro

*/

#ifndef quantlib_financial_instrument_h
#define quantlib_financial_instrument_h

#include "qldefines.h"
#include "termstructure.h"
#include "swaptionvolsurface.h"
#include "forwardvolsurface.h"
#include "null.h"

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
          termStructureHasChanged(true), swaptionVolHasChanged(true), 
          forwardVolHasChanged(true), theSettlementDate(Date()), theNPV(0.0), 
          expired(false) {}
        virtual ~Instrument();
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
        /*! This method must return <tt>true</tt> if any calculations are needed 
            besides the ones implemented in 
            <b>performTermStructureCalculations</b>,
            <b>performSwaptionVolCalculations</b>, and 
            <b>performForwardVolCalculations</b>.
        */
        virtual bool needsFinalCalculations() const;
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
        void calculate() const;
        // data members
        std::string theISINCode, theDescription;
        Handle<TermStructure> theTermStructure;
        Handle<SwaptionVolatilitySurface> theSwaptionVol;
        Handle<ForwardVolatilitySurface> theForwardVol;
        // temporaries
        mutable bool termStructureHasChanged, swaptionVolHasChanged, 
            forwardVolHasChanged;
        // observers
        // term structure
        class TermStructureObserver;
        friend class TermStructureObserver;
        class TermStructureObserver : public Patterns::Observer {
          public:
            TermStructureObserver(Instrument* i = 0) : theInstrument(i) {}
            void update() { theInstrument->termStructureHasChanged = true; }
          private:
            Instrument* theInstrument;
        };
        TermStructureObserver theTermStructureObserver;
        void registerToTermStructure();
        void unregisterFromTermStructure();
        // swaption vol
        class SwaptionVolObserver;
        friend class SwaptionVolObserver;
        class SwaptionVolObserver : public Patterns::Observer {
          public:
            SwaptionVolObserver(Instrument* i = 0) : theInstrument(i) {}
            void update() { theInstrument->swaptionVolHasChanged = true; }
          private:
            Instrument* theInstrument;
        };
        SwaptionVolObserver theSwaptionVolObserver;
        void registerToSwaptionVol();
        void unregisterFromSwaptionVol();
        // forward vol
        class ForwardVolObserver;
        friend class ForwardVolObserver;
        class ForwardVolObserver : public Patterns::Observer {
          public:
            ForwardVolObserver(Instrument* i = 0) : theInstrument(i) {}
            void update() { theInstrument->forwardVolHasChanged = true; }
          private:
            Instrument* theInstrument;
        };
        ForwardVolObserver theForwardVolObserver;
        void registerToForwardVol();
        void unregisterFromForwardVol();

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
        bool needsFinalCalculations() const { return true; }
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

    inline Instrument::~Instrument() {
        unregisterFromTermStructure();
    }

    inline void Instrument::setTermStructure(
      const Handle<TermStructure>& termStructure) {
        if (useTermStructure()) {
            unregisterFromTermStructure();
            theTermStructure = termStructure;
            registerToTermStructure();
            theSettlementDate = termStructure->settlementDate();
            termStructureHasChanged = true;
        }
    }

    inline void Instrument::registerToTermStructure() {
        if (!theTermStructure.isNull()) {
            theTermStructureObserver = TermStructureObserver(this);
            theTermStructure->registerObserver(&theTermStructureObserver);
        }
    }

    inline void Instrument::unregisterFromTermStructure() {
        if (!theTermStructure.isNull())
            theTermStructure->unregisterObserver(&theTermStructureObserver);
    }

    inline void Instrument::setSwaptionVolatility(
      const Handle<SwaptionVolatilitySurface>& vol) {
        if (useSwaptionVolatility()) {
            unregisterFromSwaptionVol();
            theSwaptionVol = vol;
            registerToSwaptionVol();
            swaptionVolHasChanged = true;
        }
    }

    inline void Instrument::registerToSwaptionVol() {
        if (!theSwaptionVol.isNull()) {
            theSwaptionVolObserver = SwaptionVolObserver(this);
            theSwaptionVol->registerObserver(&theSwaptionVolObserver);
        }
    }

    inline void Instrument::unregisterFromSwaptionVol() {
        if (!theSwaptionVol.isNull())
            theSwaptionVol->unregisterObserver(&theSwaptionVolObserver);
    }

    inline void Instrument::setForwardVolatility(
      const Handle<ForwardVolatilitySurface>& vol) {
        if (useForwardVolatility()) {
            unregisterFromForwardVol();
            theForwardVol = vol;
            registerToForwardVol();
            forwardVolHasChanged = true;
        }
    }

    inline void Instrument::registerToForwardVol() {
        if (!theForwardVol.isNull()) {
            theForwardVolObserver = ForwardVolObserver(this);
            theForwardVol->registerObserver(&theForwardVolObserver);
        }
    }

    inline void Instrument::unregisterFromForwardVol() {
        if (!theForwardVol.isNull())
            theForwardVol->unregisterObserver(&theForwardVolObserver);
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

    inline bool Instrument::needsFinalCalculations() const {
        return (termStructureHasChanged || swaptionVolHasChanged || 
            forwardVolHasChanged);
    }

    inline void Instrument::calculate() const {
        if (useTermStructure() && termStructureHasChanged) {
            QL_REQUIRE(!theTermStructure.isNull(),
                "term structure not set");
            performTermStructureCalculations();
        }
        if (useSwaptionVolatility() && swaptionVolHasChanged) {
            QL_REQUIRE(!theSwaptionVol.isNull(), 
                "swaption volatility surface not set");
            performSwaptionVolCalculations();
        }
        if (useForwardVolatility() && forwardVolHasChanged) {
            QL_REQUIRE(!theForwardVol.isNull(),
                "forward volatility surface not set");
            performForwardVolCalculations();
        }
        if (needsFinalCalculations())
            performFinalCalculations();
        termStructureHasChanged = swaptionVolHasChanged = 
            forwardVolHasChanged = false;
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
