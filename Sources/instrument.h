
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

/*! \file instrument.h
	\brief Abstract instrument class
*/

#ifndef quantlib_financial_instrument_h
#define quantlib_financial_instrument_h

#include "qldefines.h"
#include "termstructure.h"
#include "swaptionvolsurface.h"
#include "forwardvolsurface.h"

namespace QuantLib {

	//! Abstract instrument class
	/*! This class is purely abstract and defines the interface of concrete
		instruments which will be derived from this one.
	*/
	class Instrument {
	  public:
		Instrument()
		: termStructureHasChanged(true), swaptionVolHasChanged(true), forwardVolHasChanged(true), 
		  theSettlementDate(Date()), theNPV(0.0), expired(false) {}
		Instrument(const std::string& isinCode, const std::string& description)
		: theISINCode(isinCode), theDescription(description), 
		  termStructureHasChanged(true), swaptionVolHasChanged(true), forwardVolHasChanged(true), 
		  theSettlementDate(Date()), theNPV(0.0), expired(false) {}
		virtual ~Instrument();
		//! \name Modifiers
		//@{
		//! sets the price for instruments which allow to do so.
		virtual void setPrice(double price) = 0;
		//! sets the term structure to be used for pricing.
		virtual void setTermStructure(const Handle<TermStructure>&);
		//! sets the swaption volatility surface to be used for pricing.
		virtual void setSwaptionVolatility(const Handle<SwaptionVolatilitySurface>&);
		//! sets the forward volatility surface to be used for pricing.
		virtual void setForwardVolatility(const Handle<ForwardVolatilitySurface>&);
		//@}
		
		//! \name Inspectors
		//@{
		//! returns the ISIN code of the instrument.
		std::string isinCode() const;
		//! returns a brief textual description of the instrument.
		std::string description() const;
		//! returns <tt>true</tt> iff the instrument needs a term structure for pricing.
		virtual bool useTermStructure() const = 0;
		//! returns the term structure used for pricing.
		Handle<TermStructure> termStructure() const;
		//! returns <tt>true</tt> iff the instrument needs a swaption volatility surface for pricing.
		virtual bool useSwaptionVolatility() const = 0;
		//! returns the swaption volatility surface used for pricing.
		Handle<SwaptionVolatilitySurface> swaptionVolatility() const;
		//! returns <tt>true</tt> iff the instrument needs a forward volatility surface for pricing.
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
			These methods do not modify the structure of the instrument and are therefore declared
			as <tt>const</tt>. Temporary variables will be declared as mutable.
		*/
		//@{
		/*! This method must implement any calculations which must be (re)done in case
			the term structure is set or changes. A default is supplied with a null body. */
		virtual void performTermStructureCalculations() const {}
		/*! This method must implement any calculations which must be redone in case
			the swaption volatility surface is set or changes. A default is supplied with a null body. */
		virtual void performSwaptionVolCalculations() const {}
		/*! This method must implement any calculations which must be redone in case
			the forward volatility surface is set or changes. A default is supplied with a null body. */
		virtual void performForwardVolCalculations() const {}
		/*! This method must return <tt>true</tt> if any calculations are needed besides 
			the ones implemented in <b>performTermStructureCalculations</b>, 
			<b>performSwaptionVolCalculations</b>, and <b>performForwardVolCalculations</b>. */
		virtual bool needsFinalCalculations() const;
		/*! This method must implement any calculations which are needed besides 
			the ones implemented in <b>performTermStructureCalculations</b>, 
			<b>performSwaptionVolCalculations</b>, and <b>performForwardVolCalculations</b>.
			A default is supplied with a null body. */
		virtual void performFinalCalculations() const {}
		//@}
		
		//! \name Results
		//@{
		/*! The value of this attribute must be set by the instrument constructor.
		*/
		Date theSettlementDate;
		/*! The value of this attribute must be set by either of the <b>performXxxCalculations</b>
			methods.
		*/
		mutable double theNPV;
		/*! The value of this attribute must be set to <tt>true</tt> by either of the 
			<b>performXxxCalculations</b> methods if the instrument is expired.
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
		mutable bool termStructureHasChanged, swaptionVolHasChanged, forwardVolHasChanged;
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

	/*! \relates Instrument
		\brief returns <tt>true</tt> iff two instruments have the same ISIN code
	*/
	QL_DECLARE_TEMPLATE_SPECIALIZATION(
	bool operator==(const Handle<Instrument>&, const Handle<Instrument>&))
	/*! \relates Instrument */
	QL_DECLARE_TEMPLATE_SPECIALIZATION(
	bool operator!=(const Handle<Instrument>&, const Handle<Instrument>&))
	
	
	// derived classes
	
