rem * 1. 将需要的bmp、jpg、pcx资源文件打包到res.dat里， 然后将res.dat转换  *
rem *    成C语言头文件res.h，和font.h一起包含在vga.c里. *
rem * 2. 在本批处理中资源文件res.dat的构造顺序需要和resource.c里的         *
rem *    g_Img_file_list保持一致. 构造res.dat是简单的用copy命令构造的.     *

if not exist res\lz md res\lz 
del res\lz\*.* /q
if not exist res\lz pause
pause
rem call makefont.bat

makebmp  res\title.bmp  res\lz\title.lzb
if ERRORLEVEL 1 goto error
makebmp  res\bktitle.bmp  res\lz\bktitle.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\rstitle.bmp  res\lz\rstitle.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\goback.bmp  res\lz\goback.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\foot.bmp  res\lz\foot.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\backupfoot.bmp  res\lz\backupfoot.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\restorefoot.bmp  res\lz\restorefoot.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\btnfocus.bmp  res\lz\btnfocus.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\btn.bmp  res\lz\btn.lzb
if ERRORLEVEL 1 goto error

makebmp  res\btnbackup.bmp  res\lz\btnbackup.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btnbackupA.bmp  res\lz\btnbackupA.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btnrestore.bmp  res\lz\btnrestore.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btnrestoreA.bmp  res\lz\btnrestoreA.lzb
if ERRORLEVEL 1 goto error

makebmp  res\btnbackup1.bmp  res\lz\btnbackup1.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btnbackup1A.bmp  res\lz\btnbackup1A.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btnrestore1.bmp  res\lz\btnrestore1.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btnrestore1A.bmp  res\lz\btnrestore1A.lzb
if ERRORLEVEL 1 goto error

makebmp  res\btndelete.bmp  res\lz\btndelete.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btndeleteA.bmp  res\lz\btndeleteA.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btndeletedisable.bmp  res\lz\btndeletedisable.lzb
if ERRORLEVEL 1 goto error

makebmp res\mousea.pcx   res\lz\mousea.lzp
if ERRORLEVEL 1 goto error
makebmp res\curstext.pcx res\lz\curstext.lzp
if ERRORLEVEL 1 goto error
makebmp res\cursmove.pcx res\lz\cursmove.lzp
if ERRORLEVEL 1 goto error
makebmp res\curshand.pcx res\lz\curshand.lzp
if ERRORLEVEL 1 goto error
makebmp res\cancela.pcx  res\lz\cancela.lzp
if ERRORLEVEL 1 goto error

makebmp res\uparrowa.jpg  res\lz\uparrowa.lzj
if ERRORLEVEL 1 goto error
makebmp res\uparrowb.pcx  res\lz\uparrowb.lzp
if ERRORLEVEL 1 goto error
makebmp res\dnarrowa.jpg  res\lz\dnarrowa.lzj
if ERRORLEVEL 1 goto error
makebmp res\dnarrowb.pcx  res\lz\dnarrowb.lzp
if ERRORLEVEL 1 goto error
makebmp res\proc.pcx  res\lz\proc.lzp
if ERRORLEVEL 1 goto error

makebmp  res\bking.bmp  res\lz\bking.lzb	
if ERRORLEVEL 1 goto error
makebmp  res\rsting.bmp  res\lz\rsting.lzb
if ERRORLEVEL 1 goto error
makebmp  res\usb.bmp  res\lz\usb.lzb
if ERRORLEVEL 1 goto error
makebmp  res\disk.bmp  res\lz\disk.lzb
if ERRORLEVEL 1 goto error
    
makebmp  res\complete.bmp  res\lz\complete.lzb
if ERRORLEVEL 1 goto error
makebmp  res\bkend.bmp  res\lz\bkend.lzb
if ERRORLEVEL 1 goto error
makebmp  res\rstend.bmp  res\lz\rstend.lzb
if ERRORLEVEL 1 goto error
makebmp  res\down.jpg  res\lz\down.lzj
if ERRORLEVEL 1 goto error
makebmp  res\percent.bmp  res\lz\percent.lzb
if ERRORLEVEL 1 goto error

