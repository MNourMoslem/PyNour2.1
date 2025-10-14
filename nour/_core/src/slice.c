#include "slice.h"
#include "nerror.h"

int 
Node_Slice(Node* node, const Slice slice, int dim){
    if (NODE_NDIM(node) == 0 || dim < 0 || dim >= NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError ,"Invalid dimension for slicing. got %d for array with %d dimensions.", dim, NODE_NDIM(node));
        return -1;
    }

    nr_intp start = slice.start;
    nr_intp stop = slice.stop;
    nr_intp step = slice.step;

    nr_intp dim_size = NODE_SHAPE(node)[dim];

    if (start < 0) start += dim_size;
    if (stop < 0) stop += dim_size;
    if (step == 0) step = 1;

    if (start < 0 || start >= dim_size || stop < 0 || stop > dim_size || step == 0) {
        NError_RaiseError(NError_IndexError ,"Invalid slice parameters.");
        return -1;
    }

    // Now we change the shape and strides
    nr_intp new_dim_size = (stop - start + step - 1) / step;
    if (new_dim_size < 0) new_dim_size = 0;
    NODE_SHAPE(node)[dim] = new_dim_size;
    NODE_STRIDES(node)[dim] *= step;

    // Lastly, we adjust the type of the node to strided if not already
    if (!NODE_IS_STRIDED(node)){
        NR_SETFLG(node->flags, NR_NODE_STRIDED);
        NR_RMVFLG(node->flags, NR_NODE_CONTIGUOUS);
    }

    return 0;
}