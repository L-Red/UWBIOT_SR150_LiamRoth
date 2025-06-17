/* Copyright 2021-2023 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */

#ifndef FACTORY_FIRMWARE_H
#define FACTORY_FIRMWARE_H

#include <stdint.h>

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

/* Select one of the Helios FW images */

#if UWBIOT_UWBD_SR150
/*Select one of the firmware depending upon the host */
/* The following marker is used by firmware integration script, do not edit or delete these
   comments unintentionally, applicable for all the markers in this file */
/* Selection For SR150 FW Starts Here */
#   if UWBIOT_SR1XX_FW_ROW_PROD
#       define H1_IOT_SR150_FACTORY_PROD_FW_46_44_03_bin                    heliosEncryptedFactoryFwImage
#       define H1_IOT_SR150_FACTORY_PROD_FW_46_44_03_bin_len                heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_RHODES3_PROD
#       define SR140_SR150_FA_BX_RHODES3_PROD_20220316_a39c6fcb_B1v32_00_FB_bin              heliosEncryptedFactoryFwImage
#       define SR140_SR150_FA_BX_RHODES3_PROD_20220316_a39c6fcb_B1v32_00_FB_bin_len          heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_ROW_DEV
#       define H1_IOT_SR150_FACTORY_DEV_FW_46_44_03_bin                     heliosEncryptedFactoryFwImage
#       define H1_IOT_SR150_FACTORY_DEV_FW_46_44_03_bin_len                 heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_RHODES3_DEV
#       define SR140_SR150_FA_BX_RHODES3_20220316_a39c6fcb_B1v32_00_FB_bin                   heliosEncryptedFactoryFwImage
#       define SR140_SR150_FA_BX_RHODES3_20220316_a39c6fcb_B1v32_00_FB_bin_len               heliosEncryptedFactoryFwImageLen
#   else
#       error "Select anyone of the FW"
#   endif
#endif  //UWBIOT_UWBD_SR150

#if UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S
/*Select one of the firmware depending upon the host */
/* Selection For SR100T FW Starts Here */
/* Selection For SR100S FW Starts Here */
#   if UWBIOT_SR1XX_FW_ROW_PROD
#       define H1_MOBILE_ROW_FACTORY_PROD_FW_EE_20_A0_a8b28afc11bdaf6c_bin                      heliosEncryptedFactoryFwImage
#       define H1_MOBILE_ROW_FACTORY_PROD_FW_EE_20_A0_a8b28afc11bdaf6c_bin_len                  heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_RHODES3_PROD
#       define H1_MOBILE_ROW_FACTORY_RHODES3_PROD_FW_44_00_01_572492a66aac2909_bin             heliosEncryptedFactoryFwImage
#       define H1_MOBILE_ROW_FACTORY_RHODES3_PROD_FW_44_00_01_572492a66aac2909_bin_len         heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_ROW_DEV
#      define H1_MOBILE_ROW_FACTORY_DEV_FW_EE_20_A0_a8b28afc11bdaf6c_bin                        heliosEncryptedFactoryFwImage
#      define H1_MOBILE_ROW_FACTORY_DEV_FW_EE_20_A0_a8b28afc11bdaf6c_bin_len                    heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_RHODES3_DEV
#       define H1_MOBILE_ROW_FACTORY_RHODES3_DEV_FW_44_00_01_572492a66aac2909_bin                  heliosEncryptedFactoryFwImage
#       define H1_MOBILE_ROW_FACTORY_RHODES3_DEV_FW_44_00_01_572492a66aac2909_bin_len              heliosEncryptedFactoryFwImageLen
#   else
#       error "Select anyone of the FW"
#   endif
#endif //UWBIOT_UWBD_SR100T || UWBIOT_UWBD_SR100S


