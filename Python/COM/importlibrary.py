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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

"""
"""
    \file importlibrary.py
    \brief Export modules as COM objects
    
    $Source$
    $Log$
    Revision 1.7  2001/05/15 13:03:06  marmar
    Wrappers simplified, copyright changed, exceptions removed

    Revision 1.6  2001/03/30 09:59:33  marmar
    Documentation updated, Matrix call did not work properly: removed

    Revision 1.5  2001/03/30 09:56:20  marmar
    Debug is now the default, simplified and speeded up

    Revision 1.4  2001/03/14 13:36:20  marmar
    Changed to accomodate some bad behaviour of VB
"""

import pywintypes, string, types, winerror, exceptions
import win32com.client, win32com.server.util, win32com.server.policy
from win32com.server.dispatcher import DefaultDebugDispatcher
from win32com.server.policy import BasicWrapPolicy
from win32com.server.policy import DynamicPolicy
from pythoncom import *

def FixArguments(args):
    """Unicode objects are converted to strings, PyIDispatch objects and
    collections are unwrapped.
    """
    newArgs = []
    for arg in args:
        if type(arg) is types.TupleType:
            arg = FixArguments(arg)
        elif type(arg) is pywintypes.UnicodeType:
            arg = str(arg)
        newArgs.append(arg)
    return tuple(newArgs)

def FixVisualBasicName(name):
    """Unfortunately, there are a number of names that are difficult
    to type from VB.  E.g. if you type 'value' it will be converted into
    'Value', 'next' into 'Next' and so on.  This function tries to fix 
    this behaviour.
    """
    return string.lower(name[0]) + name[1:]

def PrepareForReturn(object):
    objectType = type(object)
    print "----> Returning", object, objectType
    if(objectType is types.InstanceType):
        print "Returning wrapped object"
        return WrapObject(
                    DefaultDebugDispatcher(
                            QuitePermissivePolicy, object))
    else:
        print "Returning simple object"
        return object


class QuitePermissivePolicy(DynamicPolicy):
    """A policy specialized to work with python objects.

    Note: here and there VB calls a COM object with flag DISPATCH_METHOD &
    DISPATCH_PROPERTYGET. in order to speed up the call to the python side
    this program interpreters this as a call with flag DISPATCH_METHOD. 
    Therefore, sometimes calls using DISPATCH_PROPERTYGET end up raising an
    unexpected python error.  The advise is anyway to wrap any attribute call
    in a getAttribute method.
    """
    def _wrap_(self, object):
        BasicWrapPolicy._wrap_(self, object)
        self._next_dynamic_ = self._min_dynamic_ = 1000
        self._dyn_dispid_to_name_ = {DISPID_VALUE:'_value_',
                                     DISPID_NEWENUM:'_NewEnum' }
     
    def _invokeex_(self, dispid, lcid, wFlags, args, kwargs, serviceProvider):
        name = str(self._dyn_dispid_to_name_[dispid])
                 
        args = FixArguments(args)
        if not hasattr(self._obj_, name):
            name = FixVisualBasicName(name)
        # Printing debug info
        print "self._obj_:",self._obj_
        print "name:",name
        print "args:",args
        if wFlags & DISPATCH_METHOD:
            #Next we evaluate calls of the type "PyCOM_object.name(args)"
            qlMethod = getattr(self._obj_, name)
            initializedObject = apply(qlMethod, args)
            return PrepareForReturn(initializedObject)
         
        if wFlags & DISPATCH_PROPERTYGET:
            # Here we handle the calls of the type "VB_value = PyCOM_object"
            # in the rare case in which VB uses DISPATCH_PROPERTYGET
            # sometimes these are dispatched as DISPATCH_METHOD in which
            # case an error message will be raised
            initializedObject = self._obj_.__dict__[name]
            return PrepareForReturn(initializedObject)
            
        if wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF):
            # Calls of the type "PyCOM_object = VB_value" are 
            setattr(self._obj_, name, args)

class ComImportLibrary:
    _reg_clsid_ = "{389276A1-24EB-11D5-83D4-0050DA367EDA}"
    _reg_progid_ = "QuantLib.Import"
    _public_methods_ = ['QuantLib']
    def QuantLib(self):
        print "ComImportLibrary called"
        import QuantLib
        return WrapObject(DefaultDebugDispatcher(QuitePermissivePolicy, QuantLib))

if __name__ == '__main__':
    import win32com.server.register
    win32com.server.register.UseCommandLine(ComImportLibrary)
    
