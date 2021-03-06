/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/
#ifndef RESOURCE_H
#define RESOURCE_H

#include "commondef.h"
#include "window.h"

//ID范围在0-65535之间，小于0x100的为系统默认id
#define IDCANCEL		0
#define IDOK			1
#define IDNO			IDCANCEL
#define IDYES			IDOK
#define IDB_BACK		10
#define IDB_NEXT		11
#define IDC_USER		0x100
#define IDB_BACKUP		(IDC_USER + 1)
#define IDB_RECOVER		(IDC_USER + 2)
#define IDB_DELETE		(IDC_USER + 3)
#define IDB_COMPLETE	(IDC_USER + 4)
#define IDB_RECOVER_FACTORY	(IDC_USER + 5)
#define IDB_DRIVER_INSTALL (IDC_USER + 6) // add by wh  IDB_DRV_INS_MSG
#define IDB_IDB_INIT_BACKUP	(IDC_USER + 7)
#define IDB_EXIT	(IDC_USER + 8)
#define IDB_SELECT_PART	(IDC_USER + 9)

#define IDB_PAGE_DOWN 	(IDC_USER + 10)
#define IDB_PAGE_UP 	(IDC_USER + 11)

#define IDB_BUTTON_SHOW	(IDC_USER + 12)

#define IDB_RECOVER_BTN	(IDC_USER + 13)
#define IDB_EDIT_BTN	(IDC_USER + 14)

#define IDB_PAGE_PRE 	(IDC_USER + 15)
#define IDB_PAGE_NEXT 	(IDC_USER + 16)
 
#define STRING_ASCII L" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`~!@#$%^&*()-_+={[]}|\:;',<.>/?"
#define STRING_CHINESE L"卷文档"	//可能用到的汉字
#define STRING_WELCOME L"分区表发生变化，与备份点不一致，无法进行恢复。如仍需恢复，请联系联想售后进行强制恢复！"
#define STRING_TIME L"时间"
#define STRING_MEMO L"系统已恢复完成，为保证系统正常工作，重启后将自动进行磁盘检测，请耐心等待检测完成！"
#define STRING_SERIAL L"No."
#define STRING_SELECTFILE L"请选择备份点进行恢复:"
#define STRING_INPUTNAME L"备份点说明:"
//#define STRING_BACKUP_SIZE L"原始数据大小"
#define STRING_BACKUP_SIZE L"需要备份的数据大小:"

#define STRING_PARTINFO L"容量"
#define STRING_PARTINFO1 L"剩余"
#define STRING_COMPLETE L"点击\"完成\"后重新进入系统。"
#define STRING_CANCEL L"按ESC键取消"
#define STRING_USED_TIME L"已用时间 :"
#define STRING_REMAIN_TIME L"剩余时间 :"
#define STRING_FACTORY_BACKUP L"初始备份点"

#define STRING_FACTORY_RESTORE L"此操作将会把硬盘上全部分区恢复到初始状态,请确认是否继续?"
#define STRING_FACTORY_BACKUP_NOT_FOUND L"没有找到初始备份点, 无法进行恢复。"
//#define STRING_FS_INIT_FAILED L"文件系统载入失败，目标硬盘可能处于休眠状态，请用磁盘检查工具修复设备后再备份"
#define STRING_FS_INIT_FAILED_1 L"目标分区不可用或已损坏，请确认分区处于正常状态，然后重启进入本软件进行"
#define STRING_FS_INIT_FAILED_2 L"备份。如仍无法解决，请尝试使用磁盘检查工具对该分区进行错误检查。"
#define STRING_FS_INIT_FAILED_3 L" "

//#define STRING_SELECTPART L"请选择备份文件所在磁盘"
#define STRING_BACKUPHINT  L"请选择存储设备:"
#define STRING_BACKUPHINT1 L"系统后，需要在此设备上额外占用"
#define STRING_BACKUPHINT2 L"空间。"

#define STRING_FORCE_RESTORE_MODE_ON L"强制恢复模式开启。"
#define STRING_FORCE_RESTORE_MODE_OFF L"强制恢复模式关闭。"
#define STRING_FORCE_RESTORE_HDD  L"请选择要恢复的目标硬盘:"

#define STRING_DISK_TOO_SMALL L"备份的镜像数据超出硬盘大小，无法进行恢复。"
//#define STRING_DRIVE_NO_SUPPORT_FS L"目标硬盘没有可用分区存储备份数据"
#define STRING_DRIVE_NO_SUPPORT_FS L"没有找到可用的服务分区。"

#define STRING_DISK_TOO_SMALL_TO_BACKUP L"目标硬盘没有足够空间，无法进行备份。"
#define STRING_PART_TOO_SMALL_TO_BACKUP L"目标分区没有足够空间，无法进行备份。"
#define STRING_PART_TOO_SMALL L"备份的镜像数据超出硬盘大小，无法进行恢复。"

