"""
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
 *
 * QuantLib license is also available at:
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/
"""

""" 
    $Source$
    $Log$
    Revision 1.4  2001/02/28 11:46:13  lballabio
    Removed redundant __str__ methods - __repr__ used instead

    Revision 1.3  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.2  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.1  2001/01/08 15:12:45  nando
    added test for date and distributions

"""


from QuantLib import *
from TestUnit import TestUnit

# check su Day, Month, etc in debug: adesso il check e' solo su Month in SWIG

# ci vorrebbe un general offset su cos'e' il serial number zero

class DateTest(TestUnit):
    def doTest(self):
        mindate = Date_minDate().serialNumber()
        maxdate = Date_maxDate().serialNumber() + 1 #this one is excluded
        
        dyold  = DateFromSerialNumber(mindate-1).dayOfYear()
        dold   = DateFromSerialNumber(mindate-1).dayOfMonth()
        mold   = DateFromSerialNumber(mindate-1).monthNumber()
        yold   = DateFromSerialNumber(mindate-1).year()
        wdnold = DateFromSerialNumber(mindate-1).weekdayNumber()
        
        self.printDetails(
            "test starts at %s (%d)" %
            (str(Date_minDate()), mindate)
        )
        
        for i in range(mindate,maxdate):
            try:
                t = DateFromSerialNumber(i)
                
                # check serial number consistency
                if (t.serialNumber()!=i):
                    self.printDetails("major problem")
                dy  = t.dayOfYear()
                d   = t.dayOfMonth()
                m   = t.monthNumber()
                y   = t.year()
                mm  = t.month()
                wd  = t.weekday()
                wdn = t.weekdayNumber()
                        
                # check if skipping any date
                if (dy==dyold+1) or \
                   (dy==1 and dyold==365 and not Date_isLeap(yold)) or \
                   (dy==1 and dyold==366 and Date_isLeap(yold)):
                        dyold = dy
                else:
                        self.printDetails("dyold: %d, dy: %d" % (dyold, dy))
                        raise "wrong day of year increment"
        
                # check if skipping any date
                if (d==dold+1 and m==mold      and y==yold  ) or \
                   (d==1      and m==mold+1    and y==yold  ) or \
                   (d==1      and m==1         and y==yold+1):
                        dold = d
                        mold = m
                        yold = y
                else:
                        self.printDetails(
                          'dold: %d, d: %d, mold: %d, m: %d, yold: %d, y: %d' %
                          (dold, d, mold, m, yold, y)
                        )
                        raise "wrong day, month, year increment"
        
                # check month definition
                if (m<1 or m>12):
                    self.printDetails("m: %d" % m)
                    raise "undefined month"
        
                # check day definition
                if (d<1):
                    self.printDetails("d: %d" % d)
                    raise "day < 1"
        
                # check day definition
                if (m==1   and d>31) or \
                   (m==2   and d>29) or \
                   (m==2   and d==29 and (not Date_isLeap(y))) or \
                   (m==3   and d>31) or \
                   (m==4   and d>30) or \
                   (m==5   and d>31) or \
                   (m==6   and d>30) or \
                   (m==7   and d>31) or \
                   (m==8   and d>31) or \
                   (m==9   and d>30) or \
                   (m==10  and d>31) or \
                   (m==11  and d>30) or \
                   (m==12  and d>31):
                        self.printDetails("m: %d, d: %d" % (m,d))
                        raise "day too big for month"
        
                # check weekdayNumber definition
                if (wdn==wdnold+1 or (wdn==1 and wdnold==7)):
                    wdnold=wdn
                else:
                    self.printDetails(t)
                    self.printDetails("wdn: %d, wdnold: %d" % (wdn, wdnold))
                    raise "weekdayNumber error"
                
                # create the same date with a different constructor
                s = Date(d,m,y)
                # check serial number consistency
                if (s.serialNumber()!=i):
                    self.printDetails("date: %s" % str(t))
                    self.printDetails("serial number: %d" % i)
                    self.printDetails("new date: %s" % str(s))
                    self.printDetails("serial number: %d" % s.serialNumber())
                    raise "new date serial number different from old date"
        
                # create the same date with a different constructor
                s = Date(d,mm,y)
                # check serial number consistency
                if (s.serialNumber()!=i):
                    self.printDetails("date: %s" % str(t))
                    self.printDetails("serial number: %d" % i)
                    self.printDetails("new date: %s" % str(s))
                    self.printDetails("serial number: %d" % s.serialNumber())
                    raise "new date serial number different from old date"
            except Exception, e:
                t = DateFromSerialNumber(i)
                self.printDetails("exception thrown at %s" % str(t))
                self.printDetails("serial number: %d" % i)
                self.printDetails("day of year: %d" % t.dayOfYear())
                self.printDetails("day of month: %d" % t.dayOfMonth())
                self.printDetails("month: %s" % t.month())
                self.printDetails("year: %d" % t.year())
                raise
        
        self.printDetails(
            "test ends at %s (%d)" %
            (str(t), i)
        )



if __name__ == '__main__':
    DateTest().test('dates')

