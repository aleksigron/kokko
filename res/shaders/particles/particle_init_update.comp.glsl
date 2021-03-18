
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
    // Load dead particle count:
	uint dead_count = atomicOr(counters.dead_count, 0);

	// Load alive particle count:
	uint new_alive_count = atomicOr(counters.post_sim_alive_count, 0);

	uint real_emit_count = min(dead_count, uniforms.emit_count);
    uint simulate_count = new_alive_count + real_emit_count;

	// Fill dispatch argument buffer for emitting (ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ):
    uint emit_groups = (real_emit_count + GROUP_SIZE_EMIT - 1) / GROUP_SIZE_EMIT;
    indirect_params[INDIRECT_INDEX_EMIT] = uvec4(emit_groups, 1, 1, 0);

	// Fill dispatch argument buffer for simulation (ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ):
    uint simulate_groups = (simulate_count + GROUP_SIZE_SIMULATE - 1) / GROUP_SIZE_SIMULATE;
    indirect_params[INDIRECT_INDEX_SIMULATE] = uvec4(simulate_groups, 1, 1, 0);

	// copy new alivelistcount to current alivelistcount:
    atomicExchange(counters.alive_count, new_alive_count);

	// reset new alivecount:
    atomicExchange(counters.post_sim_alive_count, 0);

	// and write real emit count:
    atomicExchange(counters.emit_count, real_emit_count);
}