#define STRING_OEM_PART_TOO_SMALL_TO_BACKUP L"服务分区容量不足，无法创建初始备份。您可以尝试备份到其他分区。"
#define STRING_PART_MIGHT_SMALL L"目标分区可能处于休眠状态或没有足够空间，点击'确定'将取消这次备份。"

#define STRING_DRIVE_MIGHT_SMALL L"目标硬盘可能处于休眠状态或没有足够空间，点击'确定'将取消这次备份。"
#define STRING_PARTITION_CHANGED L"分区表发生变化，与备份点不一致，无法进行恢复。"
#define STRING_HINTPARTTABLE L"是否恢复分区表?"
#define STRING_HINTNOSYSPART L"未检测到有效的系统分区，软件仅支持Windows/Linux系统分区的备份和恢复。"
#define STRING_HINTNOCAP L"选择的分区剩余空间太小。"
#define STRING_YES L"确定"
#define STRING_NO L"取消"
#define STRING_NO_1 L"否"
#define STRING_ERROR_WRITE_FILE L"备份数据无法写入文件，磁盘可能空间不足或被拔出。"
#define STRING_ERROR_READ_FILE L"恢复数据无法读入，磁盘可能被拔出。"

#define STRING_ERROR L"发生错误:%d, 行号:%d。"

#define STRING_BACKUP_USER_BREAK L"您确定要中断当前操作吗？"

#define STRING_USER_BREAK L"您确定要中断当前操作吗？系统分区恢复未完成，将无法启动或不可用!"
#define STRING_DELETE_CONFIRM L"您确定要删除该备份点吗？"
#define STRING_DELETE_FACTORY_CONFIRM L"您确定要删除初始备份点吗？"

//#define STRING_VERSION   L"备份恢复系统 V10.0.0.15"
#define STRING_VERSION     L"检测到您的新磁盘容量为%dGB，小于恢复当前所选系统备份需要的容量:%dGB。请选择容量大于或等于%dGB的新硬盘后再试。"

#define STRING_COPYRIGHT L"Copyright(C) 2013-2020"

#define STRING_CUR_VERSION L"V10.0.0.15"

#define STRING_REBOOT L"是否要退出并重启电脑？"

#define STRING_DRV_EXIST_HINT L"系统C分区下已存在“联想驱动与软件安装.exe”文件，请直接运行。或在Windows系统下删除该文件后再试。"

#define STRING_SELEC_PART_TITLE L"请选择存储分区（仅支持NTFS/FAT32格式）："

#define STRING_RECOVER_LIST_TITILE L"请选择要恢复的备份点："

#define STRING_BK_NUM_DESC1 L"您有"

#define STRING_BK_NUM_DESC2 L"个备份点已损坏无法使用"

#define STRING_REMOVE_DES L"*不支持移动存储设备热插拔，请在开机前插入"

#define STRING_BK_COMPLETE L"备份成功，点击\"完成\"后重新进入系统。"
#define STRING_R_COMPLETE L"恢复成功，点击\"完成\"后重新进入系统。"

#define STRING_R_SYS_HINT L"恢复系统分区时，若选定的备份点之后创建的数据未备份，则会被全部删除。点击“确定”开始恢复。"

#define STRING_NTFS_DELET_ERROR_1 L"      备份点所在分区处于休眠状态，重新登陆Windows系统即可解除休眠，"

#define STRING_NTFS_DELET_ERROR_2 L"      然后重启进入本软件尝试删除。如仍无法解决，请尝试使用Windows"

#define STRING_NTFS_DELET_ERROR_3 L"      系统工具对该分区进行错误检查。"

#define STRING_MOUNT_SYS_ERROR_1 L"系统分区处于休眠状态，请登陆Windows系统后正常关机，然后开机正常"

#define STRING_MOUNT_SYS_ERROR_2 L"进入Windows系统后重启，再按F2进入本软件尝试拷贝。如仍无法解决，"

#define STRING_MOUNT_SYS_ERROR_3 L"请尝试使用Windows系统工具对该分区进行错误检查。"

#define STRING_FIND_SYS_ERROR L"无法识别到系统分区。如果您已安装Windows系统，请使用磁盘检查工具修复系统分区后再试。"

#define STRING_NO_DISK_AVAIBLE_HINT L"目标存储设备上，没有可用作存储备份的分区（仅支持NTFS/FAT32格式）或可用分区剩余空间不足。"

#define STRING_CREATE_DIR_ERROR L"写磁盘错误 ，对该分区进行文件系统错误检查后再试。"
#define STRING_CREATE_DIR_ERROR1 L"修正系统分区大小参数检查错误。"
#define STRING_CREATE_DIR_ERROR2 L"读系统分区大小出错。"
#define STRING_CREATE_DIR_ERROR3 L"修改系统分区大小出错。"
#define STRING_CREATE_DIR_ERROR4 L"修改系统分区大小参数检查错误。"
#define STRING_CREATE_DIR_ERROR5 L"写系统分区表出错。"

