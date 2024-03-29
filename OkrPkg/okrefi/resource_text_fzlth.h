/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef RESOURCE_TEXT_FZLTH_H
#define RESOURCE_TEXT_FZLTH_H
  

#define STRING_ASCII L" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`~!@#$%^&*()-_+={[]}|\:;',<.>/?\""

//#define STRING_CHINESE L"检测到系统分区已启用BitLocker加密，请在Windows中关闭BitLocker加密后再进行系统恢复。"	
//#define STRING_CHINESE L"卷文档"	//可能用到的汉字
#define STRING_WELCOME L"检测到磁盘分区结构与所选系统备份不一致，无法进行常规恢复！请选择其它系统备份再试。或者可按Ctrl+F组合键开启强制恢复模式，将磁盘分区结构和系统恢复到所选系统备份时状态。"
//#define STRING_TIME L"时间"
//#define STRING_MEMO L"系统已恢复完成，为保证系统正常工作，重启后将自动进行磁盘检测，请耐心等待检测完成！"
//#define STRING_SERIAL L"No."
//#define STRING_SELECTFILE L"请选择备份点进行恢复："
#define STRING_INPUTNAME L"备份名称"
//#define STRING_BACKUP_SIZE L"原始数据大小"
#define STRING_BACKUP_SIZE L"存储备份文件所需空间(大约)："

//#define STRING_PARTINFO L"容量"
//#define STRING_PARTINFO1 L"剩余"
//#define STRING_COMPLETE L"点击\"完成\"后重新进入系统。"

//#define STRING_USED_TIME L"已用时间 ："
#define STRING_REMAIN_TIME L"剩余时间："


#define STRING_FACTORY_RESTORE L"强制恢复将重置磁盘分区并恢复系统到出厂或初始状态，请备份当前磁盘数据后再进行。是否继续？"
#define STRING_FACTORY_BACKUP_NOT_FOUND L"没有找到出厂或初始系统备份，无法进行恢复。"

#define STRING_FS_INIT_FAILED L""
//#define STRING_FS_INIT_FAILED L"文件系统载入失败，目标硬盘可能处于休眠状态，请用磁盘检查工具修复设备后再备份"
#define STRING_FS_INIT_FAILED_1 L"目标分区不可用或已损坏，无法进行备份。请检查并重试。"
//#define STRING_FS_INIT_FAILED_2 L""
//#define STRING_FS_INIT_FAILED_3 L" "

#define STRING_SELECTPART L""

//#define STRING_SELECTPART L"请选择备份文件所在磁盘"

//#define STRING_BACKUPHINT1 L"系统后，需要在此设备上额外占用"
//#define STRING_BACKUPHINT2 L"空间。"

#define STRING_FORCE_RESTORE_MODE_ON L"强制恢复模式：开启"
#define STRING_FORCE_RESTORE_MODE_ON_INFO	 L"在强制恢复模式下进行系统恢复时，系统及磁盘分区结构将恢复为所选系统备份文件所记录的状态。由于会删除磁盘所有数据并重新划分磁盘分区，影响比较大，请谨慎操作。"

#define STRING_FORCE_RESTORE_MODE_OFF L"强制恢复模式：关闭"


//#define STRING_DISK_TOO_SMALL L"备份的镜像数据超出硬盘大小，无法进行恢复。"
//#define STRING_DRIVE_NO_SUPPORT_FS L"目标硬盘没有可用分区存储备份数据"
#define STRING_DRIVE_NO_SUPPORT_FS L"没有找到可用的服务分区。"

#define STRING_DISK_TOO_SMALL_TO_BACKUP L"目标磁盘可用空间不足，无法进行备份。"
#define STRING_PART_TOO_SMALL_TO_BACKUP L"目标分区可用空间不足，无法进行备份。"
#define STRING_NEED_SPACE	 L"（大约需要 %d GB）"

//#define STRING_PART_TOO_SMALL L"备份的镜像数据超出硬盘大小，无法进行恢复。"

#define STRING_OEM_PART_TOO_SMALL_TO_BACKUP L""  //L"服务分区可用空间不足，无法存储初始备份。请选择其他分区或磁盘。"

//TODO
#define STRING_PART_MIGHT_SMALL L"目标分区可能处于休眠状态或没有足够空间，点击“确定”将取消这次备份。"
#define STRING_DRIVE_MIGHT_SMALL L"目标硬盘可能处于休眠状态或没有足够空间，点击“确定”将取消这次备份。"

