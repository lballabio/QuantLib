
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

/*! \file scheduler.hpp
    \brief date scheduler
*/

#ifndef quantlib_scheduler_hpp
#define quantlib_scheduler_hpp

#include <ql/calendar.hpp>
#include <vector>

namespace QuantLib {

    //! %Date scheduler
    class Scheduler {
      public:
        Scheduler(const Calendar& calendar,
                  const Date& startDate, const Date& endDate,
                  int frequency, RollingConvention rollingConvention,
                  bool isAdjusted, const Date& stubDate = Date(),
                  bool startFromEnd = false, bool longFinal = false);
        // inspectors
        const Calendar& calendar() const {
            return calendar_;
        }
        const Date& startDate() const {
            return startDate_;
        }
        const Date& endDate() const {
            return endDate_;
        }
        int frequency() const {
            return frequency_;
        }
        RollingConvention const rollingConvention() const {
            return rollingConvention_;
        }
        bool isAdjusted() const {
            return isAdjusted_;
        }
        Size size() const { return dates_.size(); }
        const Date& date(int i) const;
        bool isRegular(Size i) const;
        // iterators
        typedef std::vector<Date>::const_iterator const_iterator;
        const_iterator begin() const { return dates_.begin(); }
        const_iterator end() const { return dates_.end(); }
        const Date& operator[] (Size i) const;
      private:
        Calendar calendar_;
        Date startDate_, endDate_;
        int frequency_;
        RollingConvention rollingConvention_;
        bool isAdjusted_;
        Date stubDate_;
        bool startFromEnd_;
        bool longFinal_;
        bool finalIsRegular_;
        std::vector<Date> dates_;
    };

    inline const Date& Scheduler::date(int i) const {
        QL_REQUIRE(i >= 0 && i <= int(dates_.size()),
                   "date index out of bounds");
        return dates_[i];
    }

    inline const Date& Scheduler::operator[] (Size i) const {
        #if defined(QL_DEBUG)
        QL_REQUIRE(i >= 0 && i <= int(dates_.size()),
                   "date index out of bounds");
        #endif
        return dates_[i];
    }

    class MakeScheduler {
      public:
        MakeScheduler(const Calendar& calendar,
                      const Date& startDate, const Date& endDate,
                      int frequency, RollingConvention rollingConvention,
                      bool isAdjusted)
        : calendar_(calendar), startDate_(startDate), endDate_(endDate),
          frequency_(frequency), rollingConvention_(rollingConvention),
          isAdjusted_(isAdjusted),
          stubDate_(Date()), startFromEnd_(false), longFinal_(false) {}
        
        MakeScheduler& withStubDate(const Date& d) {
            stubDate_ = d; 
            return *this;
        }
        MakeScheduler& backwards(bool flag=true) {
            startFromEnd_ = flag; 
            return *this;
        }
        MakeScheduler& forwards(bool flag=true) {
            startFromEnd_ = !flag; 
            return *this;
        }
        MakeScheduler& longFinalPeriod(bool flag=true) {
            longFinal_ = flag; 
            return *this;
        }
        MakeScheduler& shortFinalPeriod(bool flag=true) {
            longFinal_ = !flag; 
            return *this;
        }
        operator Scheduler() {
            return Scheduler(calendar_,startDate_,endDate_,frequency_,
                             rollingConvention_,isAdjusted_,stubDate_,
                             startFromEnd_,longFinal_);
        }
      private:
        Calendar calendar_;
        Date startDate_, endDate_;
        int frequency_;
        RollingConvention rollingConvention_;
        bool isAdjusted_;
        Date stubDate_;
        bool startFromEnd_;
        bool longFinal_;
    };
    
}


#endif
