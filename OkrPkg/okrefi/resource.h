/**
Copyright (c) 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
**/   

#ifndef RESOURCE_H
#define RESOURCE_H

#include "commondef.h"
#include "window.h"
#include "resource_text_fzlth.h"		//方正兰亭黑简体
#include "resource_text_fzltzh.h"		//方正兰亭中黑简体

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

#define IDB_BUTTON_SHOW	(IDC_USER + 12)	//Disk list

#define IDB_RECOVER_BTN	(IDC_USER + 13)
#define IDB_EDIT_BTN	(IDC_USER + 14)

#define IDB_PAGE_PRE 	(IDC_USER + 15)
#define IDB_PAGE_NEXT 	(IDC_USER + 16)

////////////////////////////////////////////////////////////////
#define IDB_ABOUT		(IDC_USER + 17)

#define IDB_REBOOT		(IDC_USER + 18)
#define IDB_SHUTDOWN	(IDC_USER + 19)
#define IDB_MSG_CLOSE	(IDC_USER + 20)

#define IDB_MSG_BTN_INTRODUCE			(IDC_USER + 21)
#define IDB_MSG_BTN_OPEN_SOURCE			(IDC_USER + 22)
#define IDB_MSG_BTN_VERSION				(IDC_USER + 23)
#define IDB_MSG_BTN_FORCE_RECOVERY		(IDC_USER + 24)

#define IDOKSUB							(IDC_USER + 25)
#define IDCANCELMAIN					(IDC_USER + 26)

#define IDB_NEW_HDD_RECOVER				(IDC_USER + 27)

#define IDB_CONTINUE					(IDC_USER + 28)
#define IDB_RUN_RESTORE					(IDC_USER + 29)
#define IDB_AUTO_CONTINUE				(IDC_USER + 30)
#define IDB_AUTO_REBOOT					(IDC_USER + 31)


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
//#define IMG_FILE_BACKUP_TITLE		102	// L"bktitle.lzb"
//#define IMG_FILE_RESTORE_TITLE		103	// L"rstitle.lzb"
#define IMG_FILE_GOBACK 			104	//L"goback.lzb"
#define IMG_FILE_FOOT 				105	//L"foot.lzb"

//#define IMG_FILE_BTN_FOCUS 			106	//L"btnfocus.lzb"
#define IMG_FILE_BTN_NORMAL 		107	//L"btn.jpg"

#define IMG_FILE_MOUSEA				108	// L"mousea.lzp"
#define IMG_FILE_CURSORTEXT			109	// L"curstext.lzp"
#define IMG_FILE_CURSORMOVE			110	// L"cursmove.lzp"
#define IMG_FILE_CURSORHAND			111	// L"curshand.lzp"

#define IMG_FILE_CANCELA			112	// L"cancela.lzp"
//#define IMG_FILE_UPARROWA			113	// L"uparrowa.lzj"
//#define IMG_FILE_UPARROWB			114	// L"uparrowb.lzp"
//#define IMG_FILE_DOWNARROWA			115	// L"dnarrowa.lzj"
//#define IMG_FILE_DOWNARROWB			116	// L"dnarrowb.lzp"
//#define IMG_FILE_RADIOA				117	// L"radioa.lzp"
//#define IMG_FILE_RADIOB				118	// L"radiob.lzp"
//#define IMG_FILE_RADIOC				119	// L"radioc.lzp"
//#define IMG_FILE_DROPDOWNA			120	// L"dropdna.lzp"
//#define IMG_FILE_DROPDOWNB			121	// L"dropdnb.lzp"
//#define IMG_FILE_CHECKBOXA			122	// L"checka.lzp"
//#define IMG_FILE_CHECKBOXB			123	// L"checkb.lzp"
//#define IMG_FILE_CHECKBOXC			124	// L"checkc.lzp"
#define IMG_FILE_PROC				125	// L"proc.lzp"

//#define IMG_FILE_FOOT_BACKUP		126
//#define IMG_FILE_FOOT_RESTORE		127
//#define IMG_FILE_BACKUPING			128	//L"bking.lzb"
//#define IMG_FILE_RESTORING			129	//L"rsting.lzb"
#define IMG_FILE_REMOVEDISK			130	//L"usb.lzb"
#define IMG_FILE_HARDDISK			131	//L"disk.lzb"

//#define IMG_FILE_COMPLETE			132	// L"complete.lzb"
//#define IMG_FILE_BACKUP_COMPLETE	133	// L"bkend.lzb"
//#define IMG_FILE_RESTORE_COMPLETE	134	// L"rstend.lzb"
//#define IMG_FILE_DOWN				135	// L"down.jpg"
//#define IMG_FILE_PERCENT			136	// L"percent.lzb"
//
//#define IMG_FILE_0					140	// L"0.lzb"
//#define IMG_FILE_1					141	// L"1.lzb"
//#define IMG_FILE_2					142	// L"2.lzb"
//#define IMG_FILE_3					143	// L"3.lzb"
//#define IMG_FILE_4					144 // L"4.lzb"
//#define IMG_FILE_5					145	// L"5.lzb"
//#define IMG_FILE_6					146	// L"6.lzb"
//#define IMG_FILE_7					147	// L"7.lzb"
//#define IMG_FILE_8					148	// L"8.lzb"
//#define IMG_FILE_9					149	// L"9.lzb"

