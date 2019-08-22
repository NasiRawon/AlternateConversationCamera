# AlternateConversationCamera

Knowledge requirements:
1. C/C++
2. Microsoft Visual Studio

Build requirements:
1. Microsoft Visual Studio 2019
2. SKSE64 source code latest version

Build instructions:
1. Clone or download the project
2. Get the SKSE64 source code http://skse.silverlock.org/, choose the current SE build
3. Extract the SKSE64 source code and copy "common", "skse64", "skse64_common", and "xbyak" folders to the project root directory (same location as the solution file)
4. Open the project solution in Microsoft Visual Studio
5. Re-add the project references for both skse64 and skse64_common
   a. For skse64, remove both common and skse64_common references then add them back
   b. For skse64_common, remove common reference then add it back
6. Change the build type for the skse64 project to static lib
7. Remove the post-build event for the the skse64 project, both the command line and the description
8. Change the solution configuration to Release mode, then build the project
9. Check the \x64\Release folder, the project DLL is there
10. Done
