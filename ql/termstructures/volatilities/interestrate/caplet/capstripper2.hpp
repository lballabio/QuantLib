/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file capstripper.hpp
    \brief caplet volatility stripper
*/

#ifndef quantlib_interpolated_caplet_variance_curve2_hpp
#define quantlib_interpolated_caplet_variance_curve2_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatilities/interestrate/caplet/capletvolatilitiesstructures.hpp>
#include <ql/quote.hpp>


namespace QuantLib {
    class IborIndex;
    class YieldTermStructure;
    class Quote;
    class CapVolatilitySurface;
    class SmileSection;

    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapMatrix;


    class CapsStripper2 : public LazyObject{
      public:
        CapsStripper2(){}
        CapsStripper2(const boost::shared_ptr<CapVolatilitySurface>& surface,
                      const boost::shared_ptr<IborIndex>& index,
                      Period timeStep);

       const Matrix& capletPrices() const;
       const Matrix& syntheticCapPrices() const;
       const Matrix& forwardCapVolatilities() const;
       const std::vector<boost::shared_ptr<SmileSection> >& smileSections() const;

        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations () const;
        //@}

        Real minStrike() const;
        Real maxStrike() const;
 
        const std::vector<Period>& tenors() { return tenors_; }
        const std::vector<Rate>& strikes() { return strikes_; }
      private:
        mutable Matrix syntheticCapPrices_, forwardCapsPrices_, forwardCapsVols_, forwardCapsStdev_;
        const boost::shared_ptr<CapVolatilitySurface> surface_;
        mutable CapMatrix syntheticMarketDataCap_, forwardCaps_;
        std::vector<boost::shared_ptr<SmileSection> > smileSections_;
        Date maxDate_;
        std::vector<Period> tenors_;
        std::vector<Time> tenorsTimes_;
        std::vector<Rate> strikes_;
        const boost::shared_ptr<IborIndex> index_;
    };


    class CapStripperQuote : public Quote,
                             public Observer {
      public:
        CapStripperQuote(const boost::shared_ptr<CapsStripper2>& capsStripper,
                         Period tenor,
                         Real strike);
        Real value() const;
        bool isValid() const;
        void update();
      private:
        boost::shared_ptr<CapsStripper2> capsStripper_;
        Size tenorIndex_, strikeIndex_;
    };

    inline void CapStripperQuote::update() {
        notifyObservers();
    }
    

    inline Real CapStripperQuote::value() const {
        return capsStripper_->forwardCapVolatilities()[tenorIndex_][strikeIndex_];
    }

    inline bool CapStripperQuote::isValid() const {
        return capsStripper_;
    }

    template <class ContainerT, typename valueT>
    Size checkedIndex(const ContainerT& container, 
                      const valueT& value, 
                      const std::string& name) {
        ContainerT::const_iterator match;
        match = std::find(container.begin(), container.end(), value);
        QL_REQUIRE(match!=container.end(), name << " " << value << "does not belong \
                                                             capStripper grid");
        return match-container.begin();
    }

    inline CapStripperQuote::CapStripperQuote(const boost::shared_ptr<CapsStripper2>& capsStripper,
                                              Period tenor,
                                              Real strike):capsStripper_(capsStripper) 
    {
        tenorIndex_ = checkedIndex(capsStripper->tenors(), tenor, "tenor");
        strikeIndex_ = checkedIndex(capsStripper->strikes(), strike, "strike");
        registerWith(capsStripper);
     }

    inline Rate CapsStripper2::minStrike() const {
        return strikes_.front();
    }

    inline Rate CapsStripper2::maxStrike() const {
        return strikes_.back();
    }

    inline const Matrix& CapsStripper2::syntheticCapPrices() const {
        calculate();
        return syntheticCapPrices_;
    }

    inline const Matrix& CapsStripper2::forwardCapVolatilities() const {
        calculate();
        return forwardCapsVols_;
    }
}

#endif