//to do modify
#define IMG_FILE_BTN_BACKUP_NORMAL	150		//btnbackup.lzb
#define IMG_FILE_BTN_BACKUP_FOCUS	151		//btnbackupA.lzb
#define IMG_FILE_BTN_RESTORE_NORMAL	152		//btnrestore.lzb
#define IMG_FILE_BTN_RESTORE_FOCUS	153		//btnrestoreA.lzb
#define IMG_FILE_BTN_BACKUP_PRESS	226
#define IMG_FILE_BTN_RESTORE_PRESS	227
//to do modify end


#define IMG_FILE_BTN_BACKUP1_NORMAL		154		//btnbackup1.lzb
#define IMG_FILE_BTN_BACKUP1_FOCUS		155		//btnbackup1A.lzb
#define IMG_FILE_BTN_RESTORE1_NORMAL	156		//btnrestore1.lzb
#define IMG_FILE_BTN_RESTORE1_FOCUS		157		//btnrestore1A.lzb

#define IMG_FILE_BTN_DELETE_NORMAL		158		//btndelete.lzb
#define IMG_FILE_BTN_DELETE_FOCUS		159		//btndeleteA.lzb
#define IMG_FILE_BTN_DELETE_DISABLE		160		//btndeletedisable.lzb


//#define IMG_FILE_BUTTON_TEST		      161
//#define IMG_FILE_BUTTON_TEST_FOCUS		162

#define IMG_FILE_BUTTON_CONFIRM_FOCUS		163	//btnconfirm.lzb
//#define IMG_FILE_BUTTON_GO_BACK_FOCUS		164	//btngoback.lzb


//#define IMAG_FILE_TITLE_LENOVO_DRV_INSTALL  165
//#define IMAG_FILE_TITLE_DRV_INSTALL		166

//#define IMAG_FILE_BK_DAI_SETTINGS		167
//#define IMAG_FILE_BK_DAI_FINISH		168
//#define IMAG_FILE_BK_DAI_ERROR		169
//#define IMAG_FILE_BK_DAI_COPYING	170
//#define IMAG_FILE_BK_COMPLETE_FOCUS	171		//bk_complete2.lzb


#define IMAG_FILE_INITIAL_BK  	172			//initial_bk.lzb
//#define IMAG_FILE_INITIAL_BK_SPACE	173		//initial_bk_space.lzb
//#define IMAG_FILE_INITIAL_BK_TXT	174

//#define IMAG_FILE_EXIT_FOCUS	175
//#define IMAG_FILE_EXIT_NORMAL	176

//#define IMAG_FILE_VERSION_NORMAL 177

//#define IMAG_FILE_BK_DAI_ERROR2 178

#define IMAG_FILE_NEXT 179				//next.lzb
#define IMAG_FILE_NEXT_FOCUS 180		//nextfocus.lzb

#define IMAG_FILE_SYS_BK_TITLE 181		//sysbktitle.lzb
#define IMAG_FILE_SYS_R_TITLE 182		//sysrtitle.lzb

//#define IMAG_FILE_BK_OK_DESC   183
//#define IMAG_FILE_R_OK_DESC 184

//#define IMAG_FILE_BTN_DOWN   185
//#define IMAG_FILE_BTN_DOWN_FOCUS  186
//#define IMAG_FILE_BTN_UP   187
//#define IMAG_FILE_BTN_UP_FOCUS    188
//
//#define IMAG_FILE_COYPY_RIGHT 189
#define IMAG_FILE_GOBACK_FOCUS 190		//goback_focus.lzb

#define IMAG_FILE_CONFIRM_UNFOCUS 191	//confirm_unfocus.lzb

#define IMAG_FILE_BTN_RESTORE_DISABLE   192		//btnrestoredisable.lzb

//#define IMAG_FILE_SYS_R_TIP_SELECT_BACKUP_POINT 193
//#define IMAG_FILE_SYS_R_TIP_CHECK_BACKUP_POINT 194

//#define IMAG_FILE_TIP_USE_TIME 195
//#define IMAG_FILE_TIP_REMAIN_TIME 196

//#define IMAG_FILE_TIP_BACKUP_SELECT_STORE_DEVICE 197
//#define IMAG_FILE_TIP_BACKUP_DATA_SIZE 198
//#define IMAG_FILE_TIP_BACKUP_MEMO 199
//#define IMAG_FILE_TIP_BACKUP_HOT_PNP 200

//#define IMAG_FILE_BK_DAI_FINISH2  201

#define  IMG_FILE_BUTTON_CANCEL_UNFOCUS  202	//btn_cancel_1.lzb
#define  IMG_FILE_BUTTON_CANCEL_FOCUS  203		//btn_cancel_1A.lzb

//#define IMAG_FILE_COYPY_RIGHT_MINI  204

