
#! /usr/bin/env python
#
# test the module used to backed up cvs repository for QuantLib
#

__doc__ = """Backup for QuantLib cvs repository"""

import urllib

if __name__=='__main__':
        #
        #
        url_name="http://cvs.sourceforge.net/cvstarballs/quantlib-cvsroot.tar.gz"
        #
        local_file_name="Y:/backed_up/Linux/cvs_bck/quantlib-cvsroot.tar.gz"
        #
        urllib.urlretrieve(url_name, local_file_name)
