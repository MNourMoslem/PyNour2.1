#ifndef NOUR__CORE_SRC_NMATH_LOOPS_H__
#define NOUR__CORE_SRC_NMATH_LOOPS_H__

#include "nour/nour.h"

#define NMATH_LOOP_CCC(op, out_type, in_type) do {\
    in_type* n1_dataptr = (in_type*)n1->data;\
    in_type* n2_dataptr = (in_type*)n2->data;\
    out_type* out_dataptr = (out_type*)out->data;\
    nr_long nitems = Node_NItems(out);\
    for (nr_long i = 0; i < nitems; i++) {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)(n1_dataptr + i), \
                                            *(in_type*)(n2_dataptr + i));\
    }\
} while (0)

#define NMATH_LOOP_CSC(op, out_type, in_type) do {\
    Node* con_n = n1c ? n1 : n2;\
    Node* str_n = n1c ? n2 : n1;\
    NIter it;\
    NIter_FromNode(&it, str_n, NITER_MODE_STRIDED);\
    in_type* n_dataptr = (in_type*)con_n->data;\
    out_type* out_dataptr = (out_type*)out->data;\
    NIter_ITER((&it));\
    nr_long i = 0;\
    while (NIter_NOTDONE((&it)))\
    {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)NIter_ITEM((&it)),\
                                            *(in_type*)(n_dataptr + i));\
        i++;\
        NIter_NEXT_STRIDED(&it);\
    }\
} while(0)

#define NMATH_LOOP_CSS(op, out_type, in_type) do {\
    NIter it1;\
    NIter it2;\
    NIter_FromNode(&it1, n1, NITER_MODE_STRIDED);\
    NIter_FromNode(&it2, n2, NITER_MODE_STRIDED);\
    out_type* out_dataptr = (out_type*)out->data;\
    NIter_ITER(&it1);\
    NIter_ITER(&it2);\
    nr_long i = 0;\
    while (NIter_NOTDONE(&it1))\
    {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)NIter_ITEM(&it1),\
                                        *(in_type*)NIter_ITEM(&it2));\
        i++;\
        NIter_NEXT_STRIDED(&it1);\
        NIter_NEXT_STRIDED(&it2);\
    }\
} while(0)

#define NMATH_LOOP_SSS(op, out_type, in_type) do {\
    NIter nit1, nit2, oit;\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    NIter_FromNode(&nit1, n1, NITER_MODE_STRIDED);\
    NIter_FromNode(&nit2, n2, NITER_MODE_STRIDED);\
\
    NIter_ITER(&oit);\
    NIter_ITER(&nit1);\
    NIter_ITER(&nit2);\
    while (NIter_NOTDONE(&oit))\
    {\
        *(out_type*)NIter_ITEM(&oit) = op(*(in_type*)NIter_ITEM(&nit1),\
                                        *(in_type*)NIter_ITEM(&nit2));\
        NIter_NEXT_STRIDED(&oit);\
        NIter_NEXT_STRIDED(&nit1);\
        NIter_NEXT_STRIDED(&nit2);\
    }\
} while(0)

#define NMATH_LOOP_SCC(op, out_type, in_type) do {\
    NIter oit;\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    nr_long i = 0;\
    while (NIter_NOTDONE(&oit))\
    {\
        /* cast the void* data to in_type* BEFORE doing pointer arithmetic */\
        *(out_type*)NIter_ITEM(&oit) = op( *(((in_type*)n1->data) + i), \
                                          *(((in_type*)n2->data) + i) );\
        i++;\
        NIter_NEXT_STRIDED(&oit);\
    }\
} while(0)

#define NMATH_LOOP_SSC(op, out_type, in_type) do {\
    Node* con_n = n1c ? n1 : n2;\
    Node* str_n = n1c ? n2 : n1;\
    NIter oit, nit;\
    NIter_FromNode(&nit, str_n, NITER_MODE_STRIDED);\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    in_type* n_dataptr = (in_type*)con_n->data;\
    nr_long i = 0;\
    while (NIter_NOTDONE(&oit))\
    {\
        *(out_type*)NIter_ITEM(&oit) = op(*(in_type*)(n_dataptr + i),\
                                            *(in_type*)NIter_ITEM(&nit));\
        i++;\
        NIter_NEXT_STRIDED(&oit);\
        NIter_NEXT_STRIDED(&nit);\
    }\
} while(0)

