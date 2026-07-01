const MANIFEST_URL = "../../data/countries/manifest.json";
const ROOT_PARAM = new URLSearchParams(window.location.search).get("root");
const SOURCE_ROOT = ROOT_PARAM || "../../../work/polygon-data";
const FIT_PADDING = 24;
const DEFAULT_VIEW = {
  centerLon: 0,
  centerLat: 20,
  pixelsPerDegree: 3.2,
};

const state = {
  manifest: [],
  filteredManifest: [],
  selectedCountry: null,
  geometry: null,
  bounds: null,
  view: { ...DEFAULT_VIEW },
  drag: null,
};

const elements = {
  canvas: document.getElementById("map-canvas"),
  status: document.getElementById("status-text"),
  cursor: document.getElementById("cursor-readout"),
  search: document.getElementById("country-search"),
  select: document.getElementById("country-select"),
  fitButton: document.getElementById("fit-button"),
  resetButton: document.getElementById("reset-button"),
  metadata: document.getElementById("metadata"),
  viewport: document.getElementById("viewport-readout"),
};

const context = elements.canvas.getContext("2d");

function setStatus(message) {
  elements.status.textContent = message;
}

function formatCoordinate(value, positive, negative) {
  const suffix = value >= 0 ? positive : negative;
  return `${Math.abs(value).toFixed(4)}° ${suffix}`;
}

function setDefinitionList(element, rows) {
  element.replaceChildren();

  for (const [label, value] of rows) {
    const dt = document.createElement("dt");
    dt.textContent = label;
    const dd = document.createElement("dd");
    dd.textContent = value;
    element.append(dt, dd);
  }
}

function geometryFromPayload(payload) {
  const shape = payload?.geo_shape ?? payload;
  if (!shape) {
    throw new Error("Missing geometry payload");
  }

  if (shape.type === "Feature") {
    return shape.geometry;
  }

  if (shape.type === "FeatureCollection") {
    if (!Array.isArray(shape.features) || shape.features.length === 0) {
      throw new Error("FeatureCollection did not contain any features");
    }

    if (shape.features.length === 1) {
      return geometryFromPayload(shape.features[0]);
    }

    return {
      type: "MultiPolygon",
      coordinates: shape.features.flatMap((feature) => {
        const geometry = geometryFromPayload(feature);
        if (geometry.type === "Polygon") {
          return [geometry.coordinates];
        }
        if (geometry.type === "MultiPolygon") {
          return geometry.coordinates;
        }
        throw new Error(`Unsupported feature geometry type: ${geometry.type}`);
      }),
    };
  }

  if (shape.type === "Polygon" || shape.type === "MultiPolygon") {
    return shape;
  }

  if (shape.geometry) {
    return shape.geometry;
  }

  throw new Error(`Unsupported geometry type: ${shape.type ?? "unknown"}`);
}

function normalizeGeometry(geometry) {
  if (geometry.type === "Polygon") {
    return [geometry.coordinates];
  }

  if (geometry.type === "MultiPolygon") {
    return geometry.coordinates;
  }

  throw new Error(`Unsupported geometry type: ${geometry.type}`);
}

function computeBounds(multiPolygon) {
  let minLon = Infinity;
  let minLat = Infinity;
  let maxLon = -Infinity;
  let maxLat = -Infinity;
  let ringCount = 0;
  let pointCount = 0;

  for (const polygon of multiPolygon) {
    for (const ring of polygon) {
      ringCount += 1;
      for (const [lon, lat] of ring) {
        pointCount += 1;
        minLon = Math.min(minLon, lon);
        minLat = Math.min(minLat, lat);
        maxLon = Math.max(maxLon, lon);
        maxLat = Math.max(maxLat, lat);
      }
    }
  }

  return {
    minLon,
    minLat,
    maxLon,
    maxLat,
    width: maxLon - minLon,
    height: maxLat - minLat,
    ringCount,
    pointCount,
  };
}

function lonLatToScreen(lon, lat) {
  const { centerLon, centerLat, pixelsPerDegree } = state.view;
  const x = elements.canvas.width / 2 + (lon - centerLon) * pixelsPerDegree;
  const y = elements.canvas.height / 2 - (lat - centerLat) * pixelsPerDegree;
  return [x, y];
}

