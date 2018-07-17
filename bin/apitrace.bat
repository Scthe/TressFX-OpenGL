@rm *.trace
@mv bin\opengl32.apitrace.dll bin\opengl32.dll

@bin\TressFx.exe
C:\programs\portable\apitrace\bin\qapitrace.exe TressFx.trace

@REM @bin\tests.exe
@REM C:\programs\portable\apitrace\bin\qapitrace.exe tests.trace

@mv bin\opengl32.dll bin\opengl32.apitrace.dll