#define STRING_PART_MODIFY_ERROR L"要恢复的系统分区超出当前各硬盘的容量大小，无法进行恢复。"

#define STRING_FIND_NEW_DISK_CONFIRM L"检测到有新硬盘，是否恢复到新硬盘?"

#define STRING_RECOVER_NEW_DISK_WARNING L"免责声明：该硬盘为非本机原自带硬盘，恢复过程中将清除硬盘中的数据，并重新划分分区。请先备份该硬盘中的数据后再操作。点击“确定”继续恢复，点击“取消”返回。"

#define STRING_DISK_NOT_SELECTED_SMALL L"无效的硬盘，请重新选择硬盘！"

#define STRING_NOT_SUPPORT_DATA_DISK_TYPE   L"不支持恢复到数据盘，请重新选择硬盘！"
#define STRING_NEW_DISK_SIZE_SMALL         L"备份文件中系统分区大小%d G,,硬盘大小为%d +20 G，至少需要%d G大小的硬盘，请重新选择硬盘！"
#define STRING_DISK_ORG_SYSDISK_NOT_FOUND L"选择的硬盘是原系统盘，无法找到原C盘位置，请重新选择硬盘！"
#define STRING_DISK_UNKNOWN_TYPE L"选择的硬盘类型未知，请重新选择硬盘！"
#define STRING_DISK_ORG_SYSDISK_TOO_SMALL L"选择的硬盘是原系统盘，C盘太小无法恢复，请重新选择硬盘！"
#define STRING_DISK_SAME_SELECTED_DISK L"选择的硬盘是备份文件所在硬盘，无法恢复，请重新选择硬盘！"

#define STRING_C_PART_CHANGE_GOON L"系统盘分区表发生变化，是否继续恢复?"
#define STRING_RECOVER_OLD_DISK_WARNING L"免责声明：恢复过程中将清除硬盘中的数据，请先备份该硬盘中的数据后再操作。点击“确定”继续恢复，点击“取消”返回。"

#define STRING_RECOVER_OLD_DISK_WARNING1 L"新硬盘参数错误！"
#define STRING_RECOVER_OLD_DISK_WARNING2 L"新硬盘1扇区错误！"

#define STRING_RECOVER_NEW_DISK_SUC L"系统已成功恢复到新硬盘！请移除其中一块硬盘后再启动计算机，否则将有一块硬盘在Windows系统中将无法正常识别和加载。"

#define STRING_OLD_DISK_SYSDISK_TOO_SMALL L"原系统盘系统分区太小无法恢复，系统盘大小为%dG，所需空间至少为%dG.可对分区进行空间扩展操作再试!"

#define STRING_PARTITION_CHANGED_FORCE_WARNING L"分区表发生变化，与备份点不一致，是否强制恢复？"

#define STRING_DONOT_SELECT_REMOVEABLE_WARNING L"检测到系统要恢复到移动存储设备，请务必确认是移动硬盘，恢复后可以正常使用。如果是U盘，恢复后无法使用。是否继续?"

#define STRING_PERCENT_USING L"正在调整分区大小 %"
#define STRING_PERCENT_USING_LAST L"正在做最后设置，请稍候。。。"

#define STRING_NVME_TIPS L"检测到系统要恢复到NVMe硬盘，恢复后可能会出现蓝屏现象。如出现该情况，需进入一次安全模式系统可自动修复解决该问题。请多次重启计算机， 在出现【自动修复】页面，选择【高级选项】->【疑难解答】->【高级选项】-> 【启动设置】，点击【重启】选项，重启后选择【4启用安全模式】。是否继续?"

#define STRING_DISK_NOT_FOUND_ERROR L"没有找到原系统分区环境或没有格式化的硬盘环境，无法进行恢复。"
#define STRING_DISK_CHECK_TIPS L"由于恢复的系统分区空间大于备份的镜像分区，首次进入系统将自动进行磁盘检查，请务必等待自检完成，否则可能无法正常进入系统。"

#define STRING_DISK_READ_ERROR_TIPS L"读扇区失败"
#define STRING_DISK_WRITE_ERROR_TIPS L"写扇区失败！"

#define STRING_OPEN_DESC_1 L"3rd party credits: "
#define STRING_OPEN_DESC_2 L"7-Zip Copyright (C) 1999-2017 Igor Pavlov. "
#define STRING_OPEN_DESC_3 L"Under GNU LGPL + unRAR restriction."
#define STRING_OPEN_DESC_4 L"ntfs-3g under GNU GPL restriction."
#define STRING_OPEN_DESC_5 L"(Contact Lenovo support for information on"
#define STRING_OPEN_DESC_6 L"obtaining Open Source code)."



#define STRING_MEDIA_ERROR L"读取该备份文件失败，请检查后再试！"

