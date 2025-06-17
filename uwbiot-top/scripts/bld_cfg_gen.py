# Copyright 2021 NXP
#
# NXP Confidential. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

import os

COPYRIGHT = """/*
 *
 * Copyright 2021-2023 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!! AUTO GENERATED FILE  !!!!!!!!!!!!!!!!!!!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#pragma once

"""


def usage(script_name, demos):
    print("Usage:")
    print("   %s [selected_demo]\n" % script_name)
    print("selected_demo Can be one of:")
    for num, selected_demo in enumerate(demos):
        print("     - %d / %s" % (num, selected_demo))
    # exit(1)


def run(demos, demos_se, selected_demo):
    with open("UWBIOT_APP_BUILD.h", "w") as o:
        o.write(COPYRIGHT)
        o.write("/* Options for selection of the demos : START */\n\n")
        o.write(
            "/* Uncomment **ONE**, and comment out **rest** of them. */\n\n"
        )
        # Defines for UWB demos
        for demo in demos:
            if demo == selected_demo:
                o.write("#define UWBIOT_APP_BUILD__%s\n" % demo)
            else:
                o.write("// #define UWBIOT_APP_BUILD__%s\n" % demo)

        # Defines for SE-only demos
        for demo_se in demos_se:
            if demo_se == selected_demo:
                o.write("#define SIMW_DEMO_ENABLE__%s\n" % demo_se)
            else:
                o.write("// #define SIMW_DEMO_ENABLE__%s\n" % demo_se)

        o.write("\n/* Options for selection of the demos : END */\n")
        o.write("\n")
        o.write(
            "/* Helper compile time check to safeguard wrong selection at compile time. */"
        )
        o.write("\n\n")
        o.write("/* clang-format off */\n")

        macro_list = []
        for demo in demos:
            macro_list.append("UWBIOT_APP_BUILD__%s" % (demo))
        for demo_se in demos_se:
            macro_list.append("SIMW_DEMO_ENABLE__%s" % (demo_se))

        for i in range(len(macro_list)):
            macro1 = macro_list[i]
            for j in range(i, len(macro_list)):
                macro2 = macro_list[j]
                if macro1 != macro2:
                    o.write(
                        "#if defined(%s) && defined(%s)\n" % (macro1, macro2)
                    )
                    o.write(
                        "#    error Can not define both '%s' and '%s'\n" %
                        (macro1, macro2)
                    )
                    o.write("#endif\n\n")

        o.write("/* clang-format on */\n")


def dstfile(demos, selected_demo):
    if selected_demo:
        with open("target_file_name.sh", "w") as o:
            o.write("TARGET_FILE_NAME=%s\n" % selected_demo)
            o.write("export TARGET_FILE_NAME\n")
    else:
        try:
            os.unlink("target_file_name.sh")
        except FileNotFoundError:
            # file does not exist, ok. No worries.
            pass


def helperMacroPrint_h(demos):
    with open("helperMacroPrint.h.bak", "w") as o:
        o.write("/* do not commit this file */\n\n")
        for demo in demos:
            o.write(
                """------------------------------------------

#ifndef UWBIOT_APP_BUILD__{0}
#include "UWBIOT_APP_BUILD.h"
#endif

#ifdef UWBIOT_APP_BUILD__{0}


#endif // (UWBIOT_APP_BUILD__{0}

------------------------------------------""".format(demo)
            )


def helperMacroPrint_cmake(demos):
    with open("helperMacroPrint.txt.bak", "w") as o:
        o.write("/* do not commit this file */\n\n")
        for demo in demos:
            o.write(
                """------------------------------------------

TARGET_COMPILE_DEFINITIONS(
    ${{PROJECT_NAME}}
    PUBLIC
        UWBIOT_APP_BUILD__{0}
)

------------------------------------------""".format(demo)
            )


def generate(sys_argv, demos_str=None, demos_se_str=None):
    if demos_str is not None:
        demos = demos_str.split()
    else:
        demos = []

    if demos_se_str is not None:
        demos_se = demos_se_str.split()
    else:
        demos_se = []

    selected_demo = None
    if len(sys_argv) == 1:
        if len(demos) > 0:
            selected_demo = demos[0]
        elif len(demos_se) > 0:
            selected_demo = demos_se[0]
    elif sys_argv[1] in demos:
        selected_demo = sys_argv[1]
    elif sys_argv[1] in demos_se:
        selected_demo = sys_argv[1]
    elif sys_argv[1].isnumeric() and int(sys_argv[1]) < len(demos):
        int_argv = int(sys_argv[1])
        selected_demo = demos[int_argv]
    elif sys_argv[1].isnumeric() and int(sys_argv[1]) < len(demos_se):
        int_argv = int(sys_argv[1])
        selected_demo = demos_se[int_argv]
    else:
        usage(sys_argv[0], demos)
    if selected_demo:
        run(demos, demos_se, selected_demo)
        print("Generated. Selected %s as target." % selected_demo)
    dstfile(demos, selected_demo)
    helperMacroPrint_h(demos)
    helperMacroPrint_cmake(demos)
    print("Total %d entries" % (len(demos) + len(demos_se)))