#define NMATH_LOOP_CC_S(op, out_type, in_type) do {\
    in_type* n_dataptr = (in_type*)n->data;\
    out_type* out_dataptr = (out_type*)out->data;\
    nr_long nitems = Node_NItems(out);\
    for (nr_long i = 0; i < nitems; i++) {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)(n_dataptr + i), sclr);\
    }\
} while (0)

#define NMATH_LOOP_CS_S(op, out_type, in_type) do {\
    NIter nit;\
    NIter_FromNode(&nit, n, NITER_MODE_STRIDED);\
    out_type* out_dataptr = (out_type*)out->data;\
    NIter_ITER(&nit);\
    nr_long i = 0;\
    while (NIter_NOTDONE(&nit))\
    {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)NIter_ITEM(&nit), sclr);\
        i++;\
        NIter_NEXT_STRIDED(&nit);\
    }\
} while(0)

#define NMATH_LOOP_SC_S(op, out_type, in_type) do {\
    NIter oit;\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    in_type* n_dataptr = (in_type*)n->data;\
    NIter_ITER(&oit);\
    nr_long i = 0;\
    while (NIter_NOTDONE(&oit))\
    {\
        *(out_type*)NIter_ITEM(&oit) = op(*(in_type*)(n_dataptr + i), sclr);\
        i++;\
        NIter_NEXT_STRIDED(&oit);\
    }\
} while(0)

#define NMATH_LOOP_SS_S(op, out_type, in_type) do {\
    NIter oit, nit;\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    NIter_FromNode(&nit, n, NITER_MODE_STRIDED);\
    NIter_ITER(&oit);\
    NIter_ITER(&nit);\
    while (NIter_NOTDONE(&oit))\
    {\
        *(out_type*)NIter_ITEM(&oit) = op(*(in_type*)(NIter_ITEM(&nit)), sclr);\
        NIter_NEXT_STRIDED(&oit);\
        NIter_NEXT_STRIDED(&nit);\
    }\
} while(0)


#define NMATH_LOOP_CC_1I(op, out_type, in_type) do {\
    in_type* n1_dataptr = (in_type*)n1->data;\
    out_type* out_dataptr = (out_type*)out->data;\
    nr_long nitems = Node_NItems(out);\
    for (nr_long i = 0; i < nitems; i++) {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)(n1_dataptr + i));\
    }\
} while (0)


#define NMATH_LOOP_CS_1I(op, out_type, in_type) do {\
    NIter nit;\
    NIter_FromNode(&nit, n1, NITER_MODE_STRIDED);\
    out_type* out_dataptr = (out_type*)out->data;\
    NIter_ITER(&nit);\
    nr_long i = 0;\
    while (NIter_NOTDONE(&nit))\
    {\
        *(out_type*)(out_dataptr + i) = op(*(in_type*)NIter_ITEM(&nit));\
        i++;\
        NIter_NEXT_STRIDED(&nit);\
    }\
} while(0)

#define NMATH_LOOP_SC_1I(op, out_type, in_type) do {\
    NIter oit;\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    in_type* n1_dataptr = (in_type*)n1->data;\
    NIter_ITER(&oit);\
    nr_long i = 0;\
    while (NIter_NOTDONE(&oit))\
    {\
        *(out_type*)NIter_ITEM(&oit) = op(*(in_type*)(n1_dataptr + i));\
        i++;\
        NIter_NEXT_STRIDED(&oit);\
    }\
} while(0)

#define NMATH_LOOP_SS_1I(op, out_type, in_type) do {\
    NIter oit, nit;\
    NIter_FromNode(&oit, out, NITER_MODE_STRIDED);\
    NIter_FromNode(&nit, n1, NITER_MODE_STRIDED);\
    NIter_ITER(&oit);\
    NIter_ITER(&nit);\
    while (NIter_NOTDONE(&oit))\
    {\
        *(out_type*)NIter_ITEM(&oit) = op(*(in_type*)(NIter_ITEM(&nit)));\
        NIter_NEXT_STRIDED(&oit);\
        NIter_NEXT_STRIDED(&nit);\
    }\
} while(0)

#endif