#define STRING_UUID_ERROR L"当前系统分区发生变化，与选择的备份点系统分区信息不一致，无法进行恢复！请选择其它备份点再试，或联系联想售后进行强制恢复！"
#define STRING_SYS_UUID_ERROR L"当前系统分区与选择的备份点系统分区信息不一致，无法进行恢复！请选择其它备份点再试，或联系联想售后进行强制恢复！"

#define STRING_WAIT_NTFS_MSG   L"正在扫描磁盘分区和备份文件，请稍等....."
//#define STRING_NEW_LINCENCE L"Lenovo UEFI OKR\r\n" \
//L"This application is build using the open source software libraries listed below, including software made publicly available by Lenovo under the GNU General Public License(“GPL”) and / or GNU Library / Lesser General Public License(“LGPL”).You may obtain the corresponding source code for any such open source software licensed under the General Public License and / or the Lesser General Public License(or any other license requiring us to make a written offer to provide corresponding source code to you) from us for a period of three years after our initial shipment of your Lenovo PC at https ://pcsupport.lenovo.com/us/en/downloads/ds502005 and without charge except for the cost of media, shipping, and handling, upon written request to Lenovo. This offer is valid to anyone in receipt of your Lenovo PC. You may provide your request to Lenovo support (information on how to contact Lenovo local support was provided to you with your Lenovo PC).  Please make sure your request includes the MTM (“Machine Type Model”) and the software version associated with the Lenovo UEFI OKR software on your Lenovo PC.\r\n" \
//L"THIS OPEN SOURCE SOFTWARE IS PROVIDED AS-IS, WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND, INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. See the GPL or the LGPL for more information. \r\n" \
//L"These libraries are distributed under associated licenses, acknowledgements and required copyright notices listed below:\r\n" \
//L"=====================================================================\r\n" \
//L"ntfs - 3g_ntfsprogs - 2015.3.14\r\n" \
//L"=====================================================================\r\n" \
//L"Copyright (c) 2007-2014 Jean-Pierre Andre\r\n" \
//L"Copyright(c) 2008 - 2015 Jean - Pierre Andre\r\n" \
//L"Copyright(c) 2000 - 2010 Anton Altaparmakov\r\n" \
//L"Copyright(c) 2002 - 2005 Richard Russon\r\n" \
//L"Copyright(c) 2002 - 2009 Szabolcs Szakacsits\r\n" \
//L"Copyright(c) 2004 - 2007 Yura Pakhuchiy\r\n" \
//L"Copyright(c) 2010 Erik Larsson\r\n" \
//L"Copyright(c) 2009 Martin Bene\r\n" \
//L"Copyright(c) 2006 Hil Liao\r\n" \
//L"Copyright(c) 2008 Bernhard Kaindl\r\n" \
//L"Copyright(c) 2003 Lode Leroy\r\n" \
//L"\r\n" \
//L"\r\n" \
//L"\r\n" \
//L"\r\n" \
//L"\r\n" \
//L"Homepage: https://sourceforge.net/p/ntfs-3g/ntfs-3g/ci/2015.3.14/tree/ \r\n" \
//L"* This program/include file is free software; you can redistribute it and/or\r\n" \
//L"*modify it under the terms of the GNU General Public License as published\r\n" \
//L"* by the Free Software Foundation; either version 2 of the License, or\r\n" \
//L"*(at your option) any later version.\r\n" \
//L"*\r\n" \
//L"* This program / include file is distributed in the hope that it will be\r\n" \
//L"* useful, but WITHOUT ANY WARRANTY; without even the implied warranty\r\n" \
//L"* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the\r\n" \
//L"* GNU General Public License for more details.\r\n" \
//L"*\r\n" \
//L"* You should have received a copy of the GNU General Public License\r\n" \
//L"* along with this program(in the main directory of the NTFS - 3G\r\n" \
//L"* distribution in the file COPYING); if not, write to the Free Software\r\n" \
//L"* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111 - 1307  USA\r\n" \
//L"GNU GENERAL PUBLIC LICENSE\r\n" \
//L"Version 2, June 1991\r\n" \
//L"Copyright(C) 1989, 1991 Free Software Foundation, Inc.\r\n" \
//L"51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA\r\n" \
//L"Everyone is permitted to copy and distribute verbatim copies\r\n" \
//L"of this license document, but changing it is not allowed.\r\n" \
//L"Preamble\r\n" \
//L"The licenses for most software are designed to take away your freedom to share and change it. By contrast, the GNU General Public License is intended to guarantee your freedom to share and change free software--to make sure the software is free for all its users. This General Public License applies to most of the Free Software Foundation's software and to any other program whose authors commit to using it. (Some other Free Software Foundation software is covered by the GNU Lesser General Public License instead.) You can apply it to your programs, too.\r\n" \
//L"When we speak of free software, we are referring to freedom, not price.Our General Public Licenses are designed to make sure that you have the freedom to distribute copies of free software(and charge for this service if you wish), that you receive source code or can get it if you want it, that you can change the software or use pieces of it in new free programs; and that you know you can do these things.\r\n" \
//L"To protect your rights, we need to make restrictions that forbid anyone to deny you these rights or to ask you to surrender the rights.These restrictions translate to certain responsibilities for you if you distribute copies of the software, or if you modify it.\r\n" \
//L"For example, if you distribute copies of such a program, whether gratis or for a fee, you must give the recipients all the rights that you have.You must make sure that they, too, receive or can get the source code.And you must show them these terms so they know their rights.\r\n" \
//L"We protect your rights with two steps : (1) copyright the software, and (2) offer you this license which gives you legal permission to copy, distribute and / or modify the software.\r\n" \
//L"Also, for each author's protection and ours, we want to make certain that everyone understands that there is no warranty for this free software. If the software is modified by someone else and passed on, we want its recipients to know that what they have is not the original, so that any problems introduced by others will not reflect on the original authors' reputations.\r\n" \
//L"Finally, any free program is threatened constantly by software patents.We wish to avoid the danger that redistributors of a free program will individually obtain patent licenses, in effect making the program proprietary.To prevent this, we have made it clear that any patent must be licensed for everyone's free use or not licensed at all.\r\n" \
//L"\r\n" \
//L"The precise terms and conditions for copying, distribution and modification follow.\r\n" \
//L"TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\r\n" \
//L"0. This License applies to any program or other work which contains a notice placed by the copyright holder saying it may be distributed under the terms of this General Public License.The \"Program\", below, refers to any such program or work, and a \"work based on the Program\" means either the Program or any derivative work under copyright law : that is to say, a work containing the Program or a portion of it, either verbatim or with modifications and / or translated into another language. (Hereinafter, translation is included without limitation in the term \"modification\".) Each licensee is addressed as \"you\".\r\n" \
//L"Activities other than copying, distribution and modification are not covered by this License; they are outside its scope.The act of running the Program is not restricted, and the output from the Program is covered only if its contents constitute a work based on the Program(independent of having been made by running the Program).Whether that is true depends on what the Program does.\r\n" \
//L"1. You may copy and distribute verbatim copies of the Program's source code as you receive it, in any medium, provided that you conspicuously and appropriately publish on each copy an appropriate copyright notice and disclaimer of warranty; keep intact all the notices that refer to this License and to the absence of any warranty; and give any other recipients of the Program a copy of this License along with the Program.\r\n" \
//L"You may charge a fee for the physical act of transferring a copy, and you may at your option offer warranty protection in exchange for a fee.\r\n" \
//L"2. You may modify your copy or copies of the Program or any portion of it, thus forming a work based on the Program, and copy and distribute such modifications or work under the terms of Section 1 above, provided that you also meet all of these conditions :\r\n" \
//L"a) You must cause the modified files to carry prominent notices stating that you changed the files and the date of any change.\r\n" \
//L"b) You must cause any work that you distribute or publish, that in whole or in part contains or is derived from the Program or any part thereof, to be licensed as a whole at no charge to all third parties under the terms of this License.\r\n" \
//L"c) If the modified program normally reads commands interactively when run, you must cause it, when started running for such interactive use in the most ordinary way, to print or display an announcement including an appropriate copyright notice and a notice that there is no warranty(or else, saying that you provide a warranty) and that users may redistribute the program under these conditions, and telling the user how to view a copy of this License. (Exception : if the Program itself is interactive but does not normally print such an announcement, your work based on the Program is not required to print an announcement.)\r\n" \
//L"These requirements apply to the modified work as a whole.If identifiable sections of that work are not derived from the Program, and can be reasonably considered independent and separate works in themselves, then this License, and its terms, do not apply to those sections when you distribute them as separate works.But when you distribute the same sections as part of a whole which is a work based on the Program, the distribution of the whole must be on the terms of this License, whose permissions for other licensees extend to the entire whole, and thus to each and every part regardless of who wrote it.\r\n" \
//L"Thus, it is not the intent of this section to claim rights or contest your rights to work written entirely by you; rather, the intent is to exercise the right to control the distribution of derivative or collective works based on the Program.\r\n" \
//L"In addition, mere aggregation of another work not based on the Program with the Program(or with a work based on the Program) on a volume of a storage or distribution medium does not bring the other work under the scope of this License.\r\n" \
//L"3. You may copy and distribute the Program(or a work based on it, under Section 2) in object code or executable form under the terms of Sections 1 and 2 above provided that you also do one of the following :\r\n" \
//L"a) Accompany it with the complete corresponding machine - readable source code, which must be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or ,\r\n" \
//L"b) Accompany it with a written offer, valid for at least three years, to give any third party, for a charge no more than your cost of physically performing source distribution, a complete machine - readable copy of the corresponding source code, to be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or ,\r\n" \
//L"c) Accompany it with the information you received as to the offer to distribute corresponding source code. (This alternative is allowed only for noncommercial distribution and only if you received the program in object code or executable form with such an offer, in accord with Subsection b above.)\r\n" \
//L"The source code for a work means the preferred form of the work for making modifications to it.For an executable work, complete source code means all the source code for all modules it contains, plus any associated interface definition files, plus the scripts used to control compilation and installation of the executable.However, as a special exception, the source code distributed need not include anything that is normally distributed(in either source or binary form) with the major components(compiler, kernel, and so on) of the operating system on which the executable runs, unless that component itself accompanies the executable.\r\n" \
//L"If distribution of executable or object code is made by offering access to copy from a designated place, then offering equivalent access to copy the source code from the same place counts as distribution of the source code, even though third parties are not compelled to copy the source along with the object code.\r\n" \
//L"4. You may not copy, modify, sublicense, or distribute the Program except as expressly provided under this License.Any attempt otherwise to copy, modify, sublicense or distribute the Program is void, and will automatically terminate your rights under this License.However, parties who have received copies, or rights, from you under this License will not have their licenses terminated so long as such parties remain in full compliance.\r\n" \
//L"5. You are not required to accept this License, since you have not signed it.However, nothing else grants you permission to modify or distribute the Program or its derivative works.These actions are prohibited by law if you do not accept this License.Therefore, by modifying or distributing the Program(or any work based on the Program), you indicate your acceptance of this License to do so, and all its terms and conditions for copying, distributing or modifying the Program or works based on it.\r\n" \
//L"6. Each time you redistribute the Program(or any work based on the Program), the recipient automatically receives a license from the original licensor to copy, distribute or modify the Program subject to these terms and conditions.You may not impose any further restrictions on the recipients' exercise of the rights granted herein. You are not responsible for enforcing compliance by third parties to this License.\r\n" \
//L"7. If, as a consequence of a court judgment or allegation of patent infringement or for any other reason(not limited to patent issues), conditions are imposed on you(whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not excuse you from the conditions of this License.If you cannot distribute so as to satisfy simultaneously your obligations under this License and any other pertinent obligations, then as a consequence you may not distribute the Program at all.For example, if a patent license would not permit royalty - free redistribution of the Program by all those who receive copies directly or indirectly through you, then the only way you could satisfy both it and this License would be to refrain entirely from distribution of the Program.\r\n" \
//L"If any portion of this section is held invalid or unenforceable under any particular circumstance, the balance of the section is intended to apply and the section as a whole is intended to apply in other circumstances.\r\n" \
//L"It is not the purpose of this section to induce you to infringe any patents or other property right claims or to contest validity of any such claims; this section has the sole purpose of protecting the integrity of the free software distribution system, which is implemented by public license practices.Many people have made generous contributions to the wide range of software distributed through that system in reliance on consistent application of that system; it is up to the author / donor to decide if he or she is willing to distribute software through any other system and a licensee cannot impose that choice.\r\n" \
//L"This section is intended to make thoroughly clear what is believed to be a consequence of the rest of this License.\r\n" \
//L"8. If the distribution and / or use of the Program is restricted in certain countries either by patents or by copyrighted interfaces, the original copyright holder who places the Program under this License may add an explicit geographical distribution limitation excluding those countries, so that distribution is permitted only in or among countries not thus excluded.In such case, this License incorporates the limitation as if written in the body of this License.\r\n" \
//L"9. The Free Software Foundation may publish revised and / or new versions of the General Public License from time to time.Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns.\r\n" \
//L"Each version is given a distinguishing version number.If the Program specifies a version number of this License which applies to it and \"any later version\", you have the option of following the terms and conditions either of that version or of any later version published by the Free Software Foundation.If the Program does not specify a version number of this License, you may choose any version ever published by the Free Software Foundation.\r\n" \
//L"10. If you wish to incorporate parts of the Program into other free programs whose distribution conditions are different, write to the author to ask for permission.For software which is copyrighted by the Free Software Foundation, write to the Free Software Foundation; we sometimes make exceptions for this.Our decision will be guided by the two goals of preserving the free status of all derivatives of our free software and of promoting the sharing and reuse of software generally.\r\n" \
//L"NO WARRANTY\r\n" \
//L"11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND / OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\r\n" \
//L"12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND / OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM(INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\r\n" \
//L"END OF TERMS AND CONDITIONS\r\n" \
//L"\r\n" \
//L"\r\n" \
//L"=====================================================================\r\n" \
//L"UDK2010.SR1.UP1.P1.Complete.MyWorkSpace\r\n" \
//L"=====================================================================\r\n" \
//L"Copyright(c) 2010 - 2011, Intel Corporation.All rights reserved.<BR>\r\n" \
//L"\r\n" \
//L"This program and the accompanying materials are licensed and made available under the terms and conditions of the BSD License which accompanies this distribution.The full text of the license may be found at\r\n" \
//L"\r\n" \
//L"http://opensource.org/licenses/bsd-license.php\r\n" \
//L"\r\n" \
//L"THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN \"AS IS\" BASIS,\r\n" \
//L"WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.\r\n" \
//L"\r\n" \
//L"BSD License\r\n" \
//L"------------\r\n" \
//L"\r\n" \
//L"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met :\r\n" \
//L"1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\r\n" \
//L"2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and / or other materials provided with the distribution.\r\n" \
//L"3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\r\n" \
//L"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\r\n" \


