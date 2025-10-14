from common import *
from typing import List, Dict
import textwrap
import os

DOC = """\
/**
 * Converts data from a source node with type {ST} to a
 * destination node with type {DT} and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type {DT}. If NULL, a new node is created.
 *   - src: The source node with type {ST} containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `{ST}` to `{DT}` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */"""

FUNC_TEMP = "Node_TypeConvert_{ST}_to_{DT}"
FUNC_HEADER = "Type Convert Methods For {DT}"
COPY_FUNC = "Node_Copy"

class TCFunction:
    def __init__(self, src_type : int, dst_type : int):
        self.src_t = dtype2name[src_type]
        self.src_nt = dtype2nr_type[src_type]
        self.dst_t = dtype2name[dst_type]
        self.dst_nt = dtype2nr_type[dst_type]

    def get_doc(self):
        return textwrap.dedent(DOC.format(DT = self.dst_t, ST = self.src_t))
    
    def get_func_name(self):
        return FUNC_TEMP.format(DT = self.dst_t, ST = self.src_t)

class TCFunctionList:
    def __init__(self, src_types : List[int], dst_type : int):
        src_types = src_types if src_types else alldtypes

        self.dst_t = dtype2name[dst_type]
        self.dst_ti = dst_type
        
        self.functions = []
        for src_type in src_types:
            if src_type != dst_type:
                self.functions.append(TCFunction(src_type, dst_type))

    def get_header(self):
        return FUNC_HEADER.format(DT = self.dst_t)

def get_templates(fp : str):
    with open(fp, 'r') as f:
        content = f.read().split(MAGIC_WORD)

    return {
        "Header" : content[1],
        "Function" : content[3],
        "Array": content[4]
    }

def get_functions(f_template : str, fl : TCFunctionList):
    code = ""
    for f in fl.functions:
        code += f.get_doc()
        code += f_template.replace("%DT%", f.dst_t)\
                            .replace("%NDT%", f.dst_nt)\
                            .replace("%ST%", f.src_t)\
                            .replace("%NST%", f.src_nt)
    return code

def get_array(a_template : str, fl_list : List[TCFunctionList]):
    all_ = []
    for fl in fl_list:
        all_.append(f"\n\t//To {fl.dst_t}\n")
        dst_type = fl.dst_ti
        iter_ = iter(fl.functions)
        i = 0
        while i < NR_NUM_NUMIRC_DT:
            if i == dst_type:
                text = COPY_FUNC
            else:
                try:
                    f = next(iter_)
                    text = f.get_func_name() 
                except:
                    break
            all_.append("\t" + text + ",\n")
            i+=1

    code = "".join(all_)
    code = a_template.replace("%METHODS%", code)

    return code

def get_code(src_file_path : str ,fl_list : List[TCFunctionList]):
    templates = get_templates(src_file_path)

    code = templates['Header']

    f_template = templates["Function"]
    a_template = templates["Array"]

    for fl in fl_list:
        code += create_centered_header(fl.get_header())
        code += get_functions(f_template, fl)

    code += get_array(a_template, fl_list)

    return code

def main():
    src_file = os.path.join(SRC_NOUR_DIR, "tc_methods.c.src")

    tcfl_list = [
        TCFunctionList(None, dst_type) for dst_type in alldtypes
    ]

    code = get_code(src_file, tcfl_list)

    trg_path = GEN_DIR
    if not os.path.isdir(trg_path):
        os.mkdir(trg_path)

    trg_file = '__tc_methods_generated.c'
    trg_path = os.path.join(trg_path, trg_file)

    with open(trg_path, 'w') as f:
        f.write(code)

if __name__ == '__main__':
    main()


    
