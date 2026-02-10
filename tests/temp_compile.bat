@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl /EHsc /MD /W3 /O2 /Fe:goldsim_simulator.exe goldsim_simulator.cpp /link /SUBSYSTEM:CONSOLE