//#define STRING_INPUTNAME L"Please Enter backup name "
//#define STRING_PARTINFO L"Cap."
//#define STRING_SELECTPART L"Please Select Drive"
//#define STRING_BACKUPHINT L"Begin to backup"
//
#define IMG_FILE_BASE_ID			100
#define IMG_FILE_BACKGROUND_TITLE	101	// L"title.lzb"
#define IMG_FILE_BACKUP_TITLE		102	// L"bktitle.lzb"
#define IMG_FILE_RESTORE_TITLE		103	// L"rstitle.lzb"
#define IMG_FILE_GOBACK 			104	//L"goback.lzb"
#define IMG_FILE_FOOT 				105	//L"foot.lzb"

#define IMG_FILE_BTN_FOCUS 			106	//L"btnfocus.lzb"
#define IMG_FILE_BTN_NORMAL 		107	//L"btn.jpg"

#define IMG_FILE_MOUSEA				108	// L"mousea.lzp"
#define IMG_FILE_CURSORTEXT			109	// L"curstext.lzp"
#define IMG_FILE_CURSORMOVE			110	// L"cursmove.lzp"
#define IMG_FILE_CURSORHAND			111	// L"curshand.lzp"

#define IMG_FILE_CANCELA			112	// L"cancela.lzp"
#define IMG_FILE_UPARROWA			113	// L"uparrowa.jpg"
#define IMG_FILE_UPARROWB			114	// L"uparrowb.lzp"
#define IMG_FILE_DOWNARROWA			115	// L"dnarrowa.jpg"
#define IMG_FILE_DOWNARROWB			116	// L"dnarrowb.lzp"
#define IMG_FILE_RADIOA				117	// L"radioa.lzp"
#define IMG_FILE_RADIOB				118	// L"radiob.lzp"
#define IMG_FILE_RADIOC				119	// L"radioc.lzp"
#define IMG_FILE_DROPDOWNA			120	// L"dropdna.lzp"
#define IMG_FILE_DROPDOWNB			121	// L"dropdnb.lzp"
#define IMG_FILE_CHECKBOXA			122	// L"checka.lzp"
#define IMG_FILE_CHECKBOXB			123	// L"checkb.lzp"
#define IMG_FILE_CHECKBOXC			124	// L"checkc.lzp"
#define IMG_FILE_PROC				125	// L"proc.lzp"

