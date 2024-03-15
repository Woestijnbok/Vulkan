#include "HelperStructs.h"

bool QueueFamilyIndices::IsComplete()
{
	return GraphicsFamily.has_value();
}