//// add by wh 2018
//#define IMG_NEW_LINCENCE_PAGE_1  205
//#define IMG_NEW_LINCENCE_PAGE_2  206
//#define IMG_NEW_LINCENCE_PAGE_3  207
//#define IMG_NEW_LINCENCE_PAGE_4  208
//#define IMG_NEW_LINCENCE_PAGE_5  209
//#define IMG_NEW_LINCENCE_PAGE_6  210
//#define IMG_NEW_LINCENCE_PAGE_7  211
//#define IMG_NEW_LINCENCE_PAGE_8  212
//#define IMG_NEW_LINCENCE_PAGE_9  213
// this is not resource ID , is the count of License  
//#define NEW_LINCENCE_MAXPAGE  17
//#define IMG_LINCENCE_BACK_01     214
//#define IMG_LINCENCE_BACK_02     215
//#define IMG_LINCENCE_BACK_03     216
//#define IMG_LINCENCE_NEXT_01     217
//#define IMG_LINCENCE_NEXT_02     218
//#define IMG_LINCENCE_NEXT_03     219
//#define IMG_LINCENCE_COMFM_01    220
//#define IMG_LINCENCE_COMFM_02    221
//#define IMG_LINCENCE_CLOSE_01    222
//#define IMG_LINCENCE_CLOSE_02    223

// add 2020
#define IMG_BUTTON_FORCE_RECOVERY_FOCUS		224
#define IMG_BUTTON_FORCE_RECOVERY_UNFOCUS	225



#define IMG_FILE_BTN_ABOUT_UNFOCUS	228
#define IMG_FILE_BTN_ABOUT_FOCUS	229
#define IMG_FILE_BTN_ABOUT_PRESS	230

#define IMG_FILE_BTN_EXIT_UNFOCUS	231
#define IMG_FILE_BTN_EXIT_FOCUS		232
#define IMG_FILE_BTN_EXIT_PRESS		233

#define IMG_FILE_LOGO				234

#define IMG_FILE_ENQUIRE			235
#define IMG_FILE_INFO				236
#define IMG_FILE_WARNING			237

#define IMG_FILE_MSG_CLOSE_UNFOCUS	238
#define IMG_FILE_MSG_CLOSE_FOCUS	239
#define IMG_FILE_MSG_CLOSE_PRESS	240

#define IMG_FILE_MSG_SHUTDOWN_UNFOCUS	241
#define IMG_FILE_MSG_SHUTDOWN_FOCUS		242
#define IMG_FILE_MSG_SHUTDOWN_PRESS		243

#define IMG_FILE_MSG_REBOOT_UNFOCUS		244
#define IMG_FILE_MSG_REBOOT_FOCUS		245
#define IMG_FILE_MSG_REBOOT_PRESS		246

#define IMG_FILE_BUTTON_CONFIRM_PRESS		247
#define IMG_FILE_BUTTON_GO_BACK_PRESS		248
#define IMG_FILE_BUTTON_CANCEL_PRESS		249

#define IMG_FILE_BTN_BACKUP1_PRESS			250
#define IMG_FILE_BTN_RESTORE1_PRESS			251

#define IMG_FILE_SUCCESS					252

//#define IMG_FILE_PAGE_BG					253

#define IMG_FILE_WHITE_BG_LEFT_TOP			254
#define IMG_FILE_WHITE_BG_LEFT_BOTTOM		255
#define IMG_FILE_WHITE_BG_RIGHT_TOP			256
#define IMG_FILE_WHITE_BG_RIGHT_BOTTOM		257

#define IMG_FILE_BTN_DELETE_PRESS			258
#define IMAG_FILE_NEXT_PRESS				259
#define IMAG_FILE_APP_LOGO					260

//#define IMAG_FILE_ABOUT_INTRODUCE			261
//#define IMAG_FILE_ABOUT_INTRODUCE_FOCUS		262
//#define IMAG_FILE_ABOUT_OPEN_SOU			263
//#define IMAG_FILE_ABOUT_OPEN_SOU_FOCUS		264
//#define IMAG_FILE_ABOUT_VER					265
//#define IMAG_FILE_ABOUT_VER_FOCUS			266

#define IMAG_FILE_DISK_BG_NORMAL			267
#define IMAG_FILE_DISK_BG_SELECT			268

#define IMG_FILE_REMOVEDISK_SEL				269
#define IMG_FILE_HARDDISK_SEL				270

#define IMG_FILE_PROGRESS_HEAD				271
#define IMG_FILE_PROGRESS_TAIL				272
#define IMG_FILE_PROGRESS_FRAME_HEAD		273
#define IMG_FILE_PROGRESS_FRAME_TAIL		274

#define IMG_FILE_BACKUP_POINT				275
#define IMG_FILE_BACKUP_POINT_SEL			276
#define IMG_FILE_LOCK						277
#define IMG_FILE_LOCK_SEL					278
#define IMG_FILE_NEW_DISK					279
#define IMG_FILE_NEW_DISK_SEL				280
#define IMG_FILE_ORI_DISK					281
#define IMG_FILE_ORI_DISK_SEL				282

