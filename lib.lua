project "apmath"

location "."
kind "StaticLib"
language "C++"
externalincludedirs "include"

files { 
    "include/APMath/APInt.h",
    "src/APInt.cpp"
}

