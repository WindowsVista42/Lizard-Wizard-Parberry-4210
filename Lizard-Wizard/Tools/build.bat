pushd "..\Media\Models"

for /r %%v in (*.obj) do "..\..\Tools\meshconvert.exe" %%v -vbo -c -y

popd

pushd "..\Media\Textures"

..\..\Tools\texconv.exe -r *.png -ft dds -y

popd
