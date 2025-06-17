# Copyright 2019 NXP
#
# NXP Confidential. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

import csv
import sys
from tlvAPIs import TLVAPDU

COPYRIGHT = """/* Copyright 2019,2021 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */
"""

class k(object):
    Name = "Name"
    Description = 'Description'
    CLA = "CLA"
    INS = "INS"
    P1 = "P1"
    P2 = "P2"
    Lc = "Lc"
    CMD_TAG = 'T:C'
    CMD_TAG_Desc = 'T:Desc'
    LeCase = "LeCase"
    RSP_TAG = "T:R"
    ParamType = "ParamType"
    AppletVersion = "applet version"


class CSV2TLV(object):

    def __init__(self, csv_dict, o_header_file, o_c_file):
        self.csv_dict = csv_dict
        self.current_tag = None
        self.o_header_file = o_header_file
        self.o_c_file = o_c_file

    def _processCurrentTag(self):
        #print(self.current_tag)
        self.o_header_file.write(self.current_tag._comment_string());
        self.o_header_file.write(self.current_tag._c_APISignature());
        self.o_header_file.write(";\n\n");
        self.o_c_file.write("/* %s */\n" % (self.current_tag.appletVersion, ))
        self.o_c_file.write(self.current_tag._c_APISignature());
        self.o_c_file.write("\n{\n");
        self.o_c_file.write(self.current_tag._c_implementation());
        self.o_c_file.write("\n}\n\n");
        self.current_tag = None

    def Run(self):
        self._getTags()
        print ("Done!")

    def _getTags(self):
        for e in self.csv_dict:
            Name = e[k.Name]
            if Name:
                if self.current_tag:
                    self._processCurrentTag()
                self.current_tag = TLVAPDU(Name)
                self.current_tag.CLA(e[k.CLA]) \
                    .Description(e[k.Description]) \
                    .INS(e[k.INS]) \
                    .P1(e[k.P1]) \
                    .P2(e[k.P2]) \
                    .Lc(e[k.Lc]) \
                    .LeCase(e[k.LeCase]) \
                    .AppletVersion(e[k.AppletVersion]) \

                self.cmd_index = 0
                self.rsp_index = 0
            if e[k.CMD_TAG] and e[k.ParamType]:
                self.current_tag.Payload(
                    self.cmd_index, e[k.CMD_TAG], e[k.CMD_TAG_Desc], e[k.ParamType])
                self.cmd_index += 1
            if e[k.RSP_TAG] and e[k.ParamType]:
                self.current_tag.Response(
                    self.rsp_index, e[k.RSP_TAG], e[k.CMD_TAG_Desc], e[k.ParamType])
                self.rsp_index += 1
        # Last Entry
        if self.current_tag:
            self._processCurrentTag()

se05x_APDU_H_HEADER = """

#ifndef SE050X_APDU_H_INC
#define SE050X_APDU_H_INC

#include "se05x_tlv.h"

"""
se05x_APDU_H_FOOTER = """
#endif /* SE050X_APDU_H_INC */
"""

se05x_APDU_C_HEADER = """

/* ********************************************************* */
/* ** Auto Generated *************************************** */
/* ********************************************************* */

"""
se05x_APDU_C_FOOTER = """
#endif /* SE050X_APDU_H_INC */
"""

def doRun(n_csv_file, apdu_decl, apdu_impl):
    o_csv_file = open(n_csv_file)
    o_header_file = open(apdu_decl, "w")
    o_header_file.write(COPYRIGHT)
    o_header_file.write(se05x_APDU_H_HEADER)
    o_c_file = open(apdu_impl, "w")
    o_c_file.write(COPYRIGHT)
    o_csv_dict = csv.DictReader(o_csv_file)
    csv2tlv = CSV2TLV(o_csv_dict, o_header_file, o_c_file)
    csv2tlv.Run()
    o_csv_file.close()
    o_header_file.write(se05x_APDU_H_FOOTER)
    o_header_file.close()
    o_c_file.close()

def main():
    try:
        (n_csv_file, apdu_decl, apdu_impl) = \
            (sys.argv[1], sys.argv[2], sys.argv[3])
    except IndexError:
        usage()
        return
    doRun(n_csv_file, apdu_decl, apdu_impl)

def usage():
    print("Usage: %s <n_csv_file> <apdu_decl> <apdu_impl>"%(sys.argv[0]))

if __name__ == '__main__':
    main()
