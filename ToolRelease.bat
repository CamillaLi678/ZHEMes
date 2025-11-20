@echo off
rem "将CopyFlag带/Q则不显示拷贝的文件名称，带/Y则不提示需不需要覆盖"
set "CopyFlag=/Y"
set /P Version=请输入版本号(例如1.0.0): 
set "CurPwd=%cd%"
set "LogName=%CurPwd%/Export.txt"
set "Ymd=%date:~,4%%date:~5,2%%date:~8,2%"
set "DestPath=%CurPwd%\ToolReleaseVersions\ACComMes-V%Version%-%Ymd%"
set "SourcePath=%CurPwd%\Build"

rem "创建需要的目录"
echo 创建必要的目录...
RMDIR /S /Q "%DestPath%"
MKDIR "%DestPath%"
MKDIR "%DestPath%\Log"
MKDIR "%DestPath%\AutoTask"
MKDIR "%DestPath%\Docs"
MKDIR "%DestPath%\ProgFile"
MKDIR "%DestPath%\ProjSave"
MKDIR "%DestPath%\Report"
MKDIR "%DestPath%\ProjTemplate"
MKDIR "%DestPath%\MesTest"


XCOPY "%SourcePath%\Docs" "%DestPath%\Docs" /E %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\ACComMes.exe" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\Setting_Default.json" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\ACComMesChineseTra.dll" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\ACComMesEnglish.dll" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\ExternBurn.dll" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\Module.json" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\ExternBurn.ini" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\VC2015_vcredist_x86.exe" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\config.json" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\PrintCfg.json" "%DestPath%" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\MesTest\result.json" "%DestPath%\MesTest\" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\MesTest\programmer_info.json" "%DestPath%\MesTest\" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\MesTest\reporter.json" "%DestPath%\MesTest\" %CopyFlag% >> %LogName%
XCOPY "%SourcePath%\MesTest\Report.txt" "%DestPath%\MesTest\" %CopyFlag% >> %LogName%
XCOPY "%CurPwd%\Docs\*.pdf" "%DestPath%\Docs\" %CopyFlag% >> %LogName%
ren "%DestPath%\Setting_Default.json" "Setting.json"

pause