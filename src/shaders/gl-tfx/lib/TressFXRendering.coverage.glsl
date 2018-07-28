// START TressFXRendering.coverage

// Calculate the pixel coverage of a hair strand by computing the hair width
// p0, p1, pixelLoc are in d3d clip space (-1 to 1)x(-1 to 1)
//
// @param vec2 p0 - position of 'left' vertex after perspective projection
// @param vec2 p1 - position of 'right' vertex after perspective projection
float ComputeCoverage(vec2 p0, vec2 p1, vec2 pixelLoc, vec2 winSize) {
    // Scale positions so 1.f = half pixel width
    p0 *= winSize;
    p1 *= winSize;

    float p0dist = length(p0 - pixelLoc);
    float p1dist = length(p1 - pixelLoc);
    float hairWidth = length(p0 - p1);

    // if outside, set sign to -1, else set sign to 1
    float sign = (p0dist < hairWidth || p1dist < hairWidth) ? -1 : 1;

    // signed distance (positive if inside hair, negative if outside hair)
    float relDist = sign * clamp(min(p0dist, p1dist), 0, 1);

    // returns coverage based on the relative distance
    // 0, if completely outside hair edge
    // 1, if completely inside hair edge
    return (relDist + 1.f) * 0.5f;
}


// END TressFXRendering.coverage