function screenToLonLat(x, y) {
  const { centerLon, centerLat, pixelsPerDegree } = state.view;
  return {
    lon: centerLon + (x - elements.canvas.width / 2) / pixelsPerDegree,
    lat: centerLat - (y - elements.canvas.height / 2) / pixelsPerDegree,
  };
}

function resizeCanvas() {
  const dpr = window.devicePixelRatio || 1;
  const rect = elements.canvas.getBoundingClientRect();
  const width = Math.max(1, Math.floor(rect.width * dpr));
  const height = Math.max(1, Math.floor(rect.height * dpr));

  if (elements.canvas.width !== width || elements.canvas.height !== height) {
    elements.canvas.width = width;
    elements.canvas.height = height;
  }

  context.setTransform(1, 0, 0, 1, 0, 0);
  context.scale(dpr, dpr);
  draw();
}

function drawBackground() {
  const width = elements.canvas.clientWidth;
  const height = elements.canvas.clientHeight;

  const gradient = context.createLinearGradient(0, 0, 0, height);
  gradient.addColorStop(0, "#f9fcff");
  gradient.addColorStop(1, "#dce8f2");
  context.fillStyle = gradient;
  context.fillRect(0, 0, width, height);
}

function drawGraticule() {
  const width = elements.canvas.clientWidth;
  const height = elements.canvas.clientHeight;
  const leftTop = screenToLonLat(0, 0);
  const rightBottom = screenToLonLat(width, height);
  const minLon = Math.floor(Math.min(leftTop.lon, rightBottom.lon) / 10) * 10;
  const maxLon = Math.ceil(Math.max(leftTop.lon, rightBottom.lon) / 10) * 10;
  const minLat = Math.floor(Math.min(leftTop.lat, rightBottom.lat) / 10) * 10;
  const maxLat = Math.ceil(Math.max(leftTop.lat, rightBottom.lat) / 10) * 10;

  context.strokeStyle = "rgba(51, 70, 89, 0.14)";
  context.lineWidth = 1;

  for (let lon = minLon; lon <= maxLon; lon += 10) {
    const [x] = lonLatToScreen(lon, 0);
    context.beginPath();
    context.moveTo(x, 0);
    context.lineTo(x, height);
    context.stroke();
  }

  for (let lat = minLat; lat <= maxLat; lat += 10) {
    const [, y] = lonLatToScreen(0, lat);
    context.beginPath();
    context.moveTo(0, y);
    context.lineTo(width, y);
    context.stroke();
  }
}

function drawPolygonGeometry() {
  if (!state.geometry) {
    return;
  }

  context.fillStyle = "rgba(20, 125, 210, 0.28)";
  context.strokeStyle = "#0c4a78";
  context.lineWidth = 1.4;
  context.beginPath();

  for (const polygon of state.geometry) {
    for (const ring of polygon) {
      ring.forEach(([lon, lat], index) => {
        const [x, y] = lonLatToScreen(lon, lat);
        if (index === 0) {
          context.moveTo(x, y);
        } else {
          context.lineTo(x, y);
        }
      });
      context.closePath();
    }
  }

  context.fill("evenodd");
  context.stroke();
}

function drawBounds() {
  if (!state.bounds) {
    return;
  }

  const topLeft = lonLatToScreen(state.bounds.minLon, state.bounds.maxLat);
  const bottomRight = lonLatToScreen(state.bounds.maxLon, state.bounds.minLat);

  context.save();
  context.strokeStyle = "rgba(12, 74, 120, 0.45)";
  context.setLineDash([6, 6]);
  context.lineWidth = 1;
  context.strokeRect(
    topLeft[0],
    topLeft[1],
    bottomRight[0] - topLeft[0],
    bottomRight[1] - topLeft[1]
  );
  context.restore();
}

function updateViewportReadout() {
  const leftTop = screenToLonLat(0, 0);
  const rightBottom = screenToLonLat(elements.canvas.clientWidth, elements.canvas.clientHeight);
  setDefinitionList(elements.viewport, [
    ["Center", `${state.view.centerLat.toFixed(4)}, ${state.view.centerLon.toFixed(4)}`],
    ["Zoom", `${state.view.pixelsPerDegree.toFixed(2)} px/deg`],
    [
      "Lat Range",
      `${rightBottom.lat.toFixed(2)} to ${leftTop.lat.toFixed(2)}`,
    ],
    [
      "Lon Range",
      `${leftTop.lon.toFixed(2)} to ${rightBottom.lon.toFixed(2)}`,
    ],
  ]);
}

