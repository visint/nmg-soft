/*
 *	unit.h -- Vispace Unit Management public header
 *
 *	Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
 *
 *	See the file "license.txt" for information on usage and redistribution
 *
 */

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

/******************************** Description *********************************/

/*
 *	Vispace Unit header. This defines the Unit Management
 *	public APIs.  Include this header for files that contain access to
 *	unit inquiry or management.
 */


/********************************* Includes ***********************************/

/********************************** Defines ***********************************/
//#ifndef snprintf
//#define snprintf			_snprintf
//#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef UCHAR_T
#define UCHAR_T 1
typedef unsigned char uchar_t;
#endif

#ifndef BOOL_T
  #define BOOL_T 1
  typedef char boolean_t;
#endif

#ifndef USHORT_T
	#define USHORT_T 1
	typedef unsigned short ushort_t;
#endif
typedef unsigned char uint8_t;
//typedef unsigned char uchar_t;
/*
#ifndef uchar_t
#define uchar_t unsigned char
#endif
*/
/*enum Bool
{
	FALSE=0,
	TRUE=1
};
#define bool_t enum Bool
*/
/*
enum Switch
{
	ON=1,
	OFF=2
};
#define SwitchStatus enum switch_t
*/
#endif /* _h_UNIT */

/******************************************************************************/



