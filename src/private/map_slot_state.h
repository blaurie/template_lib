#ifndef TEMPLATE_LIB_MAP_NODE_STATE_H
#define TEMPLATE_LIB_MAP_NODE_STATE_H

enum tl_map_slot_state
{
	TL_MAPSS_EMPTY = 0,	// must be 0!!
	TL_MAPSS_DELETED,
	TL_MAPSS_OCCUPIED,
	TL_MAPSS_COLLIDED
};

#endif //TEMPLATE_LIB_MAP_NODE_STATE_H