#if UWBIOT_UWBD_SR110T
/*Select one of the firmware depending upon the host */
/* Selection For SR110T FW Starts Here */
#   if UWBIOT_SR1XX_FW_ROW_PROD
#       define H1_MOBILE_ROW_FACTORY_PROD_FW_EE_20_A0_a8b28afc11bdaf6c_bin                      heliosEncryptedFactoryFwImage
#       define H1_MOBILE_ROW_FACTORY_PROD_FW_EE_20_A0_a8b28afc11bdaf6c_bin_len                  heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_RHODES3_PROD
#       define H1_MOBILE_ROW_FACTORY_RHODES3_PROD_FW_44_00_01_572492a66aac2909_bin             heliosEncryptedFactoryFwImage
#       define H1_MOBILE_ROW_FACTORY_RHODES3_PROD_FW_44_00_01_572492a66aac2909_bin_len         heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_ROW_DEV
#      define H1_MOBILE_ROW_FACTORY_DEV_FW_EE_20_A0_a8b28afc11bdaf6c_bin                        heliosEncryptedFactoryFwImage
#      define H1_MOBILE_ROW_FACTORY_DEV_FW_EE_20_A0_a8b28afc11bdaf6c_bin_len                    heliosEncryptedFactoryFwImageLen
#   elif UWBIOT_SR1XX_FW_RHODES3_DEV
#       define H1_MOBILE_ROW_FACTORY_RHODES3_DEV_FW_44_00_01_572492a66aac2909_bin                  heliosEncryptedFactoryFwImage
#       define H1_MOBILE_ROW_FACTORY_RHODES3_DEV_FW_44_00_01_572492a66aac2909_bin_len              heliosEncryptedFactoryFwImageLen
#   else
#       error "Select anyone of the FW"
#   endif
#endif //UWBIOT_UWBD_SR110T

#if UWBIOT_UWBD_SR160
/*Select one of the firmware depending upon the host */
/* Selection For SR160 FW Starts Here */
#  if UWBIOT_SR1XX_FW_ROW_PROD
#       define H1_IOT_SR150_FACTORY_PROD_FW_EE_40_02_746b01d41a254ace_bin                      heliosEncryptedFactoryFwImage
#       define H1_IOT_SR150_FACTORY_PROD_FW_EE_40_02_746b01d41a254ace_bin_len                  heliosEncryptedFactoryFwImageLen
#  elif UWBIOT_SR1XX_FW_ROW_DEV
#      define H1_IOT_SR150_FACTORY_DEV_FW_EE_40_02_746b01d41a254ace_bin                        heliosEncryptedFactoryFwImage
#      define H1_IOT_SR150_FACTORY_DEV_FW_EE_40_02_746b01d41a254ace_bin_len                    heliosEncryptedFactoryFwImageLen
#  elif UWBIOT_SR1XX_FW_RHODES3_PROD || UWBIOT_SR1XX_FW_RHODES3_DEV
#     error "FW is Not Supported for RV3"
#  else
#     error "Select anyone of the FW"
   #endif
#endif //UWBIOT_UWBD_SR160

/* FW selection End */

/* Depending on the selection, header file would be chosen
 *
 * Steps for the header file generation:
 *
 * 1) Use xxd -i ${file_name}.bin > ${file_name}.h (or similar utility)
 * 2) in the generated header file, replace ``unsigned char`` to ``const uint8_t``
 */
#ifdef IOT_Factory_Prod_EE_40_B0_bin
#   include <IOT_Factory_Prod_EE_40_B0.h>
#endif

#ifdef IOT_Factory_Dev_EE_40_B0_bin
#   include <IOT_Factory_Dev_EE_40_B0.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_43_B1_84227a5174075ab0_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.43.B1_84227a5174075ab0.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_43_B1_84227a5174075ab0_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.43.B1_84227a5174075ab0.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_43_D1_262ade747d02c8f0_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.43.D1_262ade747d02c8f0.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_44_01_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.44.01.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_AD_42_03_253da558bab76451_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_AD.42.03_253da558bab76451.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_AD_42_03_253da558bab76451_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_AD.42.03_253da558bab76451.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_41_06_0052bbfed983a1f1_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.41.06_0052bbfed983a1f1.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_41_06_0052bbfed983a1f1_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.41.06_0052bbfed983a1f1.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_44_03_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.44.03.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_44_03_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.44.03.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_44_02_b0a4ee30e3799074_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.44.02_b0a4ee30e3799074.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_44_02_b0a4ee30e3799074_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.44.02_b0a4ee30e3799074.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_41_03_d35a37071f42eed4_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.41.03_d35a37071f42eed4.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_41_03_d35a37071f42eed4_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.41.03_d35a37071f42eed4.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_41_01_fe58b4e0def9bc65_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.41.01_fe58b4e0def9bc65.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_41_01_fe58b4e0def9bc65_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.41.01_fe58b4e0def9bc65.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_44_01_db946fe9e347b786_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.44.01_db946fe9e347b786.h>
#endif

