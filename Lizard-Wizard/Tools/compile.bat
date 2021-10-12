pushd "..\Media\Models"

for /r %%v in (*.obj) do "..\..\Tools\meshconvert.exe" %%v -vbo -c -y

popd