#define IMG_FILE_FOOT_BACKUP		126
#define IMG_FILE_FOOT_RESTORE		127
#define IMG_FILE_BACKUPING			128	//L"bking.lzb"
#define IMG_FILE_RESTORING			129	//L"rsting.lzb"
#define IMG_FILE_REMOVEDISK			130	//L"usb.lzb"
#define IMG_FILE_HARDDISK			131	//L"disk.lzb"

#define IMG_FILE_COMPLETE			132	// L"complete.lzb"
#define IMG_FILE_BACKUP_COMPLETE	133	// L"bkend.lzb"
#define IMG_FILE_RESTORE_COMPLETE	134	// L"rstend.lzb"
#define IMG_FILE_DOWN				135	// L"down.jpg"
#define IMG_FILE_PERCENT			136	// L"percent.lzb"

#define IMG_FILE_0					140	// L"0.lzb"
#define IMG_FILE_1					141	// L"1.lzb"
#define IMG_FILE_2					142	// L"2.lzb"
#define IMG_FILE_3					143	// L"3.lzb"
#define IMG_FILE_4					144 // L"4.lzb"
#define IMG_FILE_5					145	// L"5.lzb"
#define IMG_FILE_6					146	// L"6.lzb"
#define IMG_FILE_7					147	// L"7.lzb"
#define IMG_FILE_8					148	// L"8.lzb"
#define IMG_FILE_9					149	// L"9.lzb"

