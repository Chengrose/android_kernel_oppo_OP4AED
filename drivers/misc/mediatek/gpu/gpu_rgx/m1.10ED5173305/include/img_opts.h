/*************************************************************************/ /*!
@File
@Title          Driver optimisations
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Branch-specific optimisation defines.
@License        Strictly Confidential.
*/ /**************************************************************************/

#if !defined(IMG_OPTS_H)
#define IMG_OPTS_H

#define IMG_1_10_OPT_DISABLE_FW_LOGGING_BY_DEFAULT 0x1
#define IMG_1_10_OPT_TDM_TWO_LEVEL_MIPGEN 0x2
#define IMG_1_10_OPT_REDUCE_MAX_ANISO_FILTER_LEVEL 0x4

#define IMG_1_11_OPT_SCREEN_SPACE_TEX_COORD_REPLACEMENT 0x2
#define IMG_1_11_OPT_VS_OUTPUT_ELIMINATION 0x8
#define IMG_1_11_OPT_TDM_SETUP 0x10
#define IMG_1_11_OPT_MIPS_DCACHE 0x20
#define IMG_1_11_OPT_AVOID_PR 0x40

#define IMG_1_12_OPT_TRILINEAR_TO_BILINEAR 0x4

#endif /* #if !defined(IMG_OPTS_H) */
/*****************************************************************************
 End of file (img_opts.h)
*****************************************************************************/
