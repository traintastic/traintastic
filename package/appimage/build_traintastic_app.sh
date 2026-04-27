#!/bin/bash
#
# -----------------------------------------------------------------------------
#  Build Script: Traintastic AppImage
# -----------------------------------------------------------------------------
#
#  Description:
#    This script builds an AppImage for Traintastic.
#    The resulting AppImage bundles both the traintastic server and client.
#
#    On startup, the server is launched first, followed shortly by the client.
#    When the client exits, the server is automatically stopped as well.
#
#  Requirements:
#    - appstream
#      Install with: sudo apt install appstream
#
#  Author:
#    Tom (DL7BJ)
#
#  History:
#    2026-04-21  Tom (DL7BJ)    Initial version
#    2026-04-25  Tom (DL7BJ)    Change the output directory and read version
#                               info from website
#    2026-04-26  Tom (DL7BJ)    Set environment for language and manual files
#                               Rename Client&Server AppImage
#    2026-04-26  Reinder        Added LNCV XML files
#
#  Usage:
#
#  ./build_traintastic_app.sh <mode>
#       where mode is:
#           server     - build an appimage for traintastic-server
#           client     - build an appimage for traintastic-client
#           both       - build one appimage for server and client
#               This is the simplest way to start Traintastic, it
#               starts at first the server, waits a moment and then
#               the client. On closing, the server will also stopped.
#
#           both is the default.
#
# -----------------------------------------------------------------------------

MODE="${1:-both}"
case "$MODE" in
    server|client|both) ;;
    *)
        echo "Usage: $0 [server|client|both]"
        exit 1
        ;;
esac

APP_DIR="traintastic.app"
APP_NAME="Traintastic"
OUTDIR="../Apps"
# Path to binary files
SOURCE_SERVER="../../server/build/traintastic-server"
SOURCE_CLIENT="../../client/build/traintastic-client"
SOURCE_ICON="../../client/debian/traintastic_256.png"
SOURCE_LANG="../../shared/translations"
SOURCE_MANUAL="../../manual"
SOURCE_LNCVXML="../../shared/data/lncv/xml"

# Copy the build files also to TraintasticApp
if [ ! -d $OUTDIR ];then
    mkdir $OUTDIR
fi

# Names for desktop icons and extension for appname
case "$MODE" in
    server)
        DESKTOP_NAME="Traintastic Server"
        DESKTOP_COMMENT="Model railroad control server"
        EXT="-server-"
        ;;
    client)
        DESKTOP_NAME="Traintastic Client"
        DESKTOP_COMMENT="Model railroad control client"
        EXT="-client-"
        ;;
    both)
        DESKTOP_NAME="Traintastic"
        DESKTOP_COMMENT="Model railroad control and automation software"
        EXT="-"
        ;;
esac

echo "--- Build filename for AppImage"
VERSION=$(grep "TRAINTASTIC_VERSION" ../../shared/traintastic.cmake \
  | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')

CODENAME=$(grep -E "set\(TRAINTASTIC_CODENAME" ../../shared/traintastic.cmake \
  | sed -E 's/.*set\(TRAINTASTIC_CODENAME[[:space:]]+"?([^")]*)"?\).*/\1/' \
  | tr -d '[:space:]')

# ---- Git Commit (for local and CI fallback) ----
GITHASH=$(git rev-parse --short HEAD 2>/dev/null)

BUILD=$(curl -s "https://traintastic.org/en/download/develop/master" \
  | tr '\n' ' ' \
  | grep -oE 'Build #[0-9]+' \
  | head -n 1 \
  | grep -oE '[0-9]+')

BUILD=${BUILD:-$GITHASH}
ARCH=$(uname -m)

case "$ARCH" in
  x86_64) ARCHN="x86_64" ;;
  aarch64) ARCHN="arm64" ;;
  armv7l) ARCHN="armhf" ;;
esac

if [ -n "$GITHUB_ACTIONS" ]; then
    echo "Running in GitHub Actions"
    APPNAME="traintastic${EXT}${VERSION}-${GITHUB_RUN_NUMBER}-${ARCHN}.AppImage"
else
    echo "Running locally"
    APPNAME="traintastic${EXT}${VERSION}-${BUILD}-${ARCHN}-dev.AppImage"
fi

echo "--- Building AppImage for ---"
echo "Version  :" $VERSION
echo "Build    :" $BUILD
echo "Codename :" $CODENAME
echo "Githash  :" $GITHASH
echo "Arch     :" $ARCH
echo "Appname  :" $APPNAME

echo "--- Start build process for $APP_NAME ---"
# create directories
mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APP_DIR/usr/share/applications"
mkdir -p "$APP_DIR/traintastic/translations"
mkdir -p "$APP_DIR/traintastic/manual"
mkdir -p "$APP_DIR/traintastic/lncv"

