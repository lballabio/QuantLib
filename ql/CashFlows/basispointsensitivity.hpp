
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

#ifndef quantlib_bps_calculator_hpp
#define quantlib_bps_calculator_hpp

#include <ql/termstructure.hpp>
#include <ql/relinkablehandle.hpp>
#include <ql/null.hpp>
#include <ql/CashFlows/simplecashflow.hpp>
#include <ql/CashFlows/coupon.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>

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
        }
        
        class TimeBasket {
	   public:
	     class Entry {
	        public:
		Entry() {
		   date_ = Date();
		   value_ = 0.0;
		}
		Entry(const Date& date, const double value)
		   : date_(date), value_(value) {}
		const Date& date() const { return date_; }
		double value() const { return value_; }
		void value(double value) const { value_ = value; }
		Entry& operator+=(double value) {
		   value_ += value;
		   return *this;
		}
		Entry& operator+=(const Entry& entry) {
		   value_ += entry.value();
		   return *this;
		}
		Entry& operator-=(double value) {
		   value_ -= value;
		   return *this;
		}
		Entry& operator-=(const Entry& entry) {
		   value_ -= entry.value();
		   return *this;
		}
		bool operator==(const Entry& e1) const {
		   return e1.date() == date_;
		}
		class equal : public std::unary_function<Entry,bool> {
		public:
		   explicit equal(const Date& date)
		      : date_(date) {}
		   bool operator()(const Entry& entry) const {
		      return entry.date() == date_;
		   }
		private:
		   const Date date_;
		};
		class less_equal : public std::unary_function<Entry,bool> {
		public:
		   explicit less_equal(const Date& date)
		      : date_(date) {}
		   bool operator()(const Entry& entry) const {
		      return entry.date() <= date_;
		   }
		private:
		   const Date date_;
		};
		class greater_equal : public std::unary_function<Entry,bool> {
		public:
		   explicit greater_equal(const Date& date)
		      : date_(date) {}
		   bool operator()(const Entry& entry) const {
		      return entry.date() >= date_;
		   }
		private:
		   const Date date_;
		};
		class ascending {
		public:
		   int operator()(const Entry& e1,
				  const Entry& e2) const {
		      return e1.date() < e2.date();
		   }
		};
		class descending {
		public:
		   int operator()(const Entry& e1,
				  const Entry& e2) const {
		      return e1.date() > e2.date();
		   }
		};
		
	     private:
		Date date_;
		mutable double value_;
	     };
	   /*! Default constructor */
	   TimeBasket() {}
	   TimeBasket(const std::vector<Date>& dates,
		      const std::vector<double>& values);
	   TimeBasket(const Handle<TimeBasket>& original,
		      const std::vector<Date>& buckets);
	   //! \name Inspectors
	   //@{
	   //! returns the number of timeValue entries
	   Size size() const { return entries_.size(); }
	   //@}
	   //! \name TimeBasket data access
	   //@{
	   //! add new value entry
	   void push(const Entry&) const;
	   void push(const Date&, double) const;
	   void add(const Entry&) const;
	   void add(const Date&, double) const;
	   void subtract(const Entry&) const;
	   void subtract(const Date&, double) const;
	   Entry operator[](int i) const;
	   Entry operator[](const Date&) const;
	   TimeBasket& operator+=(const TimeBasket& other);
	   TimeBasket& operator-=(const TimeBasket& other);
	   //@}
	   //! single datum in TimeBasket
	   private:
	   mutable std::vector<Entry> entries_;
	};
       
        class BPSBasketCalculator : public Patterns::Visitor,
				    public CashFlow::Visitor,
				    public Coupon::Visitor,
				    public FixedRateCoupon::Visitor {
          public:
            BPSBasketCalculator(const RelinkableHandle<TermStructure>& ts,
				int basis=2)
	       : termStructure_(ts), basis_(basis) {
	       result_ = Handle<TimeBasket>(new TimeBasket());
	    }
            //! \name Visitor interface
            //@{
	    double sensfactor(const Date& date) const;
            virtual void visit(Coupon&);
            virtual void visit(FixedRateCoupon&);
            virtual void visit(CashFlow&);
            //@}
	    Handle<TimeBasket> result() const { return result_; }

          private:
            RelinkableHandle<TermStructure> termStructure_;
	    int basis_;
	    Handle<TimeBasket> result_;
        };


        // inline definitions

        inline double BPSBasketCalculator::sensfactor(const Date& date) const {
	   Time t = termStructure_->dayCounter().yearFraction(
	      termStructure_->referenceDate(),date);
	   // Based on 1st derivative of zero coupon rate
	   Rate r = termStructure_->zeroCoupon(date,basis_);
	   return -QL_POW(1.0+r/basis_,-1.0-t*basis_)*t;
	}
       
        inline void BPSBasketCalculator::visit(Coupon& c) {
	    Date today = termStructure_->todaysDate(),
	       accrualStart = c.accrualStartDate(),
	       accrualEnd = c.accrualEndDate(),
	       payment = c.date();
	    if (accrualStart > today) {
	       double bps = sensfactor(accrualStart);
	       result_->add(accrualStart,bps*c.nominal()/10000.0);
	    }
	    if (accrualEnd >= today) {
	       double bps = -sensfactor(accrualEnd);
	       DiscountFactor dfs = 1.0, dfe = 1.0;
	       if (accrualStart > today)
		  dfs = termStructure_->discount(accrualStart);
	       dfe = termStructure_->discount(accrualEnd);
	       result_->add(accrualEnd,bps*c.nominal()*(dfs/dfe)/10000.0);
	    }
	    if (payment > today) {
	       double bps = sensfactor(payment);
	       result_->add(payment,bps*c.amount()/10000.0);
	    }
        }

        inline void BPSBasketCalculator::visit(FixedRateCoupon& c) {
	    Date today = termStructure_->todaysDate(),
	       payment = c.date();
	    if (payment > today) {
	       double bps = sensfactor(payment);
	       result_->subtract(payment,bps*c.amount()/10000.0);
	    }
        }

        inline void BPSBasketCalculator::visit(CashFlow&) {
            // fall-back for all non-coupons; do nothing
        }
        
        inline TimeBasket::TimeBasket(const std::vector<Date>& dates,
				      const std::vector<double>& values) {
	   QL_REQUIRE(dates.size() == values.size(),
		      "TimeBasket parameter size descrepancy");
	   for (Size i = 0; i < dates.size(); i++)
	      entries_.push_back(Entry(dates[i],values[i]));
	   std::sort(entries_.begin(),
		     entries_.end(),
		     Entry::ascending());
	}
      
        inline TimeBasket::TimeBasket(const Handle<TimeBasket>& original,
				      const std::vector<Date>& buckets) {
	   QL_REQUIRE(buckets.size() > 0,
		      "Empty bucket structure");
	   std::vector<Date> sbuckets = buckets;
	   std::sort(sbuckets.begin(),
		     sbuckets.end(),
		     Date::descending());
	   Size i;
	   for (i = 0; i < sbuckets.size(); i++) {
	      push(sbuckets[i],0.0);
	   }
	   for (i = 0; i < original->size(); i++) {
	      Entry entry = (*original)[i];
	      Date pDate = Null<Date>(), nDate = Null<Date>();
	      const std::vector<Date>::const_iterator bi =
		 std::find_if(sbuckets.begin(),
			      sbuckets.end(),
			      Date::less_equal(entry.date()));
	      if (bi == sbuckets.end())
		 pDate = sbuckets.back();
	      else {
		 pDate = *bi;
		 if (pDate != sbuckets[0])
		    nDate = *(bi-1);
	      }
	      double value = entry.value();
	      if (pDate == entry.date())
		 add(pDate,value);
	      else if (nDate != Null<Date>()) {
		 double pDays = (double)(entry.date()-pDate);
		 double nDays = (double)(nDate-entry.date());
		 double tDays = (double)(nDate-pDate);
		 add(pDate,value*(nDays/tDays));
		 add(nDate,value*(pDays/tDays));
	      } else
		 add(pDate,value);
	   }
	}

       inline TimeBasket& TimeBasket::operator+=(const TimeBasket& other) {
	  Size i;
	  for (i = 0; i < other.size(); i++) {
	     add(other[i]);
	  }
	  return *this;
	}

       inline TimeBasket& TimeBasket::operator-=(const TimeBasket& other) {
	  Size i;
	  for (i = 0; i < other.size(); i++) {
	     subtract(other[i]);
	  }
	  return *this;
	}

        inline void TimeBasket::push(const Entry& entry) const {
	   if (entry == Entry())
	      return;
	   std::vector<Entry>::iterator ei =
	      std::find_if(entries_.begin(),
			   entries_.end(),
			   Entry::greater_equal(entry.date()));
	   if (ei != entries_.end() && ei->date() == entry.date()) {
	      ei->value(entry.value());
	   }
	   else {
	      entries_.insert(ei, entry);
	   }
	}
      
        inline void TimeBasket::push(const Date& date,
				     double value) const {
	   push(Entry(date,value));
	}
      
        inline void TimeBasket::add(const Entry& entry) const {
	   if (entry == Entry())
	      return;
	   std::vector<Entry>::iterator ei =
	      std::find_if(entries_.begin(),
			   entries_.end(),
			   Entry::greater_equal(entry.date()));
	   if (ei != entries_.end() && ei->date() == entry.date()) {
	      *ei += entry;
	   } else {
	      entries_.insert(ei, entry);
	   }
	}
      
        inline void TimeBasket::add(const Date& date,
				    double value) const {
	   add(Entry(date,value));
	}
      
        inline void TimeBasket::subtract(const Entry& entry) const {
	   if (entry == Entry())
	      return;
	   std::vector<Entry>::iterator ei =
	      std::find_if(entries_.begin(),
			   entries_.end(),
			   Entry::greater_equal(entry.date()));
	   if (ei != entries_.end() && ei->date() == entry.date()) {
	      *ei -= entry;
	   } else {
	      entries_.insert(ei, Entry(entry.date(),-entry.value()));
	   }
	}
      
        inline void TimeBasket::subtract(const Date& date,
					 double value) const {
	   subtract(Entry(date,value));
	}
      
        inline TimeBasket::Entry TimeBasket::operator[](int i) const {
	   if (i >= 0 && i < entries_.size())
	      return entries_[i];
	   return Entry();
	}
      
        inline TimeBasket::Entry TimeBasket::operator[](const Date& date) const {
	   std::vector<Entry>::iterator ei =
	      std::find_if(entries_.begin(),
			   entries_.end(),
			   Entry::equal(date));
	   if (ei == entries_.end())
	      return Entry();
	   return *ei;
	}
       
    }

}


#endif