function draw() {
  resizeCanvasBackingOnly();
  drawBackground();
  drawGraticule();
  drawPolygonGeometry();
  drawBounds();
  updateViewportReadout();
}

function resizeCanvasBackingOnly() {
  const dpr = window.devicePixelRatio || 1;
  const width = elements.canvas.clientWidth;
  const height = elements.canvas.clientHeight;
  context.setTransform(dpr, 0, 0, dpr, 0, 0);
  context.clearRect(0, 0, width, height);
}

function fitBounds(bounds) {
  if (!bounds) {
    return;
  }

  const width = Math.max(1, elements.canvas.clientWidth - FIT_PADDING * 2);
  const height = Math.max(1, elements.canvas.clientHeight - FIT_PADDING * 2);
  const xScale = width / Math.max(bounds.width, 0.5);
  const yScale = height / Math.max(bounds.height, 0.5);

  state.view.centerLon = (bounds.minLon + bounds.maxLon) / 2;
  state.view.centerLat = (bounds.minLat + bounds.maxLat) / 2;
  state.view.pixelsPerDegree = Math.min(xScale, yScale);

  if (!Number.isFinite(state.view.pixelsPerDegree) || state.view.pixelsPerDegree <= 0) {
    state.view.pixelsPerDegree = DEFAULT_VIEW.pixelsPerDegree;
  }

  draw();
}

function resetView() {
  state.view = { ...DEFAULT_VIEW };
  draw();
}

function updateMetadata(country) {
  if (!country || !state.bounds) {
    setDefinitionList(elements.metadata, [["State", "No country loaded"]]);
    return;
  }

  setDefinitionList(elements.metadata, [
    ["Country", country.countryName],
    ["ISO", country.isoCountryCode],
    ["ISO3", country.iso3Code ?? "n/a"],
    ["Region", country.region ?? "n/a"],
    ["Continent", country.continent ?? "n/a"],
    ["Points", String(state.bounds.pointCount)],
    ["Rings", String(state.bounds.ringCount)],
    [
      "Bounds",
      `${state.bounds.minLat.toFixed(2)}..${state.bounds.maxLat.toFixed(2)} / ${state.bounds.minLon.toFixed(2)}..${state.bounds.maxLon.toFixed(2)}`,
    ],
  ]);
}

function sortManifest(entries) {
  return [...entries].sort((left, right) =>
    left.countryName.localeCompare(right.countryName, "en", { sensitivity: "base" })
  );
}

function syncSelectOptions() {
  const currentValue = state.selectedCountry?.isoCountryCode ?? "";
  elements.select.replaceChildren();

  for (const country of state.filteredManifest) {
    const option = document.createElement("option");
    option.value = country.isoCountryCode;
    option.textContent = `${country.countryName} (${country.isoCountryCode})`;
    if (country.isoCountryCode === currentValue) {
      option.selected = true;
    }
    elements.select.append(option);
  }

  if (!elements.select.value && state.filteredManifest.length > 0) {
    elements.select.value = state.filteredManifest[0].isoCountryCode;
  }
}

function applySearchFilter() {
  const query = elements.search.value.trim().toLowerCase();
  state.filteredManifest = state.manifest.filter((country) => {
    if (!query) {
      return true;
    }

    return (
      country.countryName.toLowerCase().includes(query) ||
      country.isoCountryCode.toLowerCase().includes(query) ||
      (country.iso3Code ?? "").toLowerCase().includes(query)
    );
  });

  syncSelectOptions();
}