#ifdef H1_MOBILE_ROW_FACTORY_RHODES3_PROD_FW_44_00_01_572492a66aac2909_bin
#   include <H1_MOBILE_ROW_FACTORY_RHODES3_PROD_FW_44.00.01_572492a66aac2909.h>
#endif

#ifdef H1_MOBILE_ROW_FACTORY_RHODES3_DEV_FW_44_00_01_572492a66aac2909_bin
#   include <H1_MOBILE_ROW_FACTORY_RHODES3_DEV_FW_44.00.01_572492a66aac2909.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_41_05_6e5b54433f6445e9_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.41.05_6e5b54433f6445e9.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_41_05_6e5b54433f6445e9_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.41.05_6e5b54433f6445e9.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_43_A2_626936e6430bc8e9_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.43.A2_626936e6430bc8e9.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_43_A2_626936e6430bc8e9_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.43.A2_626936e6430bc8e9.h>
#endif

#ifdef SR140_SR150_FA_BX_RHODES3_20220316_a39c6fcb_B1v32_00_FB_bin
#   include <SR140_SR150_FA_BX_RHODES3_20220316_a39c6fcb_B1v32.00.FB.h>
#endif

#ifdef SR140_SR150_FA_BX_RHODES3_PROD_20220316_a39c6fcb_B1v32_00_FB_bin
#   include <SR140_SR150_FA_BX_RHODES3_PROD_20220316_a39c6fcb_B1v32.00.FB.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_43_01_7d38a06f4be3a12c_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.43.01_7d38a06f4be3a12c.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_43_01_7d38a06f4be3a12c_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.43.01_7d38a06f4be3a12c.h>
#endif

#ifdef H1_MOBILE_ROW_FACTORY_PROD_FW_EE_20_A0_a8b28afc11bdaf6c_bin
#   include <H1_MOBILE_ROW_FACTORY_PROD_FW_EE.20.A0_a8b28afc11bdaf6c.h>
#endif

#ifdef H1_MOBILE_ROW_FACTORY_DEV_FW_EE_20_A0_a8b28afc11bdaf6c_bin
#   include <H1_MOBILE_ROW_FACTORY_DEV_FW_EE.20.A0_a8b28afc11bdaf6c.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_EE_40_A0_a8b28afc11bdaf6c_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_EE.40.A0_a8b28afc11bdaf6c.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_EE_40_A0_a8b28afc11bdaf6c_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_EE.40.A0_a8b28afc11bdaf6c.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_41_02_d983ae9b7f25963d_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.41.02_d983ae9b7f25963d.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_41_02_d983ae9b7f25963d_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.41.02_d983ae9b7f25963d.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_EE_40_02_746b01d41a254ace_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_EE.40.02_746b01d41a254ace.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_EE_40_02_746b01d41a254ace_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_EE.40.02_746b01d41a254ace.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_PROD_FW_46_41_04_fd77c7cbfb1b28ee_bin
#   include <H1_IOT.SR150_FACTORY_PROD_FW_46.41.04_fd77c7cbfb1b28ee.h>
#endif

#ifdef H1_IOT_SR150_FACTORY_DEV_FW_46_41_04_fd77c7cbfb1b28ee_bin
#   include <H1_IOT.SR150_FACTORY_DEV_FW_46.41.04_fd77c7cbfb1b28ee.h>
#endif

#endif // FACTORY_FIRMWARE_H
