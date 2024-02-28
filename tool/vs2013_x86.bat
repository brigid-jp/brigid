chcp 65001
set PATH=c:\opt\lua-x86\bin;%PATH%
c:
cd \opt\brigid
%comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
luarocks --lua-version 5.1 --version