#define IMG_BUTTON_FORCE_RECOVERY_FRESS		283
#define IMG_FILE_FAILED						284

////Small
#define IMG_FILE_BTN_ABOUT_S_UNFOCUS	285
#define IMG_FILE_BTN_ABOUT_S_FOCUS		286
#define IMG_FILE_BTN_ABOUT_S_PRESS		287

#define  IMG_FILE_BUTTON_CANCEL_S_UNFOCUS  288		//btn_cancel_1_s.bmp
#define  IMG_FILE_BUTTON_CANCEL_S_FOCUS	 289
#define	 IMG_FILE_BUTTON_CANCEL_S_PRESS	 290

#define IMG_FILE_BTN_DELETE_S_NORMAL		291		//btndelete_s.bmp
#define IMG_FILE_BTN_DELETE_S_FOCUS			292
#define IMG_FILE_BTN_DELETE_S_DISABLE		293
#define IMG_FILE_BTN_DELETE_S_PRESS			294

#define IMG_FILE_ENQUIRE_S			295		//enquire_s.bmp
#define IMG_FILE_INFO_S				296		//info_s.bmp
#define IMG_FILE_WARNING_S			297

#define IMG_FILE_BTN_EXIT_S_UNFOCUS		298		//exitUnfocus_s.bmp
#define IMG_FILE_BTN_EXIT_S_FOCUS		299
#define IMG_FILE_BTN_EXIT_S_PRESS		300

#define IMG_FILE_BUTTON_GO_BACK_S_UNFOCUS		301		//msgCloseFocus_s.bmp
#define IMG_FILE_BUTTON_GO_BACK_S_FOCUS		302
#define IMG_FILE_BUTTON_GO_BACK_S_PRESS		303

#define IMAG_FILE_INITIAL_BK_S  	304		//initial_bk_s.bmp

#define IMG_FILE_MSG_CLOSE_S_UNFOCUS	305		//msgCloseUnfocus_s.bmp
#define IMG_FILE_MSG_CLOSE_S_FOCUS		306
#define IMG_FILE_MSG_CLOSE_S_PRESS		307

#define IMAG_FILE_NEXT_S			308		//next_s.bmp
#define IMAG_FILE_NEXT_S_FOCUS		309
#define IMAG_FILE_NEXT_PRESS_S		310

#define IMG_FILE_MSG_SHUTDOWN_S_UNFOCUS		311		//shutdownUnfocus_s.bmp
#define IMG_FILE_MSG_SHUTDOWN_S_FOCUS		312
#define IMG_FILE_MSG_SHUTDOWN_S_PRESS		313

#define IMG_FILE_SUCCESS_S					314
#define IMG_FILE_FAILED_S					315

#define  IMG_FILE_BUTTON_CANCEL_MAIN_UNFOCUS	316
#define  IMG_FILE_BUTTON_CANCEL_MAIN_FOCUS		317
#define	 IMG_FILE_BUTTON_CANCEL_MAIN_PRESS		318

#define  IMG_FILE_BUTTON_CANCEL_MAIN_S_UNFOCUS  319		
#define  IMG_FILE_BUTTON_CANCEL_MAIN_S_FOCUS	320
#define	 IMG_FILE_BUTTON_CANCEL_MAIN_S_PRESS	321

#define  IMG_FILE_BUTTON_OK_SUB_UNFOCUS  322
#define  IMG_FILE_BUTTON_OK_SUB_FOCUS	 323
#define	 IMG_FILE_BUTTON_OK_SUB_PRESS	 324

#define  IMG_FILE_BUTTON_OK_SUB_S_UNFOCUS	325
#define  IMG_FILE_BUTTON_OK_SUB_S_FOCUS		326
#define	 IMG_FILE_BUTTON_OK_SUB_S_PRESS		327


#define  IMG_FILE_BTN_BACKUP_NORMAL_S		331
#define  IMG_FILE_BTN_BACKUP_FOCUS_S		332
#define	 IMG_FILE_BTN_BACKUP_PRESS_S		333

#define  IMG_FILE_BTN_RESTORE_NORMAL_S		334
#define  IMG_FILE_BTN_RESTORE_FOCUS_S		335
#define	 IMG_FILE_BTN_RESTORE_PRESS_S		336

#define IMG_FILE_BACKGROUND_TITLE_S			337

#define IMG_FILE_LOGO_S						338

//#define IMAG_FILE_ABOUT_INTRODUCE_S			339
//#define IMAG_FILE_ABOUT_INTRODUCE_FOCUS_S	340
//#define IMAG_FILE_ABOUT_OPEN_SOU_S			341
//#define IMAG_FILE_ABOUT_OPEN_SOU_FOCUS_S	342
//#define IMAG_FILE_ABOUT_VER_S				343
//#define IMAG_FILE_ABOUT_VER_FOCUS_S			344

#define IMG_FILE_BACKUP_POINT_S				345
#define IMG_FILE_BACKUP_POINT_SEL_S			346

#define IMG_FILE_HARDDISK_S					347
#define IMG_FILE_HARDDISK_SEL_S				348