#define IMG_FILE_BTN_BACKUP_NORMAL	150
#define IMG_FILE_BTN_BACKUP_FOCUS	151
#define IMG_FILE_BTN_RESTORE_NORMAL	152
#define IMG_FILE_BTN_RESTORE_FOCUS	153

#define IMG_FILE_BTN_BACKUP1_NORMAL		154
#define IMG_FILE_BTN_BACKUP1_FOCUS		155
#define IMG_FILE_BTN_RESTORE1_NORMAL	156
#define IMG_FILE_BTN_RESTORE1_FOCUS		157

#define IMG_FILE_BTN_DELETE_NORMAL		158
#define IMG_FILE_BTN_DELETE_FOCUS		159
#define IMG_FILE_BTN_DELETE_DISABLE		160


//#define IMG_FILE_BUTTON_TEST		      161
//#define IMG_FILE_BUTTON_TEST_FOCUS		162

#define IMG_FILE_BUTTON_CONFIRM_FOCUS		163
#define IMG_FILE_BUTTON_GO_BACK_FOCUS		164


//#define IMAG_FILE_TITLE_LENOVO_DRV_INSTALL  165
//#define IMAG_FILE_TITLE_DRV_INSTALL		166

//#define IMAG_FILE_BK_DAI_SETTINGS		167
//#define IMAG_FILE_BK_DAI_FINISH		168
//#define IMAG_FILE_BK_DAI_ERROR		169
//#define IMAG_FILE_BK_DAI_COPYING	170
#define IMAG_FILE_BK_COMPLETE_FOCUS	171


