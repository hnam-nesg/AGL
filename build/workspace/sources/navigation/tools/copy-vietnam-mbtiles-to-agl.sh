#!/usr/bin/env bash
set -euo pipefail

TARGET=${1:?usage: copy-vietnam-mbtiles-to-agl.sh root@raspberrypi5 /path/to/vietnam.mbtiles}
MBTILES=${2:-/home/nesg/vietnam.mbtiles}
REMOTE_PATH=${REMOTE_PATH:-/usr/share/navigation/maps/vietnam.mbtiles}

if [[ ! -f "${MBTILES}" ]]; then
    echo "missing MBTiles: ${MBTILES}" >&2
    exit 1
fi

if command -v sqlite3 >/dev/null 2>&1; then
    MAX_ZOOM=$(sqlite3 "${MBTILES}" 'select max(zoom_level) from tiles;' 2>/dev/null || true)
    if [[ -n "${MAX_ZOOM}" && "${MAX_ZOOM}" -lt 14 ]]; then
        echo "warning: ${MBTILES} max zoom is ${MAX_ZOOM}; real 3D buildings need OpenMapTiles MAX_ZOOM=14" >&2
    fi
fi

scp "${MBTILES}" "${TARGET}:/tmp/vietnam.mbtiles"
ssh "${TARGET}" "mkdir -p /usr/share/navigation/maps && install -m 0644 /tmp/vietnam.mbtiles '${REMOTE_PATH}' && systemctl restart agl-app@navigation.service"
