
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

/*! \file basispointsensitivity.hpp
    \brief basis point sensitivity calculator
*/

// $Id$

#ifndef quantlib_bps_calculator_hpp
#define quantlib_bps_calculator_hpp

#include <ql/termstructure.hpp>
#include <ql/relinkablehandle.hpp>
#include <ql/CashFlows/coupon.hpp>

namespace QuantLib {

    namespace CashFlows {

        //! basis point sensitivity (BPS) calculator
        /*! Instances of this class accumulate the BPS of each
            cash flow they visit, returning the sum through their
            result() method.
        */
        class BPSCalculator : public Patterns::Visitor,
                              public CashFlow::Visitor,
                              public Coupon::Visitor {
          public:
            BPSCalculator(const RelinkableHandle<TermStructure>& ts) 
            : termStructure_(ts), result_(0.0) {}
            //! \name Visitor interface
            //@{
            virtual void visit(Coupon&);
            virtual void visit(CashFlow&);
            //@}
            double result() const { return result_; }
          private:
            RelinkableHandle<TermStructure> termStructure_;
            double result_;
        };


        // inline definitions

        inline void BPSCalculator::visit(Coupon& c) {
            // add BPS of the coupon
            result_ += c.accrualPeriod() *
                       c.nominal() *
                       termStructure_->discount(c.date());
        }

        inline void BPSCalculator::visit(CashFlow&) {
            // fall-back for all non-coupons; do nothing
        }
        
    }

}


#endif
