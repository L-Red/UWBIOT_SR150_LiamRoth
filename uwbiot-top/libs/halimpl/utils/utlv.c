/* Copyright 2020 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

/* UWB TLV */

#include <utlv.h>
#include <uwb_types.h>
#include <string.h>
#include <phNxpLogApis_App.h>
#include <nxEnsure.h>

static utlv_status_t utlv_checkLengthAndType(utlv_entry_t *pEntry, uint8_t tag_lenght)
{
    utlv_status_t retStatus = kUTLV_Status_FailedWrongType;
    switch (pEntry->tag_type) {
    case kUTLV_Type_Unknown: {
        retStatus = kUTLV_Status_Found;
        switch (tag_lenght) {
        case 1:
            pEntry->tag_type = kUTLV_Type_u8;
            break;
        case 2:
            pEntry->tag_type = kUTLV_Type_u16;
            break;
        case 4:
            pEntry->tag_type = kUTLV_Type_u32;
            break;
        default:
            pEntry->tag_type = kUTLV_Type_au8;
            break;
        }
    } break;
    case kUTLV_Type_u8:
        if (1 == tag_lenght) {
            retStatus = kUTLV_Status_Found;
        }
        break;
    case kUTLV_Type_u16:
        if (2 == tag_lenght) {
            retStatus = kUTLV_Status_Found;
        }
        break;
    case kUTLV_Type_u32:
        if (4 == tag_lenght) {
            retStatus = kUTLV_Status_Found;
        }
        break;
    case kUTLV_Type_au8:
        retStatus = kUTLV_Status_Found;
        break;
    default:
        break;
    }
    return retStatus;
}

utlv_status_t utlv_parse_entry(const uint8_t *pBuf, size_t ubufLen, utlv_entry_t *pEntry)
{
    utlv_status_t retStatus = kUTLV_Status_Failed;
    int bufLen              = (int)ubufLen;
    ENSURE_OR_GO_CLEANUP(pBuf != NULL);
    ENSURE_OR_GO_CLEANUP(pEntry != NULL);
    ENSURE_OR_GO_CLEANUP(ubufLen >= 2);
    ENSURE_OR_GO_CLEANUP(ubufLen <= 255);

    while (retStatus != kUTLV_Status_Found && bufLen >= 2) {
        if (pEntry->tag == *pBuf) {
            uint8_t tag_length;
            pBuf++;
            bufLen--;
            //we will get length from here
            tag_length = *pBuf;

            /* we know type, neet to match wth lenght information */
            retStatus = utlv_checkLengthAndType(pEntry, tag_length);
            if (retStatus != kUTLV_Status_Found) {
                goto cleanup;
            }

            /* at the value */
            pBuf++;
            switch (pEntry->tag_type) {
            case kUTLV_Type_Unknown:
            default:
                tag_length = 0;
                break;
            case kUTLV_Type_u8:
                pEntry->tag_value.vu8 = *pBuf;
                pBuf += tag_length;
                break;
            case kUTLV_Type_u16:
                UWB_STREAM_TO_UINT16((pEntry->tag_value.vu16), pBuf);
                break;
            case kUTLV_Type_u32:
                UWB_STREAM_TO_UINT32((pEntry->tag_value.vu32), pBuf); //littile to big
                break;
            case kUTLV_Type_au8:
                if (pEntry->tag_value.au8.ptr == NULL) {
                    pEntry->tag_value.au8.ptr    = (uint8_t *)pBuf;
                    pEntry->tag_value.au8.outLen = tag_length;
                }
                else if (pEntry->tag_value.au8.inMaxLen >= tag_length && pEntry->tag_value.au8.ptr != NULL) {
                    phOsalUwb_MemCopy(pEntry->tag_value.au8.ptr, pBuf, tag_length);
                    pEntry->tag_value.au8.outLen = tag_length;
                }
                else {
                    retStatus = kUTLV_Status_FailedBufferOverflow;
                }
                pBuf += tag_length;
                break;
            }
            bufLen -= tag_length;
        }
        else {
            /* Skip to next tag */
            uint8_t tag_length;
            pBuf++;
            bufLen--;
            //we will get length from here
            tag_length = *pBuf;
            pBuf += 1; /* Point to the tag */
            bufLen -= 1;
            bufLen -= tag_length;
            pBuf += tag_length;
            retStatus = kUTLV_Status_NotFound;
        }
    }

cleanup:
    return retStatus;
}

//utlv_status_t utlv_parse_entries(const uint8_t *pBuf, size_t bufLen, utlv_entry_t *pEntries, size_t entrySize)
//{
//    return kUTLV_Status_Found;
//}
