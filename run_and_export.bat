@echo off
setlocal

:: Step 1: Build the project
echo Building project...
cmake --build build
if errorlevel 1 goto :error

:: Step 2: Run your app to generate frames and audio
echo Running application...
build\MyGLFWAPP.exe
if errorlevel 1 goto :error

:: Step 3: Convert frames + audio to video using ffmpeg
echo Generating video with ffmpeg...
ffmpeg -y -framerate 30 -i version0/frames/frame_%%05d.ppm -i version0/audio.wav -c:v libx264 -pix_fmt yuv420p -c:a aac -shortest version0/output.mp4
del /q version0\frames\*.ppm
if errorlevel 1 goto :error

echo.
echo ✅ Done! Output saved to version0\output.mp4
goto :end

:error
echo.
echo ❌ Something went wrong.
exit /b 1

:end
endlocal
pause