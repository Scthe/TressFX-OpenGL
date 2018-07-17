@cls

@python bin/build_glsl.py
@if %errorlevel% neq 0 goto glsl_include_fail

@make
@if "%errorlevel%"=="0" goto success

:failure
@echo "--- compile/linking failed ---"
@goto end

:glsl_include_fail
@echo "--- shader include resolve failed ---"
@goto end

:success
@echo "--- compile/linking success ---"
@bin\TressFx.exe

:end
