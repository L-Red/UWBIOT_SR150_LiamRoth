/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Bugfix for REL14 compatibility
 * TRNG is available on QN9090 but file and peripheral is named RNG
 * which causes a build error.
 * Create this file which includes fsl_rng to fix build.
 * References to TRNG must be replaced by RNG
 */

#include <fsl_rng.h>

