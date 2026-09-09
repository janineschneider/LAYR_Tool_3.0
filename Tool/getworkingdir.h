#ifndef GETWORKINGDIR_H
#define GETWORKINGDIR_H

#include <stdio.h>  /** defines FILENAME_MAX */
#include <iostream>


#ifdef WINDOWS // Definition is set in root CMakeLists.txt
#include <direct.h>
#define getCurrentDir _getcwd
static const std::string slash = "\\";
#else
#include <unistd.h>
#define getCurrentDir getcwd
static const std::string slash = "/";
#endif

#endif // GETWORKINGDIR_H