async function loadCountry(isoCountryCode) {
  const country = state.manifest.find((entry) => entry.isoCountryCode === isoCountryCode);
  if (!country) {
    return;
  }

  setStatus(`Loading ${country.countryName}…`);

  const response = await fetch(`${SOURCE_ROOT}/${country.isoCountryCode}.json`);
  if (!response.ok) {
    throw new Error(`Failed to load geometry for ${country.isoCountryCode} from ${SOURCE_ROOT}`);
  }

  const payload = await response.json();
  const geometry = normalizeGeometry(geometryFromPayload(payload));
  const bounds = computeBounds(geometry);

  state.selectedCountry = country;
  state.geometry = geometry;
  state.bounds = bounds;

  updateMetadata(country);
  fitBounds(bounds);
  setStatus(
    `Loaded ${country.countryName} with ${bounds.pointCount.toLocaleString()} points across ${bounds.ringCount} rings`
  );
}

function onSelectChange() {
  const isoCountryCode = elements.select.value;
  if (isoCountryCode) {
    loadCountry(isoCountryCode).catch((error) => {
      setStatus(error.message);
    });
  }
}

function handlePointerDown(event) {
  state.drag = {
    x: event.clientX,
    y: event.clientY,
    centerLon: state.view.centerLon,
    centerLat: state.view.centerLat,
  };

  elements.canvas.setPointerCapture(event.pointerId);
}

function handlePointerMove(event) {
  const rect = elements.canvas.getBoundingClientRect();
  const { lon, lat } = screenToLonLat(event.clientX - rect.left, event.clientY - rect.top);
  elements.cursor.textContent = `${formatCoordinate(lat, "N", "S")} / ${formatCoordinate(lon, "E", "W")}`;

  if (!state.drag) {
    return;
  }

  const dx = event.clientX - state.drag.x;
  const dy = event.clientY - state.drag.y;
  state.view.centerLon = state.drag.centerLon - dx / state.view.pixelsPerDegree;
  state.view.centerLat = state.drag.centerLat + dy / state.view.pixelsPerDegree;
  draw();
}

function handlePointerUp(event) {
  if (state.drag) {
    state.drag = null;
    elements.canvas.releasePointerCapture(event.pointerId);
  }
}

function handleWheel(event) {
  event.preventDefault();
  const rect = elements.canvas.getBoundingClientRect();
  const pointer = screenToLonLat(event.clientX - rect.left, event.clientY - rect.top);
  const zoomFactor = event.deltaY < 0 ? 1.15 : 1 / 1.15;
  const nextScale = Math.min(300, Math.max(0.2, state.view.pixelsPerDegree * zoomFactor));

  state.view.centerLon = pointer.lon - ((event.clientX - rect.left) - rect.width / 2) / nextScale;
  state.view.centerLat = pointer.lat + ((event.clientY - rect.top) - rect.height / 2) / nextScale;
  state.view.pixelsPerDegree = nextScale;
  draw();
}

function installEventHandlers() {
  window.addEventListener("resize", resizeCanvas);

  elements.search.addEventListener("input", () => {
    applySearchFilter();
    if (elements.select.value) {
      void loadCountry(elements.select.value).catch((error) => setStatus(error.message));
    }
  });

  elements.select.addEventListener("change", onSelectChange);
  elements.fitButton.addEventListener("click", () => fitBounds(state.bounds));
  elements.resetButton.addEventListener("click", resetView);

  elements.canvas.addEventListener("pointerdown", handlePointerDown);
  elements.canvas.addEventListener("pointermove", handlePointerMove);
  elements.canvas.addEventListener("pointerup", handlePointerUp);
  elements.canvas.addEventListener("pointerleave", () => {
    elements.cursor.textContent = "";
  });
  elements.canvas.addEventListener("wheel", handleWheel, { passive: false });
  elements.canvas.addEventListener("dblclick", () => fitBounds(state.bounds));
}

async function initialize() {
  installEventHandlers();
  updateMetadata(null);
  resizeCanvas();

  const response = await fetch(MANIFEST_URL);
  if (!response.ok) {
    throw new Error("Failed to load country manifest");
  }

  state.manifest = sortManifest(await response.json());
  state.filteredManifest = [...state.manifest];
  syncSelectOptions();

  const initialCountry = state.manifest.find((country) => country.isoCountryCode === "CA")
    ?? state.manifest[0];

  if (initialCountry) {
    elements.select.value = initialCountry.isoCountryCode;
    await loadCountry(initialCountry.isoCountryCode);
  } else {
    setStatus("No countries found in manifest");
  }
}

initialize().catch((error) => {
  setStatus(error.message);
});
