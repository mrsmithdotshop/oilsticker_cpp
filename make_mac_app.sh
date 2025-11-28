#!/bin/bash
cmake -DCMAKE_PREFIX_PATH="/opt/homebrew/Cellar/qt/6.9.3/lib/cmake"
cmake --build . --config Release || { echo "Build failed"; exit 1; }
cp oilstickergui ./OilStickerApp/OilStickerApp.app/Contents/MacOS/.
/opt/homebrew/Cellar/qt/6.9.3/bin/macdeployqt ./OilStickerApp/OilStickerApp.app/ -verbose=1
#cd ./OilStickerApp
codesign --deep --force --verbose   --sign - ./OilStickerApp/OilStickerApp.app
rm OilStickerApp.dmg
create-dmg OilStickerApp.dmg ./OilStickerApp
