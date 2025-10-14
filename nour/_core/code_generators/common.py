import os

NR_BOOL = 0
NR_INT8 = 1
NR_UINT8 = 2
NR_INT16 = 3
NR_UINT16 = 4
NR_INT32 = 5
NR_UINT32 = 6
NR_INT64 = 7
NR_UINT64 = 8
NR_FLOAT32 = 9
NR_FLOAT64 = 10
NR_NUM_NUMIRC_DT = 11

alldtypes = [
    NR_BOOL,
    NR_INT8,
    NR_UINT8,
    NR_INT16,
    NR_UINT16,
    NR_INT32,
    NR_UINT32,
    NR_INT64,
    NR_UINT64,
    NR_FLOAT32,
    NR_FLOAT64,
]

itypes = [
    NR_BOOL,
    NR_INT8,
    NR_UINT8,
    NR_INT16,
    NR_UINT16,
    NR_INT32,
    NR_UINT32,
    NR_INT64,
    NR_UINT64
]

ftypes = [
    NR_FLOAT32,
    NR_FLOAT64
]

dtype2name = {
    NR_BOOL : "NR_BOOL",
    NR_INT8 : "NR_INT8",
    NR_UINT8 : "NR_UINT8",
    NR_INT16 : "NR_INT16",
    NR_UINT16 : "NR_UINT16",
    NR_INT32 : "NR_INT32",
    NR_UINT32 : "NR_UINT32",
    NR_INT64 : "NR_INT64",
    NR_UINT64 : "NR_UINT64",
    NR_FLOAT32 : "NR_FLOAT32",
    NR_FLOAT64 : "NR_FLOAT64",
}

dtype2nr_type = {
    NR_BOOL : "nr_bool",
    NR_INT8 : "nr_int8",
    NR_UINT8 : "nr_uint8",
    NR_INT16 : "nr_int16",
    NR_UINT16 : "nr_uint16",
    NR_INT32 : "nr_int32",
    NR_UINT32 : "nr_uint32",
    NR_INT64 : "nr_int64",
    NR_UINT64 : "nr_uint64",
    NR_FLOAT32 : "nr_float32",
    NR_FLOAT64 : "nr_float64",
}

THIS_DIR = os.path.dirname(__file__)
SRC_DIR = os.path.join(THIS_DIR, "../src")
# SRC_NOUR_DIR = os.path.join(SRC_DIR, "nour")
SRC_NOUR_DIR = SRC_DIR
NMATH_DIR = os.path.join(SRC_DIR, "nmath")
BUILD_DIR = os.path.join(SRC_NOUR_DIR, "build")
GEN_DIR = os.path.join(BUILD_DIR, "generated")
MAGIC_WORD = "//Template//\n"

def create_centered_header(header, total_width=50, frame_letter = "="):
    """
    Create a centered header with the given header, ensuring that
    the header fits within the specified total width.
    """
    total_width = max(total_width, len(header) + 4)
    equal_line = frame_letter * total_width
    centered_longname = header.center(total_width)
    header = f"""\
/*
{equal_line}
{centered_longname}
{equal_line}
*/
"""
    return header