/* Copyright 2022 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only
 * be used strictly in accordance with the applicable license terms.  By
 * expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you may
 * not retain, install, activate or otherwise use the software.
 */
/*
** ###################################################################
**     Processors:          QN9090
**
**     Compiler:            GNU C Compiler
**     Reference manual:    QN9090 Series Reference Manual
**
**     Abstract:
**         Linker file for the GNU C Compiler
**
**     Copyright  2019 NXP
**     All rights reserved.
**
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**
**     o Neither the name of NXP Semiconductors, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
** ###################################################################
*/


/******************* Map of QN9090 FLASH ***********************************


             0x000a_0000    m_flash_end
    - - - +---------------+ - - - - - - - -
          |   _ _ _ _ _   |
          | Flash config  |
   8.5k   |   - - - - -   |
          |               |
          |  RESERVED     |  0x9de00
    - - - +---------------+ - - - - - - - -
          |               | m_fsl_prodInfo_end
    512   | Product Info  |
          |               | m_fsl_prodInfo_start       0x9dc00
    - - - +---------------+ - - - - - - - -
          |               | NV_STORAGE_START_ADDRESS
    32k   |  sNVM_region  |
          | - - - - - - - | --- NV_VIRTUAL_PAGE_BOUND
          |  fNVM_region  |
          |               | NV_STORAGE_END_ADDRESS
    - - - +---------------+ - - - - - - - -
          |               | INT_STORAGE_START
          |               |
          |               |
   ~311k  | OTAP Internal |
          |      Storage* |
          |               |
          |               |
          |               | INT_STORAGE_END = 0x48000
    - - - +---------------+ - - - - - - - -
          |               | m_text_end
          |               |
          |               |
  ~278k** |  Application  |
          |               |
          |               |
          |               | m_text_start
    - - - +---------------+ - - - - - - - -
          |               | m_bootFlags_end
 8 bytes  |  Boot Flags*  |
          |               | m_bootFlags_start
    - - - +---------------+ - - - - - - - -
          |   Interrupt   | m_interrupts_end
  0x120   |    Vectors    |
  bytes   |               | m_interrupts_start
    - - - +---------------+ - - - - - - - -
          |               | m_bootloader_end
          |               |
   10k    |  Bootloader*  |
          |               |
          |               | m_bootloader_start = m_flash_start
    - - - +---------------+ - - - - - - - -
             0x0000_0000

 * - These sections are optional for certain applications if gUseInternalStorageLink_d is set
 ** - Application will occupy all the free space available
 *****************************************************************************/
 /* INCLUDE "QN9090_UWB_TAG_FW_libraries.ld"*/

GROUP (
  "libgcc.a"
  "libc_nano.a"
  "libstdc++_nano.a"
  "libm.a"
  "libcr_newlib_nohost.a"
)


/* Entry Point */
ENTRY(ResetISR)

/*** flash memory characteristics ***/
m_flash_start   = 0x00000000;
m_flash_end     = 0x0009FFFF;
m_flash_size    = 0x000A0000;
m_sector_size   = 512;

/****************************************************
 *******         User Defines                 *******
 ****************************************************/
UseNVMLink              = DEFINED(gUseNVMLink_d)            ? gUseNVMLink_d             : 0;
NVMSectorCountLink      = DEFINED(gNVMSectorCountLink_d)    ? gNVMSectorCountLink_d     : ( DEFINED(gUseNVMLink_d) ? 32 : 0 );
ram_vector_table        = DEFINED(__ram_vector_table__)     ? __ram_vector_table__      : 0;
UseBootloaderLink       = DEFINED(gUseBootloaderLink_d)     ? gUseBootloaderLink_d      : 0;
UseInternalStorageLink  = DEFINED(gUseInternalStorageLink_d)? gUseInternalStorageLink_d : 0;

m_app_size  = DEFINED(__app_stated_size__) ? __app_stated_size__ : 0x48000;
m_app_start = DEFINED(__app_load_address__) ? __app_load_address__ : 0x0;

m_app_end                 = m_app_start + m_app_size - 1;


