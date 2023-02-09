rem https://docs.microsoft.com/en-us/cpp/build/walkthrough-using-msbuild-to-create-a-visual-cpp-project?view=msvc-160


rem compile all libs

msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=Release /p:platform=x64
msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=Debug /p:platform=x64
msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=ReleaseMD /p:platform=x64
msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=DebugMD /p:platform=x64

msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=Release /p:platform=x86
msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=Debug /p:platform=x86
msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=ReleaseMD /p:platform=x86
msbuild VExDebugger/VExDebugger.vcxproj /t:Rebuild /p:configuration=DebugMD /p:platform=x86
