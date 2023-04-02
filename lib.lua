project "apmath"

location "."
kind "StaticLib"
language "C++"
externalincludedirs "include"

files { 
    "include/APMath/APInt.h",
    "include/APMath/APFloat.h",
    "src/APInt.cpp",
    "src/APFloat.cpp",
    "src/Conversion.cpp",
}

