#version 450

#pragma include "_utils.glsl"
#pragma include "lib/TressFXPPLL.glsl"
#pragma include "lib/TressFXPPLL.build.glsl"

// START ppll_build.frag
// (main)

// uniform mat4 g_mInvViewProj; // unused ATM
// uniform vec4 g_vViewport; // unused ATM

in vec4 ps_Position;
in vec4 ps_Tangent;
in vec4 ps_p0p1;
in vec3 ps_strandColor;
// in vec4 ps_test;
out vec4 out_color;

void main () {
	float alpha = 1.0; // get_alpha();

	// Allocate a new fragment in heads texture
	ivec2 vScreenAddress = ivec2(gl_FragCoord.xy); // ivec2(ps_Position.xy);
	uint nNewFragmentAddress = AllocateFragment(vScreenAddress); // TODO Crassin's page allocator?
	// ASSERT ( nNewFragmentAddress != FRAGMENT_LIST_NULL )

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


	// test:
	// uint prevHead = imageAtomicExchange(tRWFragmentListHead, ivec2(gl_FragCoord.xy), 1);
	// if (out_color.r < 0 && out_color.r > 1000 + prevHead){
		// uint val = uint(LinkedListUAV_[0].depth) + nNodePoolSize;
		// imageAtomicExchange(tRWFragmentListHead, ivec2(1,1), val);
	// }

	out_color = vec4(ps_strandColor, 1.0);
	// out_color = vec4(1.0, 0.0, 0.0, 1.0);
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

/*
float get_alpha () {
	vec3 vNDC = ScreenToNDC(ps_Position.xyz, g_vViewport);
	vec3 vPositionWS = NDCToWorld(vNDC, g_mInvViewProj);
	float coverage = ComputeCoverage(ps_p0p1.xy, ps_p0p1.zw, vNDC.xy, g_vViewport.zw);
	float alpha = coverage*g_MatBaseColor.a;

	// ASSERT(coverage >= 0)
	if (alpha < 1.0 / 255.0) {
		// RETURN_NOTHING
		discard;
	}
}
*/