#define STRING_PARTITION_CHANGED L"检测到磁盘分区结构与所选系统备份不一致，无法进行常规恢复！请选择其它系统备份再试。或者可按Ctrl+F组合键开启强制恢复模式，将磁盘分区结构和系统恢复到所选系统备份时状态。"
#define STRING_HINTPARTTABLE L"是否恢复分区结构？"
#define STRING_HINTNOSYSPART L"未检测到有效的Windows，Linux系统分区。"
//#define STRING_HINTNOCAP L"选择的分区剩余空间太小。"
#define STRING_YES L"确定"
#define STRING_NO L"取消"
#define STRING_NO_1 L"否"


#define STRING_ERROR_WRITE_FILE L"系统备份失败！原因：目标存储空间不足或磁盘被拔出。"
#define STRING_ERROR_READ_FILE L"系统恢复失败！原因：恢复目标磁盘被拔出。"


#define STRING_ERROR L"发生错误：%d，行号：%d。"

#define STRING_BACKUP_USER_BREAK L"您确定要终止当前操作？"

#define STRING_USER_BREAK L"您确定要终止当前操作？当前系统未完全恢复，无法启用及使用！"

#define STRING_DELETE_CONFIRM			L"您确定要删除该系统备份？"
#define STRING_DELETE_FACTORY_CONFIRM	L"您确定要删除初始系统备份？"

//#define STRING_VERSION   L"备份恢复系统 V10.0.0.21"
#define STRING_VERSION     L"新磁盘容量为%dGB，小于恢复当前所选系统备份所需容量：%dGB。请重新选择新磁盘后再试。"

//#define STRING_COPYRIGHT L"Copyright(C) 2013-2020"

#define STRING_CUR_VERSION L"当前版本：V10.2.0.8"

#define STRING_REBOOT L"您确定要重启或关机？"


//#define STRING_DRV_EXIST_HINT L"系统C分区下已存在“联想驱动与软件安装.exe”文件，请直接运行。或在Windows系统下删除该文件后再试。"


#define STRING_BK_NUM_DESC1 L"您有"

#define STRING_BK_NUM_DESC2 L"个备份点已损坏无法使用"

#define STRING_REMOVE_DES L"    不支持移动存储设备热插拔，请在开机前插入"



#define STRING_R_SYS_HINT L"恢复过程中将删除当前系统分区所有数据！\n请对当前系统分区重要数据进行备份。 \n您确定继续恢复？"

#define STRING_NTFS_DELET_ERROR_1 L"系统备份文件所在分区处于休眠状态，请登录一次Windows系统解除休眠状态后再继续。"

//#define STRING_NTFS_DELET_ERROR_2 L""

//#define STRING_NTFS_DELET_ERROR_3 L""

//#define STRING_MOUNT_SYS_ERROR_1 L"系统分区处于休眠状态，请登陆Windows系统后正常关机，然后开机正常"

//#define STRING_MOUNT_SYS_ERROR_2 L"进入Windows系统后重启，再按F2进入本软件尝试拷贝。如仍无法解决，"

//#define STRING_MOUNT_SYS_ERROR_3 L"请尝试使用Windows系统工具对该分区进行错误检查。"

#define STRING_FIND_SYS_ERROR L"找不到系统分区。"

#define STRING_NO_DISK_AVAIBLE_HINT L"没有用于存储系统备份文件的数据分区！"

#define STRING_CREATE_DIR_ERROR L"系统备份失败！ 原因：数据无法写入磁盘，请检查该磁盘状态。"
#define STRING_CREATE_DIR_ERROR1 L"系统恢复失败！原因：修正系统分区大小失败。"
#define STRING_CREATE_DIR_ERROR2 L"系统恢复失败！原因：读系统分区大小失败。"
#define STRING_CREATE_DIR_ERROR3 L"系统恢复失败！原因：修改系统分区大小失败。"
#define STRING_CREATE_DIR_ERROR4 L"系统恢复失败！原因：修改系统分区大小失败。"
#define STRING_CREATE_DIR_ERROR5 L"系统恢复失败！原因：记录系统分区结构失败。"

#define STRING_PART_MODIFY_ERROR L"当前各磁盘容量小于要恢复的系统备份容量要求，无法进行恢复。"