#define IMG_FILE_FOOT_S						349

#define IMG_FILE_WHITE_BG_LEFT_TOP_S		350
#define IMG_FILE_WHITE_BG_LEFT_BOTTOM_S		351
#define IMG_FILE_WHITE_BG_RIGHT_TOP_S		352
#define IMG_FILE_WHITE_BG_RIGHT_BOTTOM_S	353

#define IMG_FILE_LOCK_S						354
#define IMG_FILE_LOCK_SEL_S					355

#define IMG_FILE_NEW_DISK_S					356
#define IMG_FILE_NEW_DISK_SEL_S				357
#define IMG_FILE_ORI_DISK_S					358
#define IMG_FILE_ORI_DISK_SEL_S				359

#define IMG_FILE_PROGRESS_HEAD_S			360
#define IMG_FILE_PROGRESS_TAIL_S			361
#define IMG_FILE_PROGRESS_FRAME_HEAD_S		362
#define IMG_FILE_PROGRESS_FRAME_TAIL_S		363

#define IMAG_FILE_SYS_BK_TITLE_S			364
#define IMAG_FILE_SYS_R_TITLE_S				365

#define IMG_FILE_REMOVEDISK_S				366
#define IMG_FILE_REMOVEDISK_SEL_S			367


#define IMG_FILE_BTNSPL_L					368
#define IMG_FILE_BTNSPL_M					369
#define IMG_FILE_BTNSPL_R					370

#define IMG_FILE_BTNSPL_L_S					371
#define IMG_FILE_BTNSPL_M_S					372
#define IMG_FILE_BTNSPL_R_S					373

#define IMG_FILE_BTNSPL_F_L					374
#define IMG_FILE_BTNSPL_F_M					375
#define IMG_FILE_BTNSPL_F_R					376

#define IMG_FILE_BTNSPL_P_L					377
#define IMG_FILE_BTNSPL_P_M					378
#define IMG_FILE_BTNSPL_P_R					379

#define IMG_FILE_BTNSPL_F_L_S				380
#define IMG_FILE_BTNSPL_F_M_S				381
#define IMG_FILE_BTNSPL_F_R_S				382

#define IMG_FILE_BTNSPL_P_L_S				383
#define IMG_FILE_BTNSPL_P_M_S				384
#define IMG_FILE_BTNSPL_P_R_S				385

#define IMG_FILE_BTNSPL_D_L					386
#define IMG_FILE_BTNSPL_D_M					387
#define IMG_FILE_BTNSPL_D_R					388
#define IMG_FILE_BTNSPL_D_L_S				389
#define IMG_FILE_BTNSPL_D_M_S				390
#define IMG_FILE_BTNSPL_D_R_S				391
#define IMG_FILE_MOUSEA_4K					392	// L"mousea_4k.lzp"

#define IMG_FILE_HOMEBTN_LT					393
#define IMG_FILE_HOMEBTN_RT					394
#define IMG_FILE_HOMEBTN_LB					395
#define IMG_FILE_HOMEBTN_RB					396
#define IMG_FILE_HOMEBTN_TM					397
#define IMG_FILE_HOMEBTN_BM					398
#define IMG_FILE_HOMEBTN_LM					399
#define IMG_FILE_HOMEBTN_RM					400
#define IMG_FILE_HOMEBTN_ICON_BACKUP		401
#define IMG_FILE_HOMEBTN_ICON_RESTORE		402

#define IMG_FILE_HOMEBTN_LT_H					403
#define IMG_FILE_HOMEBTN_RT_H					404
#define IMG_FILE_HOMEBTN_LB_H					405
#define IMG_FILE_HOMEBTN_RB_H					406
#define IMG_FILE_HOMEBTN_TM_H					407
#define IMG_FILE_HOMEBTN_BM_H					408
#define IMG_FILE_HOMEBTN_LM_H					409
#define IMG_FILE_HOMEBTN_RM_H					410
#define IMG_FILE_HOMEBTN_ICON_BACKUP_H			411
#define IMG_FILE_HOMEBTN_ICON_RESTORE_H			412

#define IMG_FILE_HOMEBTN_LT_P					413
#define IMG_FILE_HOMEBTN_RT_P					414
#define IMG_FILE_HOMEBTN_LB_P					415
#define IMG_FILE_HOMEBTN_RB_P					416
#define IMG_FILE_HOMEBTN_TM_P					417
#define IMG_FILE_HOMEBTN_BM_P					418
#define IMG_FILE_HOMEBTN_LM_P					419
#define IMG_FILE_HOMEBTN_RM_P					420
#define IMG_FILE_HOMEBTN_ICON_BACKUP_P			421
#define IMG_FILE_HOMEBTN_ICON_RESTORE_P			422

#define IMG_FILE_HOMEBTN_LT_S					423
#define IMG_FILE_HOMEBTN_RT_S					424
#define IMG_FILE_HOMEBTN_LB_S					425
#define IMG_FILE_HOMEBTN_RB_S					426
#define IMG_FILE_HOMEBTN_TM_S					427
#define IMG_FILE_HOMEBTN_BM_S					428
#define IMG_FILE_HOMEBTN_LM_S					429
#define IMG_FILE_HOMEBTN_RM_S					430
#define IMG_FILE_HOMEBTN_ICON_BACKUP_S			431
#define IMG_FILE_HOMEBTN_ICON_RESTORE_S			432

