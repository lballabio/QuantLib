
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_financial_instrument_h
#define quantlib_financial_instrument_h

#include "qldefines.h"
#include "termstructure.h"
#include "swaptionvolsurface.h"
#include "forwardvolsurface.h"

QL_BEGIN_NAMESPACE(QuantLib)

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
	// modifiers
	virtual void setPrice(double price) = 0;
	virtual void setTermStructure(const Handle<TermStructure>&);
	virtual void setSwaptionVolatility(const Handle<SwaptionVolatilitySurface>&);
	virtual void setForwardVolatility(const Handle<ForwardVolatilitySurface>&);
	// inspectors
	std::string isinCode() const;
	std::string description() const;
	virtual bool useTermStructure() const = 0;
	Handle<TermStructure> termStructure() const;
	virtual bool useSwaptionVolatility() const = 0;
	Handle<SwaptionVolatilitySurface> swaptionVolatility() const;
	virtual bool useForwardVolatility() const = 0;
	Handle<ForwardVolatilitySurface> forwardVolatility() const;
	double NPV() const;
	virtual double price() const = 0;
  protected:
	// observers
	// term structure
	class TermStructureObserver;
	friend class TermStructureObserver;
	class TermStructureObserver : public Observer {
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
	class SwaptionVolObserver : public Observer {
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
	class ForwardVolObserver : public Observer {
	  public:
		ForwardVolObserver(Instrument* i = 0) : theInstrument(i) {}
		void update() { theInstrument->forwardVolHasChanged = true; }
	  private:
		Instrument* theInstrument;
	};
	ForwardVolObserver theForwardVolObserver;
	void registerToForwardVol();
	void unregisterFromForwardVol();
	// calculations
	// these methods do not modify the structure - temporary variables will be declared as mutable
	void calculate() const;
	virtual void performTermStructureCalculations() const {};
	virtual void performSwaptionVolCalculations() const {};
	virtual void performForwardVolCalculations() const {};
	virtual bool needsFinalCalculations() const;
	virtual void performFinalCalculations() const {};
	// data members
	std::string theISINCode, theDescription;
	Handle<TermStructure> theTermStructure;
	Handle<SwaptionVolatilitySurface> theSwaptionVol;
	Handle<ForwardVolatilitySurface> theForwardVol;
	Date theSettlementDate;
	// temporaries
	mutable double theNPV;
	mutable bool termStructureHasChanged, swaptionVolHasChanged, forwardVolHasChanged;
	mutable bool expired;
};

// comparisons based on ISIN code

QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator==(const Handle<Instrument>&, const Handle<Instrument>&))
QL_DECLARE_TEMPLATE_SPECIALIZATION(bool operator!=(const Handle<Instrument>&, const Handle<Instrument>&))


// derived classes

class PricedInstrument : public Instrument {
  public:
	PricedInstrument() : priceIsSet(false) {}
	PricedInstrument(const std::string& isinCode, const std::string& description)
	: Instrument(isinCode,description), priceIsSet(false) {}
	void setPrice(double price) { thePrice = price; priceIsSet = true; }
	double price() const { Require(priceIsSet, "price not set"); return thePrice; }
  protected:
	bool priceIsSet;
  private:
	double thePrice;
};

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

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator==(const Handle<Instrument>& i, const Handle<Instrument>& j) {
	return (i->isinCode() == j->isinCode());
}

QL_DEFINE_TEMPLATE_SPECIALIZATION
inline bool operator!=(const Handle<Instrument>& i, const Handle<Instrument>& j) {
	return (i->isinCode() != j->isinCode());
}


QL_END_NAMESPACE(QuantLib)


#endif
