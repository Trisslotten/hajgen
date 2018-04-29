#pragma once

#include "heightmap.hpp"

const int ERODE_CHUNKS = 5;


void erodeCenterHeightmap(Heightmap* maps[ERODE_CHUNKS * ERODE_CHUNKS]);
