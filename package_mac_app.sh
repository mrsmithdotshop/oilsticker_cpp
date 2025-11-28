#!/bin/bash

# -----------------------------
# Configuration
# -----------------------------
PROJECT_NAME="OilStickerApp"
EXECUTABLE_NAME="oilstickergui"
BUILD_DIR="."
ICNS_FILE="resources/oilchange.icns"
BACKGROUND_IMAGE="resources/oil_label_bg.png"  # path inside your project

APP_DIR="${PROJECT_NAME}.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"

# -----------------------------
# Step 1: Build Release
# -----------------------------
echo "Building release version..."
cmake --build "$BUILD_DIR" --config Release || { echo "Build failed"; exit 1; }

# -----------------------------
# Step 2: Create app bundle structure
# -----------------------------
echo "Creating app bundle structure..."
rm -rf "$APP_DIR"
mkdir -p "$MACOS_DIR"
mkdir -p "$RESOURCES_DIR"

# Copy executable
cp "$BUILD_DIR/Release/$EXECUTABLE_NAME" "$MACOS_DIR/"

# Copy .icns
cp "$ICNS_FILE" "$RESOURCES_DIR/"

# Copy background PNG
cp "$BACKGROUND_IMAGE" "$RESOURCES_DIR/"

# -----------------------------
# Step 3: Create Info.plist
# -----------------------------
echo "Creating Info.plist..."
cat > "$CONTENTS_DIR/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>${PROJECT_NAME}</string>
    <key>CFBundleDisplayName</key>
    <string>${PROJECT_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.mycompany.${PROJECT_NAME,,}</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleExecutable</key>
    <string>${EXECUTABLE_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>${ICNS_FILE}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
</dict>
</plist>
EOF

# -----------------------------
# Step 4: Deploy Qt frameworks
# -----------------------------
echo "Deploying Qt frameworks..."
/opt/homebrew/Cellar/qt/6.9.3/bin/macdeployqt "$APP_DIR" -verbose=1

# -----------------------------
# Step 5: Adjust background path in app (optional)
# -----------------------------
# If your LabelPreview uses ":/resources/oil_label_bg.png" internally, 
# it will now need to use "Resources/oil_label_bg.png".  

echo "App bundle created: $APP_DIR"
echo "Background image copied: $RESOURCES_DIR/$(basename $BACKGROUND_IMAGE)"
echo "You can now run it with: open $APP_DIR"

