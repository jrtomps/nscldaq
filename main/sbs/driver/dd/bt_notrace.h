/*****************************************************************************
**      Filename:   bt_notrace.h
**
**      Purpose:    dummy driver trace macros and defines for OS unique code.
**
**      $Revision: 1.3 $
**
*****************************************************************************/
/*****************************************************************************
**      Copyright (c) 1997-2004 by SBS Technologies, Inc.
**                     All Rights Reserved.
**              License governs use and distribution.
**
*****************************************************************************/

#if     !defined(_BT_NOTRACE_H)
#define _BT_NOTRACE_H

#define BT_FILE_NUMBER(fnumber) /* */
#define FUNCTION(str) /* */
#define LOG_UNKNOWN_UNIT /* */
#define FATAL_STR(str) /* */
#define FATAL_MSG(trc, str) /* */
#define WARN_STR(str) /* */
#define WARN_MSG(trc, str) /* */
#define INFO_STR(str) /* */
#define INFO_MSG(trc, str) /* */
#define FENTRY /* */
#define FEXIT(exit_val) /* */
#define TRC_STR(trc,str) /* */
#define TRC_MSG(trc,str) /* */
#define DBG_STR(str) /* */
#define DBG_MSG(trc,str) /* */
#define LOG_FMT /* */
#define LOG_ARG /* */
#ifndef LINE_STR 
#define LINE_STR /* */
#endif
#ifndef __FILE__ 
#define __FILE__ /* */
#endif

#endif  /* !defined(_BT_NOTRACE_H) */