#define IMG_FILE_HOMEBTN_LT_H_S					433
#define IMG_FILE_HOMEBTN_RT_H_S					434
#define IMG_FILE_HOMEBTN_LB_H_S					435
#define IMG_FILE_HOMEBTN_RB_H_S					436
#define IMG_FILE_HOMEBTN_TM_H_S					437
#define IMG_FILE_HOMEBTN_BM_H_S					438
#define IMG_FILE_HOMEBTN_LM_H_S					439
#define IMG_FILE_HOMEBTN_RM_H_S					440
#define IMG_FILE_HOMEBTN_ICON_BACKUP_H_S		441
#define IMG_FILE_HOMEBTN_ICON_RESTORE_H_S		442

#define IMG_FILE_HOMEBTN_LT_P_S					443
#define IMG_FILE_HOMEBTN_RT_P_S					444
#define IMG_FILE_HOMEBTN_LB_P_S					445
#define IMG_FILE_HOMEBTN_RB_P_S					446
#define IMG_FILE_HOMEBTN_TM_P_S					447
#define IMG_FILE_HOMEBTN_BM_P_S					448
#define IMG_FILE_HOMEBTN_LM_P_S					449
#define IMG_FILE_HOMEBTN_RM_P_S					450
#define IMG_FILE_HOMEBTN_ICON_BACKUP_P_S		451
#define IMG_FILE_HOMEBTN_ICON_RESTORE_P_S		452

////////////////////middle///////////////////////////////////////////
#define IMG_FILE_BTN_ABOUT_M_UNFOCUS	500
#define IMG_FILE_BTN_ABOUT_M_FOCUS		501
#define IMG_FILE_BTN_ABOUT_M_PRESS		502

#define IMG_FILE_ENQUIRE_M			503		//enquire_s.bmp
#define IMG_FILE_INFO_M				504		//info_s.bmp
#define IMG_FILE_WARNING_M			505

#define IMG_FILE_BTN_EXIT_M_UNFOCUS		506		//exitUnfocus_s.bmp
#define IMG_FILE_BTN_EXIT_M_FOCUS		507
#define IMG_FILE_BTN_EXIT_M_PRESS		508

#define IMG_FILE_SUCCESS_M					509
#define IMG_FILE_FAILED_M					510

#define IMG_FILE_FOOT_M						511

#define IMG_FILE_BUTTON_GO_BACK_M_UNFOCUS		512		//msgCloseFocus_s.bmp
#define IMG_FILE_BUTTON_GO_BACK_M_FOCUS		513
#define IMG_FILE_BUTTON_GO_BACK_M_PRESS		514

#define IMAG_FILE_INITIAL_BK_M  	515		//initial_bk_M.bmp

#define IMG_FILE_WHITE_BG_LEFT_TOP_M		516
#define IMG_FILE_WHITE_BG_LEFT_BOTTOM_M		517
#define IMG_FILE_WHITE_BG_RIGHT_TOP_M		518
#define IMG_FILE_WHITE_BG_RIGHT_BOTTOM_M	519

#define IMG_FILE_LOGO_M						520

#define IMG_FILE_MSG_CLOSE_M_UNFOCUS	521		//msgCloseUnfocus_M.bmp
#define IMG_FILE_MSG_CLOSE_M_FOCUS		522
#define IMG_FILE_MSG_CLOSE_M_PRESS		523


#define IMG_FILE_PROGRESS_HEAD_M			524
#define IMG_FILE_PROGRESS_TAIL_M			525
#define IMG_FILE_PROGRESS_FRAME_HEAD_M		526
#define IMG_FILE_PROGRESS_FRAME_TAIL_M		527

#define IMG_FILE_BACKGROUND_TITLE_M			528
#define IMAG_FILE_SYS_BK_TITLE_M			529
#define IMAG_FILE_SYS_R_TITLE_M				530
//
//#define IMG_FILE_BACKUP_POINT_M				531
//#define IMG_FILE_BACKUP_POINT_SEL_M			532
//
//#define IMG_FILE_HARDDISK_M					533
//#define IMG_FILE_HARDDISK_SEL_M				534
//
//#define IMG_FILE_LOCK_M						535
//#define IMG_FILE_LOCK_SEL_M					536
//
//#define IMG_FILE_NEW_DISK_M					537
//#define IMG_FILE_NEW_DISK_SEL_M				538
//#define IMG_FILE_ORI_DISK_M					539
//#define IMG_FILE_ORI_DISK_SEL_M				540
//
//#define IMG_FILE_REMOVEDISK_M				541
//#define IMG_FILE_REMOVEDISK_SEL_M			542

#define IMG_FILE_BTNSPL_L_M					543
#define IMG_FILE_BTNSPL_M_M					544
#define IMG_FILE_BTNSPL_R_M					545