#define STRING_FIND_NEW_DISK_CONFIRM L""

#define STRING_RECOVER_NEW_DISK_WARNING L"该磁盘为新磁盘，恢复过程中将清除该磁盘所有数据，并重置分区结构。请备份该磁盘中的重要数据。是否继续恢复？"

#define STRING_DISK_NOT_SELECTED_SMALL L"无效的磁盘，请重新选择磁盘！"

#define STRING_NOT_SUPPORT_DATA_DISK_TYPE  L""
#define STRING_NEW_DISK_SIZE_SMALL         L"恢复该系统备份需要至少%d GB空间，请选择满足该容量的磁盘！"
#define STRING_DISK_ORG_SYSDISK_NOT_FOUND L"选择的硬盘是原系统盘，无法找到原C盘位置，请重新选择硬盘！"
#define STRING_DISK_UNKNOWN_TYPE L"无法识别该磁盘，请重新选择磁盘！"
#define STRING_DISK_ORG_SYSDISK_TOO_SMALL L"选择的硬盘是原系统盘，C盘太小无法恢复，请重新选择硬盘！"
#define STRING_DISK_SAME_SELECTED_DISK L"选择的硬盘是备份文件所在硬盘，无法恢复，请重新选择硬盘！"

#define STRING_C_PART_CHANGE_GOON L"检测到当前系统分区大小发生变化，是否继续恢复？"
#define STRING_RECOVER_OLD_DISK_WARNING L""

#define STRING_RECOVER_OLD_DISK_WARNING1 L"新硬盘参数错误！"
#define STRING_RECOVER_OLD_DISK_WARNING2 L"新硬盘1扇区错误！"

#define STRING_RECOVER_NEW_DISK_SUC L"系统已成功恢复到新硬盘！请关机并移除原系统磁盘后开机启动。"

//#define STRING_OLD_DISK_SYSDISK_TOO_SMALL L"恢复该系统备份需要至少为%dGB空间，当前系统分区容量为%dGB，无法恢复！可在Windows中扩展当前系统分区容量再试！"

#define STRING_OLD_DISK_SYSDISK_TOO_SMALL L"检测到当前系统分区大小发生变化，且容量小于该系统备份恢复空间要求（大约需要%dGB），无法进行恢复！"

#define STRING_PARTITION_CHANGED_FORCE_WARNING L"当前磁盘分区结构与所选备份文件记录不一致，是否强制恢复？"

#define STRING_DONOT_SELECT_REMOVEABLE_WARNING L"检测所选磁盘为USB移动存储设备，请确认是USB移动硬盘，如是U盘恢复后可能无法使用。是否继续？"

//#define STRING_PERCENT_USING L"正在调整分区大小 %"
//#define STRING_PERCENT_USING_LAST L"正在做最后设置，请稍候。。。"

#define STRING_NVME_TIPS L"检测到系统要恢复到NVMe硬盘，恢复后可能会出现蓝屏现象。如出现该情况，在多次重启计算机后将自动进入安全模式，再次重启即可正常进入Windows。是否继续？"

#define STRING_DISK_NOT_FOUND_ERROR L"没有找到原系统分区环境或没有格式化的硬盘环境，无法进行恢复。"
#define STRING_DISK_CHECK_TIPS L"由于恢复后系统分区结构与原系统分区不一致，恢复后的系统首次启动将自动进行自检，请等待自检完成后即可正常进入系统。"

#define STRING_DISK_READ_ERROR_TIPS L"读扇区失败！"
#define STRING_DISK_WRITE_ERROR_TIPS L"写扇区失败！"

//#define STRING_OPEN_DESC_1 L"3rd party credits: "
//#define STRING_OPEN_DESC_2 L"7-Zip Copyright (C) 1999-2017 Igor Pavlov. "
//#define STRING_OPEN_DESC_3 L"Under GNU LGPL + unRAR restriction."
//#define STRING_OPEN_DESC_4 L"ntfs-3g under GNU GPL restriction."
//#define STRING_OPEN_DESC_5 L"(Contact Lenovo support for information on"
//#define STRING_OPEN_DESC_6 L"obtaining Open Source code)."



#define STRING_MEDIA_ERROR L"读取备份文件失败，请检查后再试！"

