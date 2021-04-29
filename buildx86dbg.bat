@call "D:\edk2_OKR\edksetup.bat"
rem Build -t VS2015 -a X64 -p OkrPkg\OkrPkg-dbg.dsc -m OkrPkg\okrd.inf -b DEBUG
Build -t VS2015 -a X64 -p OkrPkg\OkrPkg-dbg.dsc -m OkrPkg\okrd.inf -b RELEASE

rem pause
rem copy /b D:\edk2_OKR\Build\OkrPkg\DEBUG_VS2015\X64\OkrPkg\okrd\OUTPUT\okrefi.efi D:\edk2_OKR\Build\NT32X64\RELEASE_VS2015x86\X64 /y
rem copy /b D:\edk2_OKR\Build\OkrPkg\DEBUG_VS2015\X64\OkrPkg\okrd\OUTPUT\okrefi.efi D:\edk2_OKR\Build\OkrPkg\DEBUG_VS2015\X64\OkrPkg\okrd\OUTPUT\lenovookr.efi /y
copy /b D:\edk2_OKR\Build\OkrPkg\RELEASE_VS2015\X64\OkrPkg\okrd\OUTPUT\okrefi.efi D:\edk2_OKR\Build\NT32X64\RELEASE_VS2015x86\X64 /y
copy /b D:\edk2_OKR\Build\OkrPkg\RELEASE_VS2015\X64\OkrPkg\okrd\OUTPUT\okrefi.efi D:\edk2_OKR\Build\OkrPkg\RELEASE_VS2015\X64\OkrPkg\okrd\OUTPUT\lenovookr.efi /y
