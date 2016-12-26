/*
 *	unit.h -- Vispace Alarm Management public header
 *
 *	Copyright (c) Visint Inc., 2002-2012. All Rights Reserved.
 *
 *	See the file "license.txt" for information on usage and redistribution
 *
 */

#ifndef __INICONFIG_H
#define __INICONFIG_H 1

/******************************** Description *********************************/

/*
 *	INIfile header. This defines the INI file config
 *	public APIs.  Include this header for files that contain access to
 *	unit inquiry or management.
 */

/********************************* Includes ***********************************/

/********************************** Defines ***********************************/
extern int  fileCopy(void *source_file, void *dest_file); 
extern int  splitSectionToNameIndex(char *section, char **name, char **index);
extern int  joinNameIndexToSection(char **section, char *name, char *index);
extern int  configGetKey(void *CFG_file, void *section, void *key, void *buf); 
extern int  configSetKey(void *CFG_file, void *section, void *key, void *buf); 
extern int  configGetSections(void *CFG_file, char *sections[]); 
extern int  configGetKeys(void *CFG_file, void *section, char *keys[]);
extern void configDeleteSection(const char *CFG_file, const char *section);

/*缺省配置文件/etc/vispace.conf*/
extern int  visConfGetKey(void *section, void *key, void *buf); 
extern int  visConfSetKey(void *section, void *key, void *buf); 
extern int  visConfGetSections(char *sections[]); 
extern int  visConfGetKeys(void *section, char *keys[]);
extern void visConfDeleteSection(const char *section);

#endif /* _H_INICONFIG */

/******************************************************************************/



