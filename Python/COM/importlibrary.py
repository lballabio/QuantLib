"""
/*
 * Copyright (C) 2001
 * Marco Marchioro, Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin
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
    \file importlibrary.py
    \brief Export modules as COM objects
    
    $Source$
    $Log$
    Revision 1.5  2001/03/30 09:56:20  marmar
    Debug is now the default, simplified and speeded up

    Revision 1.4  2001/03/14 13:36:20  marmar
    Changed to accomodate some bad behaviour of VB
    
    Revision 1.1  2001/03/06 13:52:35  marmar
    Export the QuantLib functionalities to the COM world
    
"""

"""
Example:
    ' The following is an example of VB code that creates the
    ' QuantLib module and uses it

    ' The object ImportLibrary must be initialized before any python 
    ' module can be created. Tip: define QuantLib as Public
    Set QuantLib = CreateObject("QuantLib.Import")
    
    ' Create a QuantLib object
    Set uniformDeviate = QuantLib.UniformRandomGenerator()
    MsgBox uniformDeviate.next()

COM Exceptions raised:
    #5, ERROR_ACCESS_DENIED if import is called with
                    DISPATCH_PROPERTYGET or DISPATCH_METHOD
    #2, ERROR_FILE_NOT_FOUND if import of required python module failed
    #12, ERROR_INVALID_ACCESS if problems arise in applying method to object
All the other errors are not transformed into COM exception but presented as 
Unknown python errors.
"""

import pywintypes, string, types, winerror, exceptions
import win32com.client, win32com.server.util, win32com.server.policy
from win32com.server.dispatcher import DefaultDebugDispatcher
from win32com.server.exception import COMException
from win32com.server.policy import BasicWrapPolicy
from win32com.server.policy import DynamicPolicy
from pythoncom import *

def FixArguments(args):
    """Unicode objects are converted to strings, PyIDispatch objects and
    collections are unwrapped. Does this recursively on lists
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
    Also, if 'Count' is encountered it is transformed into '__len__'
    """
    if name == 'Count' :
        return '__len__'
    else:
        return string.lower(name[0]) + name[1:]

def PrepareForReturn(object):
    objectType = type(object)
    print "----> Returning", object, objectType
    if(objectType is types.InstanceType or objectType is types.ModuleType):
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
        try:
          name = str(self._dyn_dispid_to_name_[dispid])
        except KeyError:
          raise COMException(scode = winerror.DISP_E_MEMBERNOTFOUND,
                             desc="Member not found")
                 
        args = FixArguments(args)
        if not hasattr(self._obj_, name):
            name = FixVisualBasicName(name)

        print "self._obj_:",self._obj_
        print "name:",name
        print "args:",args

        if wFlags & DISPATCH_METHOD:
            if name == '_value_':
                # VB calls to COM of the type "PyCOM_object(args)" are mapped
                # into calls to the method "_value_".
                # When 'self._obj_' has the attribute '__call__' then it
                #is returned 'self._obj_(args)'
                if hasattr(self._obj_, '__call__'):
                    return PrepareForReturn(apply(self._obj_,args))
                else:
                    raise COMException(
                    "Cannot compute '%s %s'" % (repr(self._obj_), args),
                    winerror.ERROR_INVALID_ACCESS)
               
            #Next we evaluate calls of the type "PyCOM_object.name(args)"
            try:
                qlMethod = getattr(self._obj_, name)
            except AttributeError:
                raise COMException(
                    "Attribute '%s' not found for object '%s'" % (
                    name, repr(self._obj_)), winerror.ERROR_INVALID_ACCESS)
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
            if len(args) == 1:
                setattr(self._obj_, name, args)
            else:
                raise COMException(
                    "DISPATCH_PROPERTYPUT called with too many arguments",
                    winerror.E_INVALIDARG)
        raise COMException(scode=winerror.E_INVALIDARG, desc="invalid wFlags")

class ComImportLibrary:
    """ Creates a COM wrapper for QuantLib.
    """
    _reg_clsid_ = "{389276A1-24EB-11D5-83D4-0050DA367EDA}"
    _reg_progid_ = "QuantLib.Import"
    _public_methods_ = ['QuantLib']
    def QuantLib(self):
        print "ComImportLibrary called"
        try :
            import QuantLib
            return PrepareForReturn(QuantLib)
        except:
            raise COMException("Module QuantLib not found",
                               winerror.ERROR_FILE_NOT_FOUND)

if __name__ == '__main__':
    import win32com.server.register
    win32com.server.register.UseCommandLine(ComImportLibrary)
