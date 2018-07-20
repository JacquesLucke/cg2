# Compilation

## Windows

1. Install Visual Studio 2017 (we need the c++ compiler)
2. Install cmake
3. Create a folder called `cg2-git` (other names are possible as well)
4. Go into that folder (`cd cg2-git`)
5. Clone the repository (there will be a `cg2` folder inside of the `cg2-git` folder)
6. Create a `build` directory in the `cg2-git` folder
7. Go into the `build` folder
8. Open the `x64 Native Tools Command Prompt for VS 2017` (a terminal with some stuff set up already)
9. Run `cmake ..\cg2\` to create a Visual Studio project
10. Run `devenv cg2.sln /build Release` or to compile the project
11. Copy the `recourses` folder into the `Release` folder (this step is not automatic yet)
12. Run `Release\exerciseX.exe` to start the program (`X` is a number between 1 and 4)

## Linux

1. Install cmake
2. Create a folder called `cg2-git` (other names are possible as well)
3. Got into that folder (`cd cg2-git`)
4. Clone the repository (there will be a `cg2` folder inside of the `cg2-git` folder)
5. Create a `build` folder in the `cg2-git` folder
6. Go into the `build` folder
7. Run `cmake ../cg2/ -DCMAKE_BUILD_TYPE=Release`
8. Run `make`
9. Run `./exerciseX` (`X` is a number between 1 and 4)