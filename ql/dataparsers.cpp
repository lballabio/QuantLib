
/*
 Copyright (C) 2000, 2001, 2002, 2003 Andre Louw

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file dataparsers.cpp
    \brief classes used to parse data for input
*/

#include <ql/dataparsers.hpp>
#include <ql/dataformatters.hpp>
#include <ql/null.hpp>

namespace QuantLib {

    Period PeriodParser::parse(const std::string& str) {
        TimeUnit units = Days;

        QL_REQUIRE(str.length()>1, "Argument needs length of at least 2");
        Size iPos = str.find_first_of("DdWwMmYy");
        if (iPos != str.length()-1)
            QL_FAIL("Unknown units, input: '"+str+"'");
        char abbr = QL_TOUPPER(str[iPos]);
        if (abbr == 'D')      units = Days;
        else if (abbr == 'W') units = Weeks;
        else if (abbr == 'M') units = Months;
        else if (abbr == 'Y') units = Years;
        return Period(QL_ATOI(str.c_str()), units);
    }

    std::vector<std::string> DateParser::split(const std::string& str,
					       const char delim) {
       std::vector<std::string> list;
       Size sx= str.find(delim), so=0;

       while (sx != std::string::npos) {
	  list.push_back(str.substr(so,sx));
	  so += sx+1;
	  sx = str.substr(so).find(delim);
       }
       list.push_back(str.substr(so));
       return list;
    }
   
    Date DateParser::parse(const std::string& str, const std::string& fmt) {
       std::vector<std::string> slist;
       std::vector<std::string> flist;
       int d=0, m=0, y=0;

       slist = split(str,'/');
       flist = split(fmt,'/');
       if (slist.size() != flist.size())
	  return Null<Date>();
       Size i;
       for (i=0;i<flist.size();i++) {
	  std::string sub = flist[i];
	  if (StringFormatter::toLowercase(sub) == "dd")
	     d = QL_ATOI(slist[i].c_str());
	  else if (StringFormatter::toLowercase(sub) == "mm")
	     m = QL_ATOI(slist[i].c_str());
	  else if (StringFormatter::toLowercase(sub) == "yyyy") {
	     y = QL_ATOI(slist[i].c_str());
	     if (y < 100)
		y += 2000;
	  }
       }
       return Date(d,(Month)m,y);
    }
   
}