# copy binary files
if [[ -f "$SOURCE_SERVER" && -f "$SOURCE_CLIENT" ]]; then
    if [ "$MODE" = "client" ];then
        cp "$SOURCE_CLIENT" "$APP_DIR/usr/bin/traintastic-client"
        chmod +x "$APP_DIR/usr/bin/traintastic-client"
        cp "$SOURCE_CLIENT" $OUTDIR
    elif [ "$MODE" = "server" ];then
        cp "$SOURCE_SERVER" "$APP_DIR/usr/bin/traintastic-server"
        chmod +x "$APP_DIR/usr/bin/traintastic-server"
        cp "$SOURCE_SERVER" $OUTDIR
    else
        cp "$SOURCE_CLIENT" "$APP_DIR/usr/bin/traintastic-client"
        chmod +x "$APP_DIR/usr/bin/traintastic-client"
        cp "$SOURCE_SERVER" "$APP_DIR/usr/bin/traintastic-server"
        chmod +x "$APP_DIR/usr/bin/traintastic-server"
        cp "$SOURCE_CLIENT" $OUTDIR
        cp "$SOURCE_SERVER" $OUTDIR
    fi
    echo "[OK] binary files copied"
else
    echo "[ERROR] Source binaries not found"
    exit 1
fi

if [ "$MODE" = "both" ]; then
cat << 'EOF' > "$APP_DIR/AppRun"
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export TRAINTASTIC_LOCALE_PATH="$APPDIR/traintastic/translations"
export TRAINTASTIC_MANUAL_PATH="$APPDIR/traintastic/manual"
export TRAINTASTIC_LNCVXML_PATH="$APPDIR/traintastic/lncv"
LOGDIR="$HOME/.config/traintastic-server/log"
mkdir -p "$LOGDIR"
# start server (background)
"$HERE/usr/bin/traintastic-server" > "$LOGDIR/server.log" 2>&1 &
SERVER_PID=$!
sleep 1.5
# start client
"$HERE/usr/bin/traintastic-client" "$@"
# cleanup
kill $SERVER_PID 2>/dev/null
EOF

elif [ "$MODE" = "server" ]; then
cat << 'EOF' > "$APP_DIR/AppRun"
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export TRAINTASTIC_LOCALE_PATH="$APPDIR/traintastic/translations"
export TRAINTASTIC_MANUAL_PATH="$APPDIR/traintastic/manual"
LOGDIR="$HOME/.traintastic"
mkdir -p "$LOGDIR"
# run server in foreground
exec "$HERE/usr/bin/traintastic-server" "$@"
EOF

elif [ "$MODE" = "client" ]; then
cat << 'EOF' > "$APP_DIR/AppRun"
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export TRAINTASTIC_LOCALE_PATH="$APPDIR/traintastic/translations"
export TRAINTASTIC_MANUAL_PATH="$APPDIR/traintastic/manual"
export TRAINTASTIC_LNCVXML_PATH="$APPDIR/traintastic/lncv"
# run client
exec "$HERE/usr/bin/traintastic-client" "$@"
EOF
fi

chmod +x "$APP_DIR/AppRun"
echo "[OK] AppRun-Script created for mode: $MODE"

# create a desktop file
cat << EOF > "$APP_DIR/usr/share/applications/traintastic.desktop"
[Desktop Entry]
Type=Application
Name=$DESKTOP_NAME
Exec=AppRun
Comment=$DESKTOP_COMMENT
Icon=traintastic
Categories=Science;
Terminal=false
EOF
echo "[OK] traintastic.desktop created for $MODE"

# copy app icon
if [[ -f "$SOURCE_ICON" ]]; then
    cp "$SOURCE_ICON" "$APP_DIR/usr/share/icons/hicolor/256x256/apps/traintastic.png"
else
    # Fallback: if no icon exists, create an empty icon
    touch "$APP_DIR/usr/share/icons/hicolor/256x256/apps/traintastic.png"
    echo "[INFO] no icon found, created a placeholder"
fi
# copy translations
if [[ -f "$SOURCE_LANG/en-us.lang" ]]; then
    cp -a ${SOURCE_LANG}/*.lang "$APP_DIR/traintastic/translations"
else
    # no language files?
    echo "[INFO] no language files found"
fi
# copy manual
cp -a ${SOURCE_MANUAL}/output/. "$APP_DIR/traintastic/manual/"
# copy lncv
cp -a ${SOURCE_LNCVXML}/. "$APP_DIR/traintastic/lncv/"

# load linuxdeploy, if not exists
LINUXDEPLOY="linuxdeploy-${ARCH}.AppImage"

if [ ! -f "./${LINUXDEPLOY}" ]; then
    echo "download linuxdeploy from github"
    wget -q "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/${LINUXDEPLOY}"
    chmod +x "${LINUXDEPLOY}"
fi
# Build the image with linuxdeploy
echo "Build the AppImage with ${LINUXDEPLOY}"

export ARCH
"./${LINUXDEPLOY}" --appdir "$APP_DIR" --output appimage

# Copy generated appimage
APPIMAGE_FILE=$(find . -maxdepth 1 -name "$APP_NAME*.AppImage" | head -n 1)
if [ -n "$APPIMAGE_FILE" ]; then
    cp "$APPIMAGE_FILE" "$OUTDIR/$APPNAME"
else
    echo "[ERROR] AppImage not found"
    exit 1
fi
# Cleanup
if [ -f "./${LINUXDEPLOY}" ]; then
   rm -f "./${LINUXDEPLOY}"
fi

rm -f *.AppImage

if [ -d "$APP_DIR" ]; then
   rm -rf "$APP_DIR"
fi

echo "--- Ready! AppImage build" $OUTDIR