#define IMAG_FILE_INITIAL_BK  	172
#define IMAG_FILE_INITIAL_BK_SPACE	173
#define IMAG_FILE_INITIAL_BK_TXT	174

#define IMAG_FILE_EXIT_FOCUS	175
#define IMAG_FILE_EXIT_NORMAL	176

#define IMAG_FILE_VERSION_NORMAL 177

//#define IMAG_FILE_BK_DAI_ERROR2 178

#define IMAG_FILE_NEXT 179
#define IMAG_FILE_NEXT_FOCUS 180

#define IMAG_FILE_SYS_BK_TITLE 181
#define IMAG_FILE_SYS_R_TITLE 182

#define IMAG_FILE_BK_OK_DESC   183
#define IMAG_FILE_R_OK_DESC 184

#define IMAG_FILE_BTN_DOWN   185
#define IMAG_FILE_BTN_DOWN_FOCUS  186
#define IMAG_FILE_BTN_UP   187
#define IMAG_FILE_BTN_UP_FOCUS    188

#define IMAG_FILE_COYPY_RIGHT 189
#define IMAG_FILE_GOBACK_FOCUS 190

#define IMAG_FILE_CONFIRM_UNFOCUS 191

#define IMAG_FILE_BTN_RESTORE_DISABLE   192

//#define IMAG_FILE_SYS_R_TIP_SELECT_BACKUP_POINT 193
//#define IMAG_FILE_SYS_R_TIP_CHECK_BACKUP_POINT 194

//#define IMAG_FILE_TIP_USE_TIME 195
//#define IMAG_FILE_TIP_REMAIN_TIME 196

//#define IMAG_FILE_TIP_BACKUP_SELECT_STORE_DEVICE 197
//#define IMAG_FILE_TIP_BACKUP_DATA_SIZE 198
//#define IMAG_FILE_TIP_BACKUP_MEMO 199
//#define IMAG_FILE_TIP_BACKUP_HOT_PNP 200

//#define IMAG_FILE_BK_DAI_FINISH2  201

#define  IMG_FILE_BUTTON_CANCEL_FOCUS  202
#define  IMG_FILE_BUTTON_CANCEL_UNFOCUS  203

#define IMAG_FILE_COYPY_RIGHT_MINI  204

// add by wh 2018
#define IMG_NEW_LINCENCE_PAGE_1  205
#define IMG_NEW_LINCENCE_PAGE_2  206
#define IMG_NEW_LINCENCE_PAGE_3  207
#define IMG_NEW_LINCENCE_PAGE_4  208
#define IMG_NEW_LINCENCE_PAGE_5  209
#define IMG_NEW_LINCENCE_PAGE_6  210
#define IMG_NEW_LINCENCE_PAGE_7  211
#define IMG_NEW_LINCENCE_PAGE_8  212
#define IMG_NEW_LINCENCE_PAGE_9  213
// this is not resource ID , is the count of License  
#define NEW_LINCENCE_MAXPAGE  17
#define IMG_LINCENCE_BACK_01     214
#define IMG_LINCENCE_BACK_02     215
#define IMG_LINCENCE_BACK_03     216
#define IMG_LINCENCE_NEXT_01     217
#define IMG_LINCENCE_NEXT_02     218
#define IMG_LINCENCE_NEXT_03     219
#define IMG_LINCENCE_COMFM_01    220
#define IMG_LINCENCE_COMFM_02    221
#define IMG_LINCENCE_CLOSE_01    222
#define IMG_LINCENCE_CLOSE_02    223


#define BACKUP_DIR	L"\\.okr"

#define EFI_OCV_GUID \
  { \
    0x91deb06d, 0x49fa, 0x4d5d, {0xbf, 0x4e, 0xd2, 0x98, 0x6b, 0x96, 0xda, 0x45 } \
  }

PIMAGE_FILE_INFO FindImgFileFromName(DWORD fileid);

#endif