	//! Abstract instrument class
	/*! It implements the <b>setPrice</b> and <b>price</b> methods for instruments
		for instruments whose prices are available on the market.
	*/
	class PricedInstrument : public Instrument {
	  public:
		PricedInstrument() : priceIsSet(false) {}
		PricedInstrument(const std::string& isinCode, const std::string& description)
		: Instrument(isinCode,description), priceIsSet(false) {}
		void setPrice(double price) { thePrice = price; priceIsSet = true; }
		double price() const { Require(priceIsSet, "price not set"); return thePrice; }
	  private:
		bool priceIsSet;
		double thePrice;
	};
	
	//! Abstract instrument class
	/*! It inhibits the <b>setPrice</b> method and redirects the <b>price</b> method to <b>NPV</b>
		for over-the-counter instruments.
	*/
	class OTCInstrument : public Instrument { // over the counter
	  public:
		OTCInstrument() {}
		OTCInstrument(const std::string& isinCode, const std::string& description)
		: Instrument(isinCode,description) {}
		void setPrice(double price) { throw Error("Cannot set price"); }
		double price() const { return NPV(); }
	};
	
	
	// inline definitions
	
	inline Instrument::~Instrument() {
		unregisterFromTermStructure();
	}
	
	inline void Instrument::setTermStructure(const Handle<TermStructure>& termStructure) {
		if (useTermStructure()) {
			unregisterFromTermStructure();
			theTermStructure = termStructure;
			registerToTermStructure();
			theSettlementDate = termStructure->settlementDate();
			termStructureHasChanged = true;
		}
	}
	
	inline void Instrument::registerToTermStructure() {
		if (!IsNull(theTermStructure)) {
			theTermStructureObserver = TermStructureObserver(this);
			theTermStructure->registerObserver(&theTermStructureObserver);
		}
	}
	
	inline void Instrument::unregisterFromTermStructure() {
		if (!IsNull(theTermStructure))
			theTermStructure->unregisterObserver(&theTermStructureObserver);
	}
	
	inline void Instrument::setSwaptionVolatility(const Handle<SwaptionVolatilitySurface>& vol) {
		if (useSwaptionVolatility()) {
			unregisterFromSwaptionVol();
			theSwaptionVol = vol;
			registerToSwaptionVol();
			swaptionVolHasChanged = true; 
		}
	}
	
	inline void Instrument::registerToSwaptionVol() {
		if (!IsNull(theSwaptionVol)) {
			theSwaptionVolObserver = SwaptionVolObserver(this);
			theSwaptionVol->registerObserver(&theSwaptionVolObserver);
		}
	}
	
	inline void Instrument::unregisterFromSwaptionVol() {
		if (!IsNull(theSwaptionVol))
			theSwaptionVol->unregisterObserver(&theSwaptionVolObserver);
	}
	
	inline void Instrument::setForwardVolatility(const Handle<ForwardVolatilitySurface>& vol) {
		if (useForwardVolatility()) {
			unregisterFromForwardVol();
			theForwardVol = vol;
			registerToForwardVol();
			forwardVolHasChanged = true; 
		}
	}
	
	inline void Instrument::registerToForwardVol() {
		if (!IsNull(theForwardVol)) {
			theForwardVolObserver = ForwardVolObserver(this);
			theForwardVol->registerObserver(&theForwardVolObserver);
		}
	}
	
	inline void Instrument::unregisterFromForwardVol() {
		if (!IsNull(theForwardVol))
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
		return (expired ? 0.0 :theNPV); 
	}
	
	inline Handle<TermStructure> Instrument::termStructure() const {
		Require(!IsNull(theTermStructure),"term structure not set");
		return theTermStructure;
	}
	
	inline Handle<SwaptionVolatilitySurface> Instrument::swaptionVolatility() const { 
		Require(!IsNull(theSwaptionVol),"swaption volatility matrix not set");
		return theSwaptionVol; 
	}
	
	inline Handle<ForwardVolatilitySurface> Instrument::forwardVolatility() const { 
		Require(!IsNull(theForwardVol),"forward volatility matrix not set");
		return theForwardVol; 
	}
	
	inline void Instrument::calculate() const {
		if (termStructureHasChanged)
			performTermStructureCalculations();
		if (swaptionVolHasChanged)
			performSwaptionVolCalculations();
		if (forwardVolHasChanged)
			performForwardVolCalculations();
		if (needsFinalCalculations())
			performFinalCalculations();
		termStructureHasChanged = swaptionVolHasChanged = forwardVolHasChanged = false;
	}
	
	inline bool Instrument::needsFinalCalculations() const {
		return (termStructureHasChanged || swaptionVolHasChanged || forwardVolHasChanged);
	}
	
	// comparisons 
	
	QL_TEMPLATE_SPECIALIZATION
	inline bool operator==(const Handle<Instrument>& i, const Handle<Instrument>& j) {
		return (i->isinCode() == j->isinCode());
	}
	
	QL_TEMPLATE_SPECIALIZATION
	inline bool operator!=(const Handle<Instrument>& i, const Handle<Instrument>& j) {
		return (i->isinCode() != j->isinCode());
	}

}


#endif
