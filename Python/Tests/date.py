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
    Revision 1.5  2001/04/04 11:08:11  lballabio
    Python tests implemented on top of PyUnit

    Revision 1.4  2001/02/28 11:46:13  lballabio
    Removed redundant __str__ methods - __repr__ used instead

    Revision 1.3  2001/02/22 14:27:26  lballabio
    Implemented new test framework

    Revision 1.2  2001/01/08 16:19:29  nando
    more homogeneous format

    Revision 1.1  2001/01/08 15:12:45  nando
    added test for date and distributions

"""

import QuantLib
import unittest

# check su Day, Month, etc in debug: adesso il check e' solo su Month in SWIG

# ci vorrebbe un general offset su cos'e' il serial number zero

class DateTest(unittest.TestCase):
    def runTest(self):
        "Testing dates"
        mindate = QuantLib.Date_minDate().serialNumber()
        maxdate = QuantLib.Date_maxDate().serialNumber() + 1 #excluded
        
        dyold  = QuantLib.DateFromSerialNumber(mindate-1).dayOfYear()
        dold   = QuantLib.DateFromSerialNumber(mindate-1).dayOfMonth()
        mold   = QuantLib.DateFromSerialNumber(mindate-1).monthNumber()
        yold   = QuantLib.DateFromSerialNumber(mindate-1).year()
        wdnold = QuantLib.DateFromSerialNumber(mindate-1).weekdayNumber()
        
        for i in range(mindate,maxdate):
            t = QuantLib.DateFromSerialNumber(i)
            
            # check serial number consistency
            assert t.serialNumber() == i, \
                "inconsistent serial number:\n" + \
                "original:      %d\n" % i + \
                "date:          %s\n" % t + \
                "serial number: %d\n" % t.serialNumber()
            
            dy  = t.dayOfYear()
            d   = t.dayOfMonth()
            m   = t.monthNumber()
            y   = t.year()
            mm  = t.month()
            wd  = t.weekday()
            wdn = t.weekdayNumber()
            
            # check if skipping any date
            assert (dy==dyold+1) or \
                   (dy==1 and dyold==365 \
                    and not QuantLib.Date_isLeap(yold)) or \
                   (dy==1 and dyold==366 and QuantLib.Date_isLeap(yold)), \
                   "wrong day of year increment:\n" + \
                   "date: %s\n" % t + \
                   "day of year: %d\n" % dy + \
                   "previous:    %d\n" % dyold
            dyold = dy
            
            # check if skipping any date
            assert (d==dold+1 and m==mold      and y==yold  ) or \
                   (d==1      and m==mold+1    and y==yold  ) or \
                   (d==1      and m==1         and y==yold+1), \
                   "wrong day, month, year increment\n" + \
                   "date: %s\n" % t + \
                   "day, month, year: %d, %d, %d\n" % (d,m,y) + \
                   "previous:         %d, %d, %d\n" % (dold,mold,yold)
            dold = d
            mold = m
            yold = y
            
            # check month definition
            assert (m>=1 and  m<=12), \
                "invalid month\n" + \
                "date: %s\n" % t + \
                "month: %d\n" % m
            
            # check day definition
            assert d >= 1, \
               "invalid day of month\n" + \
               "date: %s\n" % t + \
               "day: %d\n" % d
            
            # check day definition
            assert (m==1   and d<=31) or \
                   (m==2   and d<=28) or \
                   (m==2   and d==29 and QuantLib.Date_isLeap(y)) or \
                   (m==3   and d<=31) or \
                   (m==4   and d<=30) or \
                   (m==5   and d<=31) or \
                   (m==6   and d<=30) or \
                   (m==7   and d<=31) or \
                   (m==8   and d<=31) or \
                   (m==9   and d<=30) or \
                   (m==10  and d<=31) or \
                   (m==11  and d<=30) or \
                   (m==12  and d<=31), \
                   "invalid day of month\n" + \
                   "date: %s\n" % t + \
                   "day: %d\n" % d + \
                   "month: %s\n" % mm
            
            # check weekdayNumber definition
            assert (wdn==wdnold+1 or (wdn==1 and wdnold==7)), \
               "wrong weekday number increment\n" + \
               "date: %s\n" % t + \
               "weekday number: %d\n" % wdn + \
               "previous:       %d\n" % wdnold
            wdnold=wdn
            
            # create the same date with a different constructor
            s = QuantLib.Date(d,m,y)
            # check serial number consistency
            assert s.serialNumber()==i, \
                "inconsistent serial number\n" + \
                "date: %s\n" % t + \
                "serial number: %d\n" % i + \
                "cloned date: %s\n" % s + \
                "serial number: %d\n" % s.serialNumber()
            
            # create the same date with a different constructor
            s = QuantLib.Date(d,mm,y)
            # check serial number consistency
            assert s.serialNumber()==i, \
                "inconsistent serial number\n" + \
                "date: %s\n" % t + \
                "serial number: %d\n" % i + \
                "cloned date: %s\n" % s + \
                "serial number: %d\n" % s.serialNumber()


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(DateTest())
    unittest.TextTestRunner().run(suite)


