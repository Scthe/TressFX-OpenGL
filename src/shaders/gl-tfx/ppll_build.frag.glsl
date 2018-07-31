#version 450

#pragma include "_utils.glsl"
#pragma include "lib/TressFXPPLL.glsl"
#pragma include "lib/TressFXPPLL.build.glsl"
#pragma include "lib/TressFXRendering.coverage.glsl"

// START ppll_build.frag
// (main)

// TODO Crassin's page allocator?

uniform vec2 g_WinSize;

in vec4 ps_Position;
in vec4 ps_Tangent;
in vec4 ps_p0p1;
in vec4 ps_strandColor;
// out vec4 out_color;

// NOTE: very important
// Remember we are not writing fragment color, but for all fragments we write
// to SSBO / image2d. Normally, depth stencil can be done whatever (exp. when
// fragment's depth is modified in pixel shader). But in our case, if depth/stencil
// is done too late, we already written to SSBO etc. so yeah, do early
// depth/stencil here
layout(early_fragment_tests) in; // [earlydepthstencil]


float get_alpha () {
	float coverage = ComputeCoverage(ps_p0p1.xy, ps_p0p1.zw, gl_FragCoord.xy, g_WinSize);
	return coverage * ps_strandColor.a;
}

void main () {
	float alpha = get_alpha(); // 1.0;
	if (alpha < 1.0 / 255.0) {
		discard;
	}

	// Allocate a new fragment in heads texture
	ivec2 vScreenAddress = ivec2(gl_FragCoord.xy);
	uint nNewFragmentAddress = AllocateFragment(vScreenAddress);

	if (nNewFragmentAddress != FRAGMENT_LIST_NULL) {
		uint nOldFragmentAddress = MakeFragmentLink(vScreenAddress, nNewFragmentAddress);
		WriteFragmentAttributes(
			nNewFragmentAddress,
			nOldFragmentAddress,
			vec4(ps_Tangent.xyz * 0.5 + vec3(0.5, 0.5, 0.5), alpha), // data
			ps_strandColor.xyz, // color
			ps_Position.z // depth
		);
	}

	// out_color = vec4(0.16, 0.45, 0.64, 1.0);
}

// paging algo breakdown(very rough draft):
// 1) pointer = curr_head[x,y]
// 2) for i in [:PAGE] // or we can store in pointer current page offset
//      if linked_list[pointer+i] is unused: (should this be atomic?)
//        write here; break;
//      else:
//        addres = atomic_inc * 4
//        do the swap in head_texture
//        write to linked_list, update old pointer
//
