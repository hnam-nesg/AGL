#!/usr/bin/env bash
set -euo pipefail

OPENMAPTILES_DIR="${OPENMAPTILES_DIR:-/home/nesg/openmaptiles}"

AREA_NAME="${AREA_NAME:-hcmc}"
SOURCE_PBF="${SOURCE_PBF:-/home/nesg/vietnam-260507.osm.pbf}"
AREA_PBF="${AREA_PBF:-/home/nesg/${AREA_NAME}.osm.pbf}"
OUT_MBTILES="${OUT_MBTILES:-/home/nesg/vietnam.mbtiles}"

# TP.HCM mở rộng, bao gồm tọa độ app đang dùng: 10.80690, 106.70290
BBOX="${BBOX:-106.3500,10.3500,107.1500,11.2000}"

MIN_ZOOM="${MIN_ZOOM:-0}"
MAX_ZOOM="${MAX_ZOOM:-14}"

GEOM_URL="${GEOM_URL:-https://download.geofabrik.de/asia/vietnam-latest.osm.pbf}"

need_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "missing command: $1" >&2
        exit 1
    fi
}

need_cmd git
need_cmd docker
need_cmd osmium
need_cmd sqlite3
need_cmd wget

echo "== OpenMapTiles build config =="
echo "OPENMAPTILES_DIR=${OPENMAPTILES_DIR}"
echo "AREA_NAME=${AREA_NAME}"
echo "SOURCE_PBF=${SOURCE_PBF}"
echo "AREA_PBF=${AREA_PBF}"
echo "OUT_MBTILES=${OUT_MBTILES}"
echo "BBOX=${BBOX}"
echo "MIN_ZOOM=${MIN_ZOOM}"
echo "MAX_ZOOM=${MAX_ZOOM}"
echo

if [[ ! -d "${OPENMAPTILES_DIR}/.git" ]]; then
    echo "== Clone OpenMapTiles =="
    git clone https://github.com/openmaptiles/openmaptiles.git "${OPENMAPTILES_DIR}"
fi

if [[ ! -f "${SOURCE_PBF}" ]]; then
    echo "== Download Vietnam OSM PBF =="
    wget -O "${SOURCE_PBF}" "${GEOM_URL}"
fi

echo "== Extract area PBF =="
osmium extract \
    --bbox "${BBOX}" \
    --overwrite \
    -o "${AREA_PBF}" \
    "${SOURCE_PBF}"

cd "${OPENMAPTILES_DIR}"

echo "== Prepare OpenMapTiles data =="
mkdir -p data

cp "${AREA_PBF}" "data/${AREA_NAME}.osm.pbf"

# Ghi bbox cho area.
echo "${BBOX}" > "data/${AREA_NAME}.bbox"

# Set zoom trong .env.
touch .env

if grep -q '^MIN_ZOOM=' .env; then
    sed -i "s/^MIN_ZOOM=.*/MIN_ZOOM=${MIN_ZOOM}/" .env
else
    echo "MIN_ZOOM=${MIN_ZOOM}" >> .env
fi

if grep -q '^MAX_ZOOM=' .env; then
    sed -i "s/^MAX_ZOOM=.*/MAX_ZOOM=${MAX_ZOOM}/" .env
else
    echo "MAX_ZOOM=${MAX_ZOOM}" >> .env
fi

if grep -q '^BBOX=' .env; then
    sed -i "s/^BBOX=.*/BBOX=${BBOX}/" .env
else
    echo "BBOX=${BBOX}" >> .env
fi

echo "== Pull OpenMapTiles Docker images =="
docker compose pull

echo "== Run OpenMapTiles quickstart =="
./quickstart.sh "${AREA_NAME}"

echo "== Find generated MBTiles =="
GENERATED_MBTILES="$(find data -maxdepth 2 -type f -name '*.mbtiles' -printf '%T@ %p\n' | sort -nr | head -1 | cut -d' ' -f2-)"

if [[ -z "${GENERATED_MBTILES}" || ! -f "${GENERATED_MBTILES}" ]]; then
    echo "failed: no .mbtiles generated in ${OPENMAPTILES_DIR}/data" >&2
    exit 1
fi

echo "Generated: ${GENERATED_MBTILES}"

mkdir -p "$(dirname "${OUT_MBTILES}")"
cp "${GENERATED_MBTILES}" "${OUT_MBTILES}"

echo "== Validate output =="
echo "Output: ${OUT_MBTILES}"
ls -lh "${OUT_MBTILES}"

echo
echo "Metadata:"
sqlite3 "${OUT_MBTILES}" "SELECT name,value FROM metadata WHERE name IN ('minzoom','maxzoom','bounds','center','format','scheme');"

echo
echo "Zoom stats:"
sqlite3 "${OUT_MBTILES}" "
SELECT zoom_level, COUNT(*)
FROM tiles
GROUP BY zoom_level
ORDER BY zoom_level;
"

echo
echo "Vector layers:"
sqlite3 "${OUT_MBTILES}" "SELECT value FROM metadata WHERE name='json';" \
    | python3 -m json.tool \
    | grep -E '"id"|building|transportation|water|landuse|render_height|render_min_height' || true

MAX_IN_FILE="$(sqlite3 "${OUT_MBTILES}" "SELECT value FROM metadata WHERE name='maxzoom';" || true)"

if [[ "${MAX_IN_FILE}" != "${MAX_ZOOM}" ]]; then
    echo
    echo "WARNING: metadata maxzoom=${MAX_IN_FILE}, expected ${MAX_ZOOM}"
    echo "Check OpenMapTiles .env or quickstart output."
fi

echo
echo "DONE: ${OUT_MBTILES}"