rem makebmp  res\button_test.bmp  res\lz\button_test.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\button_testA.bmp  res\lz\button_testA.lzb
rem if ERRORLEVEL 1 goto error

makebmp  res\btnconfirm.bmp  res\lz\btnconfirm.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btngoback.bmp  res\lz\btngoback.lzb
if ERRORLEVEL 1 goto error

rem makebmp  res\titile_lenovo_drv_install.bmp  res\lz\titile_lenovo_drv_install.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\titile_drv_install.bmp  res\lz\titile_drv_install.lzb
rem if ERRORLEVEL 1 goto error

rem makebmp  res\bk_dai_settings.bmp  res\lz\bk_dai_settings.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\bk_dai_finish.bmp  res\lz\bk_dai_finish.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\bk_dai_error.bmp  res\lz\bk_dai_error.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\bk_dai_copying.bmp  res\lz\bk_dai_copying.lzb
rem if ERRORLEVEL 1 goto error
makebmp  res\bk_complete2.bmp  res\lz\bk_complete2.lzb
if ERRORLEVEL 1 goto error    

makebmp  res\initial_bk.bmp  res\lz\initial_bk.lzb
if ERRORLEVEL 1 goto error
makebmp  res\initial_bk_space.bmp  res\lz\initial_bk_space.lzb
if ERRORLEVEL 1 goto error
makebmp  res\initial_bk_txt.bmp  res\lz\initial_bk_txt.lzb
if ERRORLEVEL 1 goto error  

makebmp  res\exit_focus.bmp  res\lz\exit_focus.lzb
if ERRORLEVEL 1 goto error
makebmp  res\exit_normal.bmp  res\lz\exit_normal.lzb
if ERRORLEVEL 1 goto error
makebmp  res\version_normal.bmp  res\lz\version_normal.lzb
if ERRORLEVEL 1 goto error  
rem makebmp  res\bk_dai_error2.bmp  res\lz\bk_dai_error2.lzb
rem if ERRORLEVEL 1 goto error  

makebmp  res\next.bmp  res\lz\next.lzb
if ERRORLEVEL 1 goto error  
makebmp  res\nextfocus.bmp  res\lz\nextfocus.lzb
if ERRORLEVEL 1 goto error  


makebmp  res\sysbktitle.bmp  res\lz\sysbktitle.lzb
if ERRORLEVEL 1 goto error  
makebmp  res\sysrtitle.bmp  res\lz\sysrtitle.lzb
if ERRORLEVEL 1 goto error 
makebmp  res\bk_ok_desc.bmp  res\lz\bk_ok_desc.lzb
if ERRORLEVEL 1 goto error  
makebmp  res\r_ok_desc.bmp  res\lz\r_ok_desc.lzb
if ERRORLEVEL 1 goto error 
makebmp  res\btn_down.bmp  res\lz\btn_down.lzb
if ERRORLEVEL 1 goto error  
makebmp  res\btn_down_focus.bmp  res\lz\btn_down_focus.lzb
if ERRORLEVEL 1 goto error 
makebmp  res\btn_up.bmp  res\lz\btn_up.lzb
if ERRORLEVEL 1 goto error  
makebmp  res\btn_up_focus.bmp  res\lz\btn_up_focus.lzb
if ERRORLEVEL 1 goto error  
makebmp  res\copyright.bmp  res\lz\copyright.lzb
if ERRORLEVEL 1 goto error 
makebmp  res\goback_focus.bmp  res\lz\goback_focus.lzb
if ERRORLEVEL 1 goto error  

makebmp  res\confirm_unfocus.bmp  res\lz\confirm_unfocus.lzb
if ERRORLEVEL 1 goto error  