#define IMG_FILE_BTNSPL_F_L_M				546
#define IMG_FILE_BTNSPL_F_M_M				547
#define IMG_FILE_BTNSPL_F_R_M				548

#define IMG_FILE_BTNSPL_P_L_M				549
#define IMG_FILE_BTNSPL_P_M_M				550
#define IMG_FILE_BTNSPL_P_R_M				551

#define IMG_FILE_BTNSPL_D_L_M				552
#define IMG_FILE_BTNSPL_D_M_M				553
#define IMG_FILE_BTNSPL_D_R_M				554

//#define IMG_FILE_HOMEBTN_LT_M					555
//#define IMG_FILE_HOMEBTN_RT_M					556
//#define IMG_FILE_HOMEBTN_LB_M					557
//#define IMG_FILE_HOMEBTN_RB_M					558
//#define IMG_FILE_HOMEBTN_TM_M					559
//#define IMG_FILE_HOMEBTN_BM_M					560
//#define IMG_FILE_HOMEBTN_LM_M					561
//#define IMG_FILE_HOMEBTN_RM_M					562
//#define IMG_FILE_HOMEBTN_ICON_BACKUP_M			563
//#define IMG_FILE_HOMEBTN_ICON_RESTORE_M			564
//
//#define IMG_FILE_HOMEBTN_LT_H_M					565
//#define IMG_FILE_HOMEBTN_RT_H_M					566
//#define IMG_FILE_HOMEBTN_LB_H_M					567
//#define IMG_FILE_HOMEBTN_RB_H_M					568
//#define IMG_FILE_HOMEBTN_TM_H_M					569
//#define IMG_FILE_HOMEBTN_BM_H_M					570
//#define IMG_FILE_HOMEBTN_LM_H_M					571
//#define IMG_FILE_HOMEBTN_RM_H_M					572
//#define IMG_FILE_HOMEBTN_ICON_BACKUP_H_M		573
//#define IMG_FILE_HOMEBTN_ICON_RESTORE_H_M		574
//
//#define IMG_FILE_HOMEBTN_LT_P_M					575
//#define IMG_FILE_HOMEBTN_RT_P_M					576
//#define IMG_FILE_HOMEBTN_LB_P_M					577
//#define IMG_FILE_HOMEBTN_RB_P_M					578
//#define IMG_FILE_HOMEBTN_TM_P_M					579
//#define IMG_FILE_HOMEBTN_BM_P_M					580
//#define IMG_FILE_HOMEBTN_LM_P_M					581
//#define IMG_FILE_HOMEBTN_RM_P_M					582
//#define IMG_FILE_HOMEBTN_ICON_BACKUP_P_M		583
//#define IMG_FILE_HOMEBTN_ICON_RESTORE_P_M		584

#define IMAG_FILE_APP_LOGO_S				585
#define IMAG_FILE_APP_LOGO_M				586

