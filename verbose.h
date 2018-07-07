/*
 * $Source: d:/source/tools/RCS/verbose.h,v $
 * $Revision: 1.3 $
 * $Date: 1998/06/04 06:32:22 $
 * $Author: vitus $
 *
 * Interface to verbose.c
 *
 * $Log: verbose.h,v $
 * Revision 1.3  1998/06/04 06:32:22  vitus
 * - added module to Verbose() call
 * - added SetDisplaylevel()
 *
 * Revision 1.2  1997/01/22 00:42:25  vitus
 * Added LOGBUFSIZ
 * Loglevel now unsigned
 *
 * Revision 1.1  1996/01/27 02:53:40  vitus
 * Initial revision
 */

#if !defined(LOGBUFSIZ)
# define LOGBUFSIZ	1024
#endif


#define VL_FATAL	0			/* fatal error */
#define VL_ERROR	1			/* prohibits normal operation */
#define VL_WARN		2			/* nice to know */
#define VL_OTHER	3			/* 'quassel' */


extern void	Verbose(unsigned level,char const * module,char const * fmt,...);
extern HFILE	SetLogfile(HFILE);		/* use 0 to close logfile */
extern unsigned	SetLoglevel(unsigned);
extern unsigned	SetDisplaylevel(unsigned);