makebmp  res\btnrestoredisable.bmp  res\lz\btnrestoredisable.lzb
if ERRORLEVEL 1 goto error  

rem makebmp  res\recover_tip_check_backup_point.bmp  res\lz\recover_tip_check_backup_point.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\recover_tip_select_backup_point.bmp  res\lz\recover_tip_select_backup_point.lzb
rem if ERRORLEVEL 1 goto error

rem makebmp  res\backup_use_time.bmp  res\lz\backup_use_time.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\backup_remain_time.bmp  res\lz\backup_remain_time.lzb
rem if ERRORLEVEL 1 goto error


rem makebmp  res\backup_select_store_device.bmp  res\lz\backup_select_store_device.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\backup_data_size.bmp  res\lz\backup_data_size.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\backup_memo.bmp  res\lz\backup_memo.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\backup_hot_pnp.bmp  res\lz\backup_hot_pnp.lzb
rem if ERRORLEVEL 1 goto error
rem makebmp  res\bk_dai_finish2.bmp  res\lz\bk_dai_finish2.lzb
rem if ERRORLEVEL 1 goto error
makebmp  res\btn_cancel_1.bmp  res\lz\btn_cancel_1.lzb
if ERRORLEVEL 1 goto error
makebmp  res\btn_cancel_1A.bmp  res\lz\btn_cancel_1A.lzb
if ERRORLEVEL 1 goto error
makebmp  res\copyright-mini.bmp  res\lz\copyright-mini.lzb
if ERRORLEVEL 1 goto error


makebmp  res\0.bmp  res\lz\0.lzb
makebmp  res\1.bmp  res\lz\1.lzb
makebmp  res\2.bmp  res\lz\2.lzb
makebmp  res\3.bmp  res\lz\3.lzb
makebmp  res\4.bmp  res\lz\4.lzb
makebmp  res\5.bmp  res\lz\5.lzb
makebmp  res\6.bmp  res\lz\6.lzb
makebmp  res\7.bmp  res\lz\7.lzb
makebmp  res\8.bmp  res\lz\8.lzb
makebmp  res\9.bmp  res\lz\9.lzb

makebmp  res\license01.bmp  res\lz\license01.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license02.bmp  res\lz\license02.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license03.bmp  res\lz\license03.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license04.bmp  res\lz\license04.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license05.bmp  res\lz\license05.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license06.bmp  res\lz\license06.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license07.bmp  res\lz\license07.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license08.bmp  res\lz\license08.lzb
if ERRORLEVEL 1 goto error
makebmp  res\license09.bmp  res\lz\license09.lzb
if ERRORLEVEL 1 goto error

makebmp  res\licenseBackbtn01.bmp  res\lz\licenseBackbtn01.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licenseBackbtn02.bmp  res\lz\licenseBackbtn02.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licenseBackbtn03.bmp  res\lz\licenseBackbtn03.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licensenextbtn01.bmp  res\lz\licensenextbtn01.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licensenextbtn02.bmp  res\lz\licensenextbtn02.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licensenextbtn03.bmp  res\lz\licensenextbtn03.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licenseconfirmbtn01.bmp  res\lz\licenseconfirmbtn01.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licenseconfirmbtn02.bmp  res\lz\licenseconfirmbtn02.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licenseclose01.bmp  res\lz\licenseclose01.lzb
if ERRORLEVEL 1 goto error
makebmp  res\licenseclose02.bmp  res\lz\licenseclose02.lzb
if ERRORLEVEL 1 goto error



