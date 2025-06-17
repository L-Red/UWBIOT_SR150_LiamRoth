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

SUPPORTED_PARAM_TYPES = {
    "U8" : "uint8_t",
    "u8" : "uint8_t",
    "U16" : "uint16_t",
    "U32" : "uint32_t",
    #"ePolicy",
    #"eCurveID",
    "u8buf" : "u8buf",
    "Se05xSession" : "pSe05xSession_t",
    "Se05xPolicy" : "pSe05xPolicy_t",
    # "void *",
}


class TLVAPDU(object):

    def __init__(self, name):
        self.name = name
        self.payload = {}
        self.response = {}
        self._c_implementation = self._c_implementationV2

    def Description(self, description):
        self.description = description
        return self

    def CLA(self, CLA):
        self.cla = int(CLA, 16)
        return self

    def INS(self, INS):
        self.ins = int(INS, 16)
        return self

    def P1(self, P1):
        self.p1 = int(P1, 16)
        return self

    def P2(self, P2):
        self.p2 = int(P2, 16)
        return self

    def Lc(self, Lc):
        self.lc = Lc
        return self

    def LeCase(self, leCase):
        if (leCase):
            self.LeCase = int(leCase)
        else:
            self.LeCase = -1
        return self

    def AppletVersion(self, AppletVersion):
        self.appletVersion = AppletVersion
        return self

    def apiParam(self, api_param):
        if ":" not in api_param:
            print("Bad api_param='%s'. Must be 'Name:Type'" % (api_param,))
            assert(":" in api_param)
        (param_name, param_type) = api_param.split(":")
        param_name = param_name.strip()
        param_type = param_type.strip()
        if param_type  not in SUPPORTED_PARAM_TYPES:
            print("Bad param_type='%s'" % (param_type,))
            assert(param_type  in SUPPORTED_PARAM_TYPES)
        return(param_name, param_type)

    def Payload(self, index, TAG, description, APIParam):
        assert(0 != len(TAG))
        assert(0 != len(APIParam))
        self.payload[index] = (TAG.strip(), description.strip(), self.apiParam(APIParam))
        return self

    def Response(self, index, TAG, description, APIParam):
        assert(0 != len(TAG))
        assert(0 != len(APIParam))
        self.response[index] = (TAG.strip(), description.strip(), self.apiParam(APIParam))
        return self

    def _pretty_dump(self):
        ret_string = []
        ret_string.append("%s : %s @%s" % (self.name, self.description, self.appletVersion))
        ret_string.append("CLA=0x%02X INS=0x%02X P1=0x%02X P2=0x%02X" % (
            self.cla, self.ins, self.p1, self.p2))
        for i in range(len(self.payload)):
            ret_string.append("CMD[%d]\tTAG=%-15s Desc='%s'\n\t\tSSCP=%s" % (
                i, self.payload[i][0], self.payload[i][1], self.payload[i][2]))
        for i in range(len(self.response)):
            ret_string.append("RSP[%d]\tTAG=%-15s Desc='%s'\n\t\tSSCP=%s" % (
                i, self.response[i][0], self.response[i][1], self.response[i][2]))
        ret_string.append("")
        return "\n\t".join(ret_string)

    def _c_FnName(self):
        return ("Se05x_API_%s" % (self.name))

    def _c_APISignature(self):
        ret_string = []
        ret_string.append("smStatus_t %s(" % (self._c_FnName(),))
        for (i, (tag, description, (param_name, param_type))) \
            in list(self.payload.items()):
            if "u8buf" == param_type:
                ret_string.append("    const uint8_t *%s," % (param_name))
                ret_string.append("    size_t %sLen," % (param_name,))
            else:
                ret_string.append("    %s %s," % (  SUPPORTED_PARAM_TYPES[param_type], param_name))
        for (i, (tag, description, (param_name, param_type))) \
            in list(self.response.items()):
            if "u8buf" == param_type:
                ret_string.append("    uint8_t *%s," % (param_name))
                ret_string.append("    size_t *p%sLen," % (param_name))
            else:
                ret_string.append("    %s *p%s," % (SUPPORTED_PARAM_TYPES[param_type], param_name))
        if ret_string[-1][-1] == ",":
            # remove last comma and put a closing bracket
            ret_string[-1] = ret_string[-1][:-1] + ")"
        return "\n".join(ret_string)

    def _comment_string(self):
        ret_string = []
        ret_string.append("/** %s" % (self._c_FnName(),))
        ret_string.append(" *")
        ret_string.append(" * %s" % (self.description,))
        ret_string.append(" *")
        for (i, (tag, description, (param_name, param_type))) \
            in list(self.payload.items()):
            ret_string.append(" * @param %s[in] %s [%d:%s]" % (
                param_name, description, i, tag))
            if "u8buf" == param_type:
                ret_string.append(" * @param %sLen[in] Length of %s" % (
                    param_name, param_name))
        for (i, (tag, description, (param_name, param_type))) \
            in list(self.response.items()):
            ret_string.append(" * @param %s[out] %s [%d:%s]" % (
                param_name, description, i, tag))
            if "u8buf" == param_type:
                ret_string.append(" * @param p%sLen[in,out] Length for %s" % (
                    param_name, param_name))
        ret_string.append(" */")
        ret_string.append("")
        return "\n".join(ret_string)

    def _c_implementationV2(self):
        ret_string = []
        ret_string.append("")
        ret_string.append("smStatus_t retStatus = SM_NOT_OK;")
        ret_string.append("const tlvHeader_t hdr = {{0x%02X, 0x%02X, 0x%02X, 0x%02X}};" % (
            self.cla, self.ins, self.p1, self.p2))
        ret_string.append("uint8_t cmdbuf[SE05X_MAX_BUF_SIZE_CMD];")
        ret_string.append("uint8_t *pCmdbuf = &cmdbuf[0];");
        ret_string.append("size_t cmdbufLen = 0;")
        ret_string.append("int tlvRet = 0;")
        if len(self.response):
            ret_string.append("uint8_t rspbuf[SE05X_MAX_BUF_SIZE_RSP];")
            ret_string.append("uint8_t *pRspbuf = &rspbuf[0];")
            ret_string.append("size_t rspbufLen = ARRAY_SIZE(rspbuf);")
        ret_string.append("#if VERBOSE_APDU_LOGS")
        ret_string.append('printf("\\r\\n");')
        ret_string.append('nLog("APDU", NX_LEVEL_DEBUG, "%s [%s]");'%(
        self.name,  self.description));

        ret_string.append("#endif /* VERBOSE_APDU_LOGS */")

        for (_, (tag, description, (param_name, param_type))) \
            in list(self.payload.items()):
            if "u8buf" == param_type:
                ret_string.append("tlvRet = TLVSET_%s(\"%s\", &pCmdbuf, &cmdbufLen, %s, %s, %sLen);" % (
                    param_type, description, tag, param_name, param_name))
            else:
                ret_string.append("tlvRet = TLVSET_%s(\"%s\", &pCmdbuf, &cmdbufLen, %s, %s);"% (
                    param_type, description, tag, param_name))
            ret_string.append("if (0 != tlvRet) {")
            ret_string.append("    goto cleanup;")
            ret_string.append("}")
        if len(self.response):
            if self.LeCase == 4:
                ret_string.append("retStatus = DoAPDUTxRx_Case4(&hdr, cmdbuf, cmdbufLen, rspbuf, &rspbufLen);");
            else:
                ret_string.append("retStatus = DoAPDUTxRx_Case3(&hdr, cmdbuf, cmdbufLen, rspbuf, &rspbufLen);");
            ret_string.append("if (retStatus == SM_OK) {");
            ret_string.append("    retStatus = SM_NOT_OK;");
            ret_string.append("    size_t rspIndex = 0;");

            for (_, (tag, description, (param_name, param_type))) \
                in list(self.response.items()):
                if "u8buf" == param_type:
                    ret_string.append("    tlvRet = tlvGet_%s(&pRspbuf, &rspIndex, rspbufLen, %s, %s, p%sLen); /* %s */" % (
                        param_type, tag, param_name, param_name, description))
                else:
                    ret_string.append("    tlvRet = tlvGet_%s(&pRspbuf, &rspIndex, rspbufLen, %s, p%s); /* %s */" % (
                        param_type, tag, param_name, description))
                ret_string.append("    if (0 != tlvRet) {")
                ret_string.append("        goto cleanup;")
                ret_string.append("    }")
            ret_string.append("    if ((rspIndex + 2) == rspbufLen) {")
            ret_string.append("        retStatus = (pRspbuf[rspIndex] << 8 )| (pRspbuf[rspIndex + 1]);")
            ret_string.append("    }")
            ret_string.append("}");
        else:
            ret_string.append("retStatus = DoAPDUTx(&hdr, cmdbuf, cmdbufLen);");
        ret_string.append("\ncleanup:");
        ret_string.append("return retStatus;")
        return "\n    ".join(ret_string)

    def _header_string(self):
        pass

    def __str__(self):
        return self._pretty_dump()