//////////////////////ss///////////////////////////////////////////
//#define IMG_FILE_BTN_ABOUT_SS_UNFOCUS	600
//#define IMG_FILE_BTN_ABOUT_SS_FOCUS		601
//#define IMG_FILE_BTN_ABOUT_SS_PRESS		602
//
//#define IMG_FILE_ENQUIRE_SS			603		//enquire_s.bmp
//#define IMG_FILE_INFO_SS				604		//info_s.bmp
//#define IMG_FILE_WARNING_SS			605
//
//#define IMG_FILE_BTN_EXIT_SS_UNFOCUS		606		//exitUnfocus_s.bmp
//#define IMG_FILE_BTN_EXIT_SS_FOCUS		607
//#define IMG_FILE_BTN_EXIT_SS_PRESS		608
//
//#define IMG_FILE_SUCCESS_SS					609
//#define IMG_FILE_FAILED_SS					610
//
//#define IMG_FILE_FOOT_SS						611
//
//#define IMG_FILE_BUTTON_GO_BACK_SS_UNFOCUS		612		//msgCloseFocus_s.bmp
//#define IMG_FILE_BUTTON_GO_BACK_SS_FOCUS		613
//#define IMG_FILE_BUTTON_GO_BACK_SS_PRESS		614
//
//#define IMAG_FILE_INITIAL_BK_SS  	615		//initial_bk_SS.bmp
//
//#define IMG_FILE_WHITE_BG_LEFT_TOP_SS		616
//#define IMG_FILE_WHITE_BG_LEFT_BOTTOM_SS		617
//#define IMG_FILE_WHITE_BG_RIGHT_TOP_SS		618
//#define IMG_FILE_WHITE_BG_RIGHT_BOTTOM_SS	619
//
//#define IMG_FILE_LOGO_SS						620
//
//#define IMG_FILE_MSG_CLOSE_SS_UNFOCUS	621		//msgCloseUnfocus_SS.bmp
//#define IMG_FILE_MSG_CLOSE_SS_FOCUS		622
//#define IMG_FILE_MSG_CLOSE_SS_PRESS		623
//
//
//#define IMG_FILE_PROGRESS_HEAD_SS			624
//#define IMG_FILE_PROGRESS_TAIL_SS			625
//#define IMG_FILE_PROGRESS_FRAME_HEAD_SS		626
//#define IMG_FILE_PROGRESS_FRAME_TAIL_SS		627
//
//#define IMG_FILE_BACKGROUND_TITLE_SS			628
//#define IMAG_FILE_SYS_BK_TITLE_SS			629
//#define IMAG_FILE_SYS_R_TITLE_SS				630
////
////#define IMG_FILE_BACKUP_POINT_SS				631
////#define IMG_FILE_BACKUP_POINT_SEL_SS			632
////
////#define IMG_FILE_HARDDISK_SS					633
////#define IMG_FILE_HARDDISK_SEL_SS				634
////
////#define IMG_FILE_LOCK_SS						635
////#define IMG_FILE_LOCK_SEL_SS					636
////
////#define IMG_FILE_NEW_DISK_SS					637
////#define IMG_FILE_NEW_DISK_SEL_SS				638
////#define IMG_FILE_ORI_DISK_SS					639
////#define IMG_FILE_ORI_DISK_SEL_SS				640
////
////#define IMG_FILE_REMOVEDISK_SS				641
////#define IMG_FILE_REMOVEDISK_SEL_SS			642
//
//#define IMG_FILE_BTNSPL_L_SS					643
//#define IMG_FILE_BTNSPL_M_SS					644
//#define IMG_FILE_BTNSPL_R_SS					645
//
//#define IMG_FILE_BTNSPL_F_L_SS				646
//#define IMG_FILE_BTNSPL_F_M_SS				647
//#define IMG_FILE_BTNSPL_F_R_SS				648
//
//#define IMG_FILE_BTNSPL_P_L_SS				649
//#define IMG_FILE_BTNSPL_P_M_SS				650
//#define IMG_FILE_BTNSPL_P_R_SS				651
//
//#define IMG_FILE_BTNSPL_D_L_SS				652
//#define IMG_FILE_BTNSPL_D_M_SS				653
//#define IMG_FILE_BTNSPL_D_R_SS				654
//
////#define IMG_FILE_HOMEBTN_LT_SS					655
////#define IMG_FILE_HOMEBTN_RT_SS					656
////#define IMG_FILE_HOMEBTN_LB_SS					657
////#define IMG_FILE_HOMEBTN_RB_SS					658
////#define IMG_FILE_HOMEBTN_TM_SS					659
////#define IMG_FILE_HOMEBTN_BM_SS					660
////#define IMG_FILE_HOMEBTN_LM_SS					661
////#define IMG_FILE_HOMEBTN_RM_SS					662
////#define IMG_FILE_HOMEBTN_ICON_BACKUP_SS			663
////#define IMG_FILE_HOMEBTN_ICON_RESTORE_SS			664
////
////#define IMG_FILE_HOMEBTN_LT_H_SS					665
////#define IMG_FILE_HOMEBTN_RT_H_SS					666
////#define IMG_FILE_HOMEBTN_LB_H_SS					667
////#define IMG_FILE_HOMEBTN_RB_H_SS					668
////#define IMG_FILE_HOMEBTN_TM_H_SS					669
////#define IMG_FILE_HOMEBTN_BM_H_SS					670
////#define IMG_FILE_HOMEBTN_LM_H_SS					671
////#define IMG_FILE_HOMEBTN_RM_H_SS					672
////#define IMG_FILE_HOMEBTN_ICON_BACKUP_H_SS		673
////#define IMG_FILE_HOMEBTN_ICON_RESTORE_H_SS		674
////
////#define IMG_FILE_HOMEBTN_LT_P_SS					675
////#define IMG_FILE_HOMEBTN_RT_P_SS					676
////#define IMG_FILE_HOMEBTN_LB_P_SS					677
////#define IMG_FILE_HOMEBTN_RB_P_SS					678
////#define IMG_FILE_HOMEBTN_TM_P_SS					679
////#define IMG_FILE_HOMEBTN_BM_P_SS					680
////#define IMG_FILE_HOMEBTN_LM_P_SS					681
////#define IMG_FILE_HOMEBTN_RM_P_SS					682
////#define IMG_FILE_HOMEBTN_ICON_BACKUP_P_SS		683
////#define IMG_FILE_HOMEBTN_ICON_RESTORE_P_SS		684

#define IMG_FILE_FIND_BITLOCKER_KEY		685
#define IMG_FILE_FIND_BITLOCKER_KEY_M	686
#define IMG_FILE_FIND_BITLOCKER_KEY_S	687

////Lenovo part use .okr folder, others use LenovoOKR_Backup folder.
#define BACKUP_DIR	L"\\.okr"
#define BACKUP_DIR_CUSTOMER		L"\\LenovoOKR_Backup"

#define EFI_OCV_GUID \
  { \
    0x91deb06d, 0x49fa, 0x4d5d, {0xbf, 0x4e, 0xd2, 0x98, 0x6b, 0x96, 0xda, 0x45 } \
  }

PIMAGE_FILE_INFO FindImgFileFromName(DWORD fileid);

#endif

