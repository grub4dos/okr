/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef RESOURCE_H
#define RESOURCE_H

#include "commondef.h"
#include "window.h"
#define STRING_ASCII L" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`~!@#$%^&*()-_+={[]}|\:;',<.>/?"
#define STRING_CHINESE L"卷文档"	//可能用到的汉字
#define STRING_WELCOME L"Welcome"
#define STRING_TIME L"Time"
#define STRING_MEMO L"Description"
#define STRING_SERIAL L"No."
#define STRING_SELECTFILE L"Please select the drive that contains the backup"
#define STRING_INPUTNAME L"Backup Description"
#define STRING_PARTINFO L"Size"
#define STRING_PARTINFO1 L"Free"
#define STRING_COMPLETE L"Click\"DONE\"to restart the system."
#define STRING_CANCEL L"Press ESC to cancel..."
#define STRING_USED_TIME L"Elapsed:"
#define STRING_REMAIN_TIME L"Remaining:"

//#define STRING_SELECTPART L"Please select the drive that contains the backup"
#define STRING_BACKUPHINT   L"Please select the drive to store the "
#define STRING_BACKUPHINT1  L"backup, the backup will take %d "
#define STRING_BACKUPHINT2  L"disk space"

#define STRING_DISK_TOO_SMALL L"The backup file is larger than the hard disk, cannot restore!"
#define STRING_DRIVE_MIGHT_SMALL L"The drive does not have enough space for the backup, select Cancel to stop!"

#define STRING_HINTPARTTABLE L"Restore partition tables?"
#define STRING_HINTNOSYSPART L"Cannot detect the system partition!"
#define STRING_HINTNOCAP L"The drive does not have enough free space!"
#define STRING_YES L"OK"
#define STRING_NO L"Cancel"
//#define STRING_INPUTNAME L"Please Enter backup name "
//#define STRING_PARTINFO L"Cap."
//#define STRING_SELECTPART L"Please Select Drive"
//#define STRING_BACKUPHINT L"Begin to backup"
//

#define IMG_FILE_BACKGROUND L"bk.jpg"

#define IMG_FILE_MOUSEA L"mousea.lzp"
#define IMG_FILE_CURSORTEXT L"curstext.lzp"
#define IMG_FILE_CURSORMOVE L"cursmove.lzp"
#define IMG_FILE_CURSORHAND L"curshand.lzp"
#define IMG_FILE_BEIJIN L"beijin.jpg"
#define IMG_FILE_BEIJIN1 L"beijin1.jpg"

#define IMG_FILE_CANCELA L"cancela.lzp"
#define IMG_FILE_CANCELB L"cancelb.lzp"
#define IMG_FILE_CANCELC L"cancelc.lzp"
#define IMG_FILE_UPARROWA L"uparrowa.jpg"
#define IMG_FILE_UPARROWB L"uparrowb.lzp"
#define IMG_FILE_DOWNARROWA L"dnarrowa.jpg"
#define IMG_FILE_DOWNARROWB L"dnarrowb.lzp"
#define IMG_FILE_RADIOA L"radioa.lzp"
#define IMG_FILE_RADIOB L"radiob.lzp"
#define IMG_FILE_RADIOC L"radioc.lzp"
#define IMG_FILE_DROPDOWNA L"dropdna.lzp"
#define IMG_FILE_DROPDOWNB L"dropdnb.lzp"
#define IMG_FILE_CHECKBOXA L"checka.lzp"
#define IMG_FILE_CHECKBOXB L"checkb.lzp"
#define IMG_FILE_CHECKBOXC L"checkc.lzp"
#define IMG_FILE_PROC L"proc.lzp"

#define IMG_FILE_BACKUPA L"backupa.jpg"
#define IMG_FILE_BACKUPB L"backupb.jpg"
#define IMG_FILE_BACKUPC L"backupc.jpg"

#define IMG_FILE_TITLE1 L"title1.jpg"

#define IMG_FILE_RECOVERA L"recovera.jpg"
#define IMG_FILE_RECOVERB L"recoverb.jpg"
#define IMG_FILE_RECOVERC L"recoverc.jpg"

#define IMG_FILE_REMOVEDISK L"usb.lzb"
#define IMG_FILE_HARDDISK L"disk.lzb"

#define IMG_FILE_BEGINBAA L"beginbaa.jpg"
#define IMG_FILE_BEGINBAC L"beginbac.jpg"

#define IMG_FILE_BEGINREA L"beginrea.jpg"
#define IMG_FILE_BEGINREC L"beginrec.jpg"

#define IMG_FILE_COMPLETA L"completa.jpg"
#define IMG_FILE_COMPLETC L"completc.jpg"

#define IMG_FILE_TITLERE L"titlere.jpg"
#define IMG_FILE_TITLEBA L"titleba.jpg"
#define IMG_FILE_TITLECOM L"titlecom.jpg"
#define IMG_FILE_DOWN L"down.jpg"

#define BACKUP_DIR	L"\\system.backup"

#define EFI_OCV_GUID \
  { \
    0x91deb06d, 0x49fa, 0x4d5d, {0xbf, 0x4e, 0xd2, 0x98, 0x6b, 0x96, 0xda, 0x45 } \
  }

PIMAGE_FILE_INFO FindImgFileFromName(CHAR16 *name);


#endif