pause
cd res\lz
copy /b title.lzb+bktitle.lzb+rstitle.lzb+goback.lzb+foot.lzb            okr0.dat
copy /b btnfocus.lzb+btn.lzb+mousea.lzp+curstext.lzp+cursmove.lzp        okr5.dat
copy /b curshand.lzp+cancela.lzp+uparrowa.lzj+uparrowb.lzp+dnarrowa.lzj  okr10.dat
copy /b dnarrowb.lzp+proc.lzp+bking.lzb+backupfoot.lzb+restorefoot.lzb   okr15.dat
copy /b rsting.lzb+usb.lzb+disk.lzb+complete.lzb+bkend.lzb               okr20.dat
copy /b rstend.lzb+down.lzj+percent.lzb                                  okr25.dat
copy /b btnbackup.lzb+btnbackupA.lzb+btnrestore.lzb+btnrestoreA.lzb      okr28.dat
copy /b btnbackup1.lzb+btnbackup1A.lzb+btnrestore1.lzb+btnrestore1A.lzb  okr32.dat
copy /b btndelete.lzb+btndeleteA.lzb+btndeletedisable.lzb                okr36.dat

rem copy /b button_test.lzb+button_testA.lzb                                 okr39.dat
copy /b btnconfirm.lzb+btngoback.lzb                                     okr41.dat

rem copy /b titile_lenovo_drv_install.lzb+titile_drv_install.lzb             okr43.dat
rem copy /b bk_dai_settings.lzb+bk_dai_finish.lzb+bk_dai_error.lzb+bk_dai_copying.lzb+bk_complete2.lzb         okr45.dat
copy /b bk_complete2.lzb         okr45.dat
copy /b initial_bk.lzb+initial_bk_space.lzb+initial_bk_txt.lzb                okr50.dat
rem copy /b exit_focus.lzb+exit_normal.lzb+version_normal.lzb+bk_dai_error2.lzb   okr53.dat
copy /b exit_focus.lzb+exit_normal.lzb+version_normal.lzb   okr53.dat
copy /b next.lzb+nextfocus.lzb+sysbktitle.lzb+sysrtitle.lzb                   okr57.dat
copy /b bk_ok_desc.lzb+r_ok_desc.lzb+btn_down.lzb+btn_down_focus.lzb          okr61.dat
copy /b btn_up.lzb+btn_up_focus.lzb+copyright.lzb+goback_focus.lzb+confirm_unfocus.lzb+btnrestoredisable.lzb    okr65.dat
rem copy /b recover_tip_select_backup_point.lzb+recover_tip_check_backup_point.lzb+backup_use_time.lzb+backup_remain_time.lzb+backup_select_store_device.lzb+backup_data_size.lzb+backup_memo.lzb+backup_hot_pnp.lzb+bk_dai_finish2.lzb+btn_cancel_1.lzb+btn_cancel_1A.lzb      okr66.dat
copy /b btn_cancel_1.lzb+btn_cancel_1A.lzb+copyright-mini.lzb      okr66.dat

copy /b 0.lzb+1.lzb+2.lzb+3.lzb+4.lzb+5.lzb+6.lzb+7.lzb+8.lzb+9.lzb   okr71.dat
copy /b license01.lzb+license02.lzb+license03.lzb+license04.lzb+license05.lzb+license06.lzb+license07.lzb+license08.lzb+license09.lzb   okr72.dat
copy /b licenseBackbtn01.lzb+licenseBackbtn02.lzb+licenseBackbtn03.lzb+licensenextbtn01.lzb+licensenextbtn02.lzb+licensenextbtn03.lzb+licenseconfirmbtn01.lzb+licenseconfirmbtn02.lzb+licenseclose01.lzb+licenseclose02.lzb okr73.dat


copy /b okr0.dat+okr5.dat+okr10.dat+okr15.dat+okr20.dat+okr25.dat+okr28.dat+okr32.dat+okr36.dat+okr41.dat+okr45.dat+okr50.dat+okr53.dat+okr57.dat+okr61.dat+okr65.dat+okr66.dat+okr71.dat+okr73.dat  res.dat
copy res.dat ..\..
cd ..\..
bintoc res.dat res.h
bintoc fonts.lz fonts.h

pause
goto end

:error
echo error error error!!!
pause

:end
