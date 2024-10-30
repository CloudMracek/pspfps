#include "bwm.h"
#include <stdio.h>

BWM_Header* bwm_header(void *data) {
	return (BWM_Header*) data;
}

BWM_Vertex* bwm_vertices(void *data) {
	return (BWM_Vertex*) (data+sizeof(BWM_Header));
}

BWM_VertexIndice* bwm_vertex_indices(void *data) {
	BWM_Header* header = data;
	return (BWM_VertexIndice*) (data
		+sizeof(BWM_Header)
		+(sizeof(BWM_Vertex)*header->vertex_count));
}