#define STRING_UUID_ERROR L"检测到磁盘分区结构与所选系统备份不一致，无法进行常规恢复！请选择其它系统备份再试。或者可按Ctrl+F组合键开启强制恢复模式，将磁盘分区结构和系统恢复到所选系统备份时状态。"
//#define STRING_SYS_UUID_ERROR L"当前系统分区与选择的备份点系统分区信息不一致，无法进行恢复！请选择其它备份点再试，或联系联想售后进行强制恢复！"

#define STRING_WAIT_NTFS_MSG   L"正在扫描磁盘分区和系统备份文件，请稍等..."



////////////////////////////////////Add////////////////////////////////

#define STRING_BITLOCKER_NO_SYSTEM	L"未检测到系统分区！\n检测到存在BitLocker加密分区，请在Windows中关闭BitLocker\n加密后，再进行系统备份。"

#define STRING_BITLOCKER_NO_DATA_PAR  L"未检测到数据存储分区！\n检测到存在BitLocker加密分区，请在Windows中关闭BitLocker加密后，再进行系统备份。"

#define STRING_BITLOCKER_EXIST_BACKUPFILE L"检测到有些磁盘分区启用了BitLocker加密，存储在这些分区的系统备份文件无法访问。请在Windows中关闭BitLocker加密后使用。[关闭BitLocker加密请参看‘关于-软件介绍’内容]"
#define STRING_BITLOCKER_EXIST_BACKUPFILE_1  L"检测到有些磁盘分区启用了BitLocker加密，存储在这些分区的系统备份文件无法访问。请在Windows中关闭BitLocker加密后使用。"
#define STRING_BITLOCKER_EXIST_BACKUPFILE_2  L"[关闭BitLocker加密请参看‘关于-软件介绍’内容]"

//For 1024*768
#define STRING_BITLOCKER_EXIST_BACKUPFILE_1_1  L"检测到有些磁盘分区启用了BitLocker加密，存储在这些分区的系统备份文件无法访问。"
#define STRING_BITLOCKER_EXIST_BACKUPFILE_1_2  L"请在Windows中关闭BitLocker加密后使用。"


#define STRING_BITLOCKER_NO_BACKUPFILE L""

#define STRING_BITLOCKER_SYSTEM_PAR_BOOTPOS_CHANGE   L"系统分区信息不匹配！请选择其它系统备份再试。如需要可强制恢复。强制恢复将重置磁盘到出厂/初始状态。您确认强制恢复？"

#define STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE  L"检测到系统分区大小有变化，且启用了BitLocker加密。请在Windows中关闭BitLocker加密后再试。\n或者可进行强制恢复，将磁盘分区和Windows系统恢复到所选系统备份文件时状态。"

//#define STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_1  L"检测到系统分区大小有变化，且启用了BitLocker加密。"
//#define STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_2  L"请在Windows中关闭BitLocker加密后再试。"
//#define STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_3  L"或者可强制恢复，将磁盘分区和Windows系统恢复到出厂/初始状态。"
//#define STRING_BITLOCKER_SYSTEM_PAR_SIZE_CHANGE_4  L"[关闭BitLocker加密请参看‘关于-软件介绍’内容]"

#define STRING_FORCE_RECOVERY_INFO		L"强制恢复过程中，将删除当前磁盘上的数据和分区，强烈建议您对计算机中的数据进行备份后再进行。是否执行强制恢复？"

#define STRING_BITLOCKER_SELECT_PAR		L"此分区启用了BitLocker加密，无法存储系统备份文件。\n请在Windows中关闭BitLocker加密后，再进行系统备份。"

#define STRING_MAIN_PAGE_INFO	L"按 Enter 键确认，按 Tab 键切换，按 ESC 键返回上一级，按 F1 键查看关于信息。"


#define STRING_FORCE_RECOVERY L"强制恢复"
#define STRING_CLOSE_BTN L"关机"
#define STRING_REBOOT_BTN L"重启"

#define STRING_STAR L"*"

#define STRING_PARTITION_TYPE_INFO L"    分区仅支持NTFS/FAT32格式"
#define STRING_BACKUP_INFO		   L"备份过程中请勿重启、关机或移除存储设备"
#define STRING_RECOVERY_INFO	   L"恢复过程中请勿重启、关机或移除存储设备"
#define STRING_COPYRIGHT		   L"Copyright (C) 2023 Lenovo. All Rights Reserved."

