# Contributing

If you want to contribute to this project, please fork the repository and
submit pull requests.

If you have questions about you can reach me via Email with the Email address shown in
my profile.

## Rules

* The license is MIT and submitted code must not be incompatible with it.
* The code must compile without warnings, or the CI build fails. The CI build also
fails when a test fails.
* You are encouraged to use new C++ features. Partly this project is also a playground
to explore modern C++.
* The code should be readable and it should be obvious what it does. 
* Please stick to the coding style.

## Windows

On Windows the easiest is to use Visual Studio 2019 (Community). For each project
there is a solution file `.sln` in the respective directory. There is also
a solution file (`absall/absall.sln`) which contains all server projects.

## Linux

As C++ IDE on Linux I prefer QtCreator. There are QtCreator project
files `.creator` in the subdirectories of the projects. You can also setup
QtCreator to build the projects with the makefiles in the `makefiles` directory.