HEAP_SIZE               = DEFINED(__heap_size__)            ? __heap_size__             : 0;
STACK_SIZE              = DEFINED(__stack_size__)           ? __stack_size__            : ( DEFINED(__special_stack_size__) ? __special_stack_size__ : (0x1000 - 32));
vector_table_size       = 0x120;

/*
 * stack size for the boot rom during warm boot and application
 * 256 is sufficient (pwrm_test) but keep it large to 1024
 */
BOOT_RESUME_STACK_SIZE = 1024;


/****************************************************
 ******          Flash regions sizes           ******
 ****************************************************/
m_bootloader_size       = (UseBootloaderLink) ? 20 * m_sector_size : 0;
/* m_bootloader_size       = 0; */
m_bootFlags_size        = (UseBootloaderLink) ? 0x10 : 0x0;
NV_STORAGE_SIZE         = NVMSectorCountLink * m_sector_size;
NV_STORAGE_MAX_SECTORS  = NVMSectorCountLink;
NV_STORAGE_SECTOR_SIZE = m_sector_size;
m_fsl_prodInfo_size     = m_sector_size;

/*
*  m_int_start_addr must be 8k aligned for remap to work
* can be as big as 0x4c000 i.e. 311k
*/
m_int_start_addr        = m_app_end + 1;

/****************************************************
 ******        Flash Regions, low to high      ******
 ****************************************************/
m_bootloader_start      = m_flash_start; /* + m_bootloader_size , can be zero */
m_interrupts_start      = m_bootloader_size;
m_interrupts_end        = m_interrupts_start + vector_table_size - 1;
m_bootFlags_start       = m_interrupts_end + 1;
m_bootFlags_end         = m_bootFlags_start + m_bootFlags_size - 1;
m_text_start            = m_bootFlags_end + 1;



/****************************************************
 *****        Flash Regions, high to low       ******
 ****************************************************/
m_fsl_prodInfo_end      = m_flash_size - 17 * m_sector_size - 1;
m_fsl_prodInfo_start    = m_fsl_prodInfo_end - m_fsl_prodInfo_size + 1;

NV_STORAGE_START_ADDRESS = m_fsl_prodInfo_start - 1;
NV_STORAGE_END_ADDRESS  = NV_STORAGE_START_ADDRESS - NV_STORAGE_SIZE + 1;
INT_STORAGE_START       = NV_STORAGE_END_ADDRESS - 1;
INT_STORAGE_END         = (UseInternalStorageLink) ? m_int_start_addr : INT_STORAGE_START;
INT_STORAGE_SIZE        = INT_STORAGE_START - INT_STORAGE_END;
m_text_end              = INT_STORAGE_END - 1;



/* Other Defines */
 m_text_size             = m_text_end + 1 - m_text_start ;


__BootFlags_Start__     = m_bootFlags_start;
INT_STORAGE_SECTOR_SIZE = m_sector_size;
M_VECTOR_RAM_SIZE       = DEFINED(__ram_vector_table__) ? vector_table_size : 0x0;
FREESCALE_PROD_DATA_BASE_ADDR = m_fsl_prodInfo_start;


/* Specify the memory areas */
MEMORY
{
  /* Define each memory region */
    PROGRAM_FLASH        (RX) : ORIGIN = m_interrupts_start,     LENGTH = m_text_size
  SCRATCH_RAM(rwx)   : ORIGIN = 0x4000000, LENGTH = 0x400   /* 1K bytes (alias SCRATCH_RAM) */
  SRAM (rwx)    : ORIGIN = 0x4000400, LENGTH = 0x15c00 /* 87K bytes (alias RAM) */
  SRAM1 (rwx)    : ORIGIN = 0x4020000, LENGTH = 0x10000 /* 64K bytes (alias RAM2) */
}

  /* Define a symbol for the top of each memory region */
  __base_PROGRAM_FLASH = 0x0  ; /* PROGRAM_FLASH */
  __base_Flash = 0x0 ; /* Flash */
  __top_PROGRAM_FLASH = 0x0 + 0xa0000 ; /* 640K bytes */
  __top_Flash = 0x0 + 0xa0000 ; /* 640K bytes */
  __base_unretainedRAM0 = 0x4000000  ; /* RAM0 bank : trampled at Warm boot, can be used as scratch area */
  __top_unretainedRAM0 =  0x4000400  ; /* start of normal use RAM0 */
  __base_SRAM = 0x4000400  ; /* SRAM */
  __base_RAM0 = 0x4000400  ; /* SRAM */
  __base_RAM = 0x4000400 ; /* RAM */
  __top_SRAM = 0x4000400 + 0x15c00 ; /* 87K bytes */
  __top_RAM = 0x4000400 + 0x15c00 ; /* 87K bytes */
  __base_SRAM1 = 0x4020000  ; /* SRAM1 */
  __base_RAM2 = 0x4020000 ; /* RAM2 */
  __top_SRAM1 = 0x4020000 + 0x10000 ; /* 64K bytes */
  __top_RAM2 = 0x4020000 + 0x10000 ; /* 64K bytes */
  /* HEAP_SIZE  = 0xFA0; */
  HEAP_SIZE  = 0x0200;


SECTIONS
{
    /* MAIN TEXT SECTION */
    .m_interrupts : ALIGN(4)
    {
        _flash_start = ABSOLUTE(.);
        _flash_beg = ABSOLUTE(.);

        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        __VECTOR_TABLE = .;
        KEEP(*(.isr_vector))
        FILL(0xff)
        . = ALIGN (0x10);
    } > PROGRAM_FLASH

    .boot_flags :
    {
        . = ALIGN(4);
  __BootFlags_Start__ = ABSOLUTE(.);
        *(.BootloaderFlags) /* Boot Flags */
        . = ALIGN(4);
    } > PROGRAM_FLASH

    /* MAIN TEXT SECTION */
    .data_bss : ALIGN(10)
    {
        /* Global Section Table */
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data_RAM2));
        LONG(    ADDR(.data_RAM2));
        LONG(  SIZEOF(.data_RAM2));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss_RAM2));
        LONG(  SIZEOF(.bss_RAM2));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        FILL(0xff)
        . = ALIGN (0x10);
    } > PROGRAM_FLASH

    .ro_nonce : ALIGN(0x10)
    {
        _FlsNonceStart = ABSOLUTE(.);
        *(.ro_nonce) /* nonce value is 16 bytes.*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > PROGRAM_FLASH

    .ro_ota_header : ALIGN(0x10)
    {
        _enc_start = ABSOLUTE(.);
        _enc_offset = (_enc_start & 0x0000000F);
        _FlsOtaHeader = ABSOLUTE(.);
        *(.ro_ota_header) /* Ota Header 69 bytes*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > PROGRAM_FLASH

    .ro_se_lnkKey (ALIGN((. - _enc_offset), 16) + _enc_offset):
    {
        _FlsLinkKey = ABSOLUTE(.);
        *(.ro_se_lnkKey)  /* Link Key 16 bytes*/
        FILL(0xff)
        . = ALIGN (0x10);
    } > PROGRAM_FLASH

    .filler :
    {
        FILL(0xff);
        . = ALIGN(0x40);
    } > PROGRAM_FLASH

    .text : ALIGN(0x40)
    {
        FILL(0xff)

       *(.after_vectors*)
       *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
    } > PROGRAM_FLASH

    .NVM_TABLE :
    {
      . = ALIGN(4);
      PROVIDE(__start_NVM_TABLE = .);
      KEEP(*(.NVM_TABLE));
      PROVIDE(__stop_NVM_TABLE = .);
      . = ALIGN(4);
    } > PROGRAM_FLASH

    .VERSION_TAGS :
    {
        . = ALIGN(4);
        PROVIDE(__start_VERSION_TAGS = .);
        KEEP(*(.VERSION_TAGS));
        PROVIDE(__stop_VERSION_TAGS = .);
        . = ALIGN(4);
    } > PROGRAM_FLASH

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this.
     */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > PROGRAM_FLASH
    __exidx_start = .;

    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > PROGRAM_FLASH
    __exidx_end = .;

    _etext = .;

    /* DATA section for SRAM1 */
    .data_RAM2 : ALIGN(4)
    {
        FILL(0xff)
        PROVIDE(__start_data_RAM2 = .) ;
        *(.ramfunc.$RAM2)
        *(.ramfunc.$SRAM1)
        *(.data.$RAM2*)
        *(.data.$SRAM1*)
        . = ALIGN(4) ;
        PROVIDE(__end_data_RAM2 = .) ;
     } > SRAM1 AT>PROGRAM_FLASH

    .interrupts_ram :
    {
        . = ALIGN(4);
        __VECTOR_RAM__ = .;
        __interrupts_ram_start__ = .;   /* Create a global symbol at data start */
        *(.m_interrupts_ram)            /* This is a user defined section */
        . += M_VECTOR_RAM_SIZE;
        . = ALIGN(4);
        __interrupts_ram_end__ = .;     /* Define a global symbol at data end */
    } > SRAM
    .scratch_area (NOLOAD): ALIGN(4)
    {
       __scratch_area_start__ = .;
       . = ALIGN(4) ;
       . += 0x400;
       __scratch_area_top__ = .;

    } > SCRATCH_RAM

    /* Main DATA section (SRAM) */
    .data : ALIGN(4)
    {
       FILL(0xff)
       *(.ll_exchange_mem)   /* TO fix : create a specific section for *(.ll_exchange_mem) before the . data section */
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
    } > SRAM AT>PROGRAM_FLASH

    __VECTOR_RAM = __VECTOR_RAM__;
    __RAM_VECTOR_TABLE_SIZE_BYTES = DEFINED(__ram_vector_table__) ? (__interrupts_ram_end__ - __interrupts_ram_start__) : 0x0;

    /* BSS section for SRAM1 */
    .bss_RAM2 (NOLOAD) : ALIGN(4)
    {
       PROVIDE(__start_bss_RAM2 = .) ;
       . = ALIGN(4);
       *(.bss_RAM2)
       *(.bss.$RAM2*)
       *(.bss.$SRAM1*)
       . = ALIGN (. != 0 ? 4 : 1) ; /* avoid empty segment */
       PROVIDE(__end_bss_RAM2 = .) ;
    } > SRAM1

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;

        PROVIDE(end = .);
    } > SRAM

    /* NOINIT section for SRAM1 */
    .noinit_RAM2 (NOLOAD) : ALIGN(4)
    {
       *(.noinit.$RAM2*)
       *(.noinit.$SRAM1*)
       . = ALIGN(4) ;
    } > SRAM1

    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*)
        . = ALIGN(4) ;
        _end_noinit = .;
    } > SRAM

    /* end of firmware RAM to be retained in power down mode */
    _end_fw_retention = .;

    /* For now, Heap does not need to ne retained as configuration is baremetal
       and no allocator is implemented. Used only ponctually by stdio (sprintf) */
    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : .);
    PROVIDE(_pvHeapLimit = _pvHeapStart + (HEAP_SIZE));

    PROVIDE(_scratch_buf_start = __scratch_area_start__);
    PROVIDE(_scratch_buf_end = __scratch_area_top__);
    /* stack for rom boot during warm resume */
    .boot_resume_stack (NOLOAD): ALIGN(4)
    {
        _boot_resume_stack = .;
        *(.boot_resume_stack*)
        . += BOOT_RESUME_STACK_SIZE;
        . = ALIGN(4) ;
        _end_boot_resume_stack = .;
    } > SRAM

    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_SRAM - 32);
    __StackLimit = _vStackTop - STACK_SIZE;
    /* note : in power down, the usage of the stack is reduced, so BOOT_RESUME_STACK_SIZE is not considered
       (Bare metal only)*/
    ASSERT(__StackLimit >= _pvHeapLimit, "Possible stack corruption with data/bss")

}
ASSERT(m_app_size % 8192 == 0, "Stated size must be a multiple of 8k")

GROUP(libcr_nohost.a libcr_c.a libcr_eabihelpers.a)