#define STRING_LIST_ITEM_BK_POINT			L"系统备份"
#define STRING_LIST_ITEM_BK_TIME			L"备份时间"
#define STRING_LIST_ITEM_BK_IMG_SIZE		L"备份镜像大小"

///Select bk pos, par list
#define STRING_LIST_ITEM_BK_VOLUME			L"卷标"
#define STRING_LIST_ITEM_BK_TYPE			L"格式"
#define STRING_LIST_ITEM_BK_TOTAL_SIZE		L"总容量"
#define STRING_LIST_ITEM_BK_REMAIN_SIZE		L"剩余空间"

#define STRING_DEFAULT_BK_NAME				L"系统备份-%04d%02d%02d%02d%02d"
#define STRING_DEFAULT_BK_NAME_RAID			L"系统备份-%04d%02d%02d%02d%02d_RAID"
#define STRING_DEFAULT_BK_NAME_1			L""

#define STRING_LIST_ITEM_BK_VOLUME_VALUE_B	L"分区%d （BitLocker加密）"
#define STRING_LIST_ITEM_BK_VOLUME_VALUE	L"分区%d"

#define STRING_LIST_ITEM_DISK				L"硬盘"
#define STRING_LIST_ITEM_TYPE				L"型号"
#define STRING_LIST_ITEM_CAP				L"容量"

#define STRING_DATA_DISK					L"数据硬盘"
#define STRING_SYSTEM_DISK					L"系统硬盘"

#define STRING_ORI_DISK						L"原始硬盘"
#define STRING_NEW_DISK						L"新增硬盘"

///工厂preload工具创建的备份
#define STRING_FACTORY_BACKUP	L"出厂备份"
#define STRING_FACTORY_BACKUP_RAID	L"出厂备份_RAID"
#define STRING_AUTO_FACTORY_BACKUP_RAID L"自动出厂备份_RAID"
#define STRING_AUTO_FACTORY_BACKUP	L"自动出厂备份"
#define STRING_AUTO_BACKUP	L"自动备份"

///无出厂备份用户第一个创建的备份
#define STRING_INIT_BACKUP		L"初始备份"
#define STRING_INIT_BACKUP_RAID		L"初始备份_RAID"

#define STRING_CANCEL  L"按 ESC 键取消"

#define STRING_CAPACITY_GB	 L"容量： %d GB"
#define STRING_CAPACITY_MB	 L"容量： %d MB"


#define STRING_CREATE_INIT_BK	 L"欢迎您创建第一个系统备份！该备份将标记为初始备份，存储在联想服务分区中，不可删除，以备系统异常或崩溃时快速恢复使用。"


#define STRING_ADJUST_SYSTEM_SIZE			L"正在优化系统分区，请稍等..."		//34
#define STRING_SET_CONFIG					L"正在做最后设置，请稍等..."			//34

#define STRING_ABOUT_INTRODUCE				L"软件介绍"			//40
#define STRING_ABOUT_OPEN_SOURCE			L"开源代码声明"		//40
#define STRING_ABOUT_VERSION				L"软件版本信息"		//40


#define STRING_REMAIN_TIME_VALUE		    L"--:--"		//40

#define STRING_BTNTEXT_BACKUP			L"备份"	
#define STRING_HOMEBTNTEXT_BACKUP		L"备份系统"	
#define STRING_HOMEBTNTEXT_RESTORE		L"恢复系统"
#define STRING_BTNTEXT_DELETE			L"删除"	
#define STRING_BTNTEXT_RECOVER			L"恢复"	
#define STRING_BTNTEXT_NEXT				L"下一步"
#define STRING_BTNTEXT_CONTINUE			L"继续恢复"
#define STRING_BTNTEXT_RUN_RESTORE		L"执行恢复"


#define STRING_NO_BACKUP_FILE			L"在当前磁盘及外接USB存储设备中没有找到系统备份镜像文件！"

#define STRING_LOCAL_DISK					L"Local Drive"
#define STRING_REMOVABLE_DISK				L"Removable Drive"

#define	STRING_PREPARE_BACKUP				L"正在准备备份，请稍等..."    //40
#define	STRING_PREPARE_RECOVERY				L"正在准备恢复，请稍等..."    //40

#define	STRING_CALCU_IMAGE_VALUE			L"正在校验备份镜像文件，大约需要几分钟，请稍等..."    //40
#define	STRING_VALID_IMAGE_FILE				L"正在验证备份镜像文件，大约需要几分钟，请稍等..."    //40

