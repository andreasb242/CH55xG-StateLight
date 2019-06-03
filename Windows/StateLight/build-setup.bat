@echo OFF

echo Building Solution...

"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" StateLight.sln /build Release

echo Copy Files...
if exist setup_data rmdir setup_data /s /q
if exist StateLigh.zip rmdir StateLigh.zip /s /q
mkdir setup_data

COPY bin\Release\StateLight.exe setup_data
COPY bin\Release\StateLight.exe.config setup_data
COPY bin\Release\StateLightPluginDef.dll setup_data
mkdir setup_data\Plugin

mkdir setup_data\Plugin\Lync2010
COPY ..\Lync2010Plugin\bin\Release\Lync2010Plugin.dll setup_data\Plugin\Lync2010\
COPY ..\Lync2010Plugin\SDK\*.dll setup_data\Plugin\Lync2010\
COPY PluginDeploy\Lync2010\*.xml setup_data\Plugin\Lync2010\


echo Packe files...
cd setup_data
"C:\Program Files\7-Zip\7z.exe" a ..\StateLigh.zip "*.*" -r
cd ..

pause