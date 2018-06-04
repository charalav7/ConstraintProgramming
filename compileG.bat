cl /DNDEBUG /EHsc /MD /Ox /wd4355 -I"C:\Program Files\Gecode\include" -c -Fo%1.obj -Tp%1.cpp
cl /DNDEBUG /EHsc /MD /Ox /wd4355 -I"C:\Program Files\Gecode\include" -Fe%1.exe %1.obj /link /LIBPATH:"C:\Program Files\Gecode\lib"
%1.exe %2