#define STRING_MULTI_OS				L"检测到当前电脑存在多个Windows系统，无法进行备份！"

#define STRING_CLOSE_BITLOCKER		L"[关闭BitLocker加密请参看‘关于-软件介绍’内容]"

#define STRING_NO_STORE_OS_PAR		L"    备份文件只能存储在非系统分区上"

#define STRING_RAID_DISK_NO_RAID_BK_RECOVERY_ERROR		L"检测到当前系统磁盘为RAID模式，您所选系统镜像与其不匹配，无法进行恢复！\n请选择名称中有‘RAID’标识的系统备份镜像再试。"
#define STRING_RAID_BK_NO_RAID_DISK_RECOVERY_ERROR		L"您所选的系统镜像为RAID模式，与当前系统磁盘模式不匹配，无法进行恢复！\n请选择名称中无‘RAID’标识的系统备份镜像再试。"
#define STRING_RAID_BK_INFO_NO_MATCH_RECOVERY_ERROR		L"检测到当前系统磁盘为RAID模式，但与所选的系统备份镜像中的RAID信息不一致，无法进行恢复！\n请选择其它名称中有‘RAID’标识的系统备份镜像再试。"

#define STRING_BITLOCKER_DATA_PAR	L"检测到有磁盘/分区启用了BitLocker加密，请在Windows中关闭BitLocker加密后再进行系统恢复操作。"
#define STRING_BITLOCKER_FIND_KEY	L"系统恢复后，之前启用了BitLocker加密的磁盘/分区可能仍处于加密状态，请扫描二维码了解如何找到恢复密钥并进行解密操作。"

//自动备份恢复相关的
#define STRING_AUTOBR_DATAPART_NOTEXIST L"未找到可保存备份镜像文件的数据分区。请在Windows磁盘管理中创建数据分区后再试！\n"
#define STRING_AUTOBR_DATAPART_NOTENOUGH L"检查到数据分区可用空间不足。请在Windows中进行处理并释放空间后再试！\n"
#define STRING_AUTOBR_DATAPART_BITLOCKED L"当前可用数据分区空间不足且有数据分区处于BitLocker加密，请在\nWindows中关闭BitLocker加密并确保可用空间大于{%d GB}后再试！\n"
#define STRING_AUTOBR_IMG_NOTFOUND L"未找到系统备份镜像文件。\n"
#define STRING_AUTOBR_IMG_BITLOCKED L"检测到存储系统自动备份镜像文件的数据分区启用了BitLocker加密，请在Windows中关闭BitLocker加密后再试。"

#define STRING_AUTOBR_BITLOCKER_DATA_PAR	L"检测到有磁盘分区启用了BitLocker加密，请在Windows中关闭后再进行系统恢复操作。\n如果系统已崩溃，可进行强制恢复。"
#define STRING_AUTOBR_BITLOCKER_FIND_KEY	L"执行强制恢复后，之前启用了BitLocker加密的磁盘/分区可能仍处于加密状态，请扫描二维码了解如何找到恢复密钥并进行解密操作。"

#define STRING_AUTOBR_BTNTEXT_CONTINUE			L"强制恢复"
#define STRING_AUTOBR_NO L"重启"

#define STRING_AUTOBR_DISK_NOT_FOUND_ERROR L"没有找到原系统分区环境，无法进行恢复。"

#define STRING_AUTOBR_FORCE_RECOVERY		L"您好，即将对系统进行强制恢复！\n强制恢复过程中将删除当前磁盘所有分区和数据，请务必提前备份重要数据。"

#define STRING_AUTOBR_PARTITION_CHANGED		L"检测到当前磁盘分区结构与准备恢复的自动备份镜像不一致，无法进行自动恢复！请按“F2”键进行常规恢复，选择其它系统备份镜像进行恢复或进行强制恢复！"

//备份文件校验
#define STRING_CALCULATE_CHECK_VALUE_ERROR	L"创建备份文件的校验信息失败，请重新备份。"
#define STRING_VALID_IMAGE_CHECK_VALUE_ERROR	L"备份文件校验失败，请检查确认后再试。"

#define STRING_MULTI_OS_RECOVER				L"检测到当前电脑存在多个Windows系统，无法进行恢复！"

#endif

