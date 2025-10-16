import textwrap
import os
import argparse
from typing import List, Dict

from common import *

SRC_C_FILE = os.path.join(NMATH_DIR, "math_basic.c.src")
SRC_H_FILE = os.path.join(NMATH_DIR, "math_basic.h.src")

DOC = """\
    /**
    * Performs `{LN}` on two input nodes with type {IT} and stores the result in the output node.
    *
    * Parameters:
    *   - args: A pointer to `NFuncArgs`, containing:
    *       - nodes[0]: The first input node with type {IT} (`n1`).
    *       - nodes[1]: The second input node with type {IT} (`n2`).
    *       - out: The output node with type {OT} to store the result.
    * Return:
    *   - 0 on success.
    *   - -1 if memory allocation or iteration fails.
    */
"""

io_none = 0
io_bool = 1
io_int = 2
io_float = 4

iotypes = {
    io_none : "NMATH_IOTYPE_NONE",
    io_bool : "NMATH_IOTYPE_BOOL",
    io_int : "NMATH_IOTYPE_INT",
    io_float : "NMATH_IOTYPE_FLOAT",
}

class MathFunction:
    def __init__(self, name : str, longname : str, out_dtype : int, in_dtype : int = None):
        self.name = name
        self.longname = longname

        self.out_type = dtype2name[out_dtype]
        self.out_nr_type = dtype2nr_type[out_dtype]

        if in_dtype:
            self.in_type = dtype2name[in_dtype]
            self.in_nr_type = dtype2nr_type[in_dtype]
        else:
            self.in_type = self.out_type
            self.in_nr_type = self.out_nr_type
            
    def get_doc(self):
        return textwrap.dedent(DOC.format(LN = self.longname,
                                 IT = self.in_type, OT = self.out_type))

class MathFunctionList:
    def __init__(
                self,
                name : str,
                longname : str,
                out_iotype : int,
                in_iotype : int,
                out_dtype_list : list = [],
                in_dtype_list : list = []
                ):
        
        if not out_dtype_list:
            if out_iotype == io_bool:
                out_dtype_list = [NR_BOOL]
            elif out_iotype == io_float:
                out_dtype_list = ftypes
            elif out_iotype == io_int:
                out_dtype_list = itypes
            else:
                out_dtype_list = alldtypes
        
        if not in_dtype_list:
            if in_iotype == io_bool:
                in_dtype_list = [NR_BOOL]
            elif in_iotype == io_float:
                in_dtype_list = ftypes
            elif in_iotype == io_int:
                in_dtype_list = itypes
            else:
                in_dtype_list = alldtypes 

        self.functions = [
            MathFunction(name, longname, out, in_) 
            for out, in_ in zip(out_dtype_list ,in_dtype_list)
        ]
        self.name = name
        self.longname = longname

        self.out_iotype = out_iotype
        self.in_iotype = in_iotype

    def get_header(self):
        return textwrap.dedent(create_centered_header(self.longname))
    
    def get_ioout(self):
        return iotypes[self.out_iotype]
    
    def get_ioin(self):
        return iotypes[self.in_iotype]


def get_doc(math_func : MathFunction):
    return textwrap.dedent(DOC.format(OP = math_func.name, T = math_func.in_type))

def get_c_templates(f : str):
    with open(f, 'r') as F:
        context = F.read()

    temps = context.split(MAGIC_WORD)
    return {
        "Header" : temps[0],
        "Function" : temps[1],
        "Array" : temps[2],
        "Main" : temps[3]
    }

def get_c_function(template : str, fl : MathFunctionList):
    code = ""
    for f in fl.functions:
        code += f.get_doc()
        code += template.replace("%OP%", f.name)\
                        .replace("%T%", f.in_type)\
                        .replace("%I_NT%", f.in_nr_type)\
                        .replace("%O_NT%", f.out_nr_type)\
                        .replace("%OPC%", f.name.upper())
    return code

def get_c_array(template : str, fl : MathFunctionList):
    all_ = []
    for f in fl.functions:
        all_.append(f"\tNMath_{f.name}_{f.in_type}")

    code = ",\n".join(all_)
    code = template.replace("%METHODS%", code)\
                    .replace("%OPC%", fl.name.upper())

    return code

def get_c_mainfunction(template : str, fl : MathFunctionList):
    return template.replace("%OP%", fl.name)\
                    .replace("%OPC%", fl.name.upper())\
                    .replace("%OUTTYPE%", fl.get_ioout())\
                    .replace("%INTYPE%", fl.get_ioin())\

def get_c_code(fl_list : List[MathFunctionList]):
    templates = get_c_templates(SRC_C_FILE)
    
    code = templates['Header']

    f_temp = templates['Function']
    a_temp = templates['Array']
    m_temp = templates['Main']

    for fl in fl_list:
        code += fl.get_header()
        code += get_c_function(f_temp, fl)
        code += get_c_array(a_temp, fl)
        code += get_c_mainfunction(m_temp, fl)

    return code

def get_h_code(fl_list : List[MathFunctionList]):
    with open(SRC_H_FILE, 'r') as f:
        content = f.read().split(MAGIC_WORD)

    tempalte = content[1]

    code = content[0]
    for fl in fl_list:
        code += tempalte.replace("%OP%", fl.name)
    code += content[2]

    return code


def main():
    boollist = [NR_BOOL] * NR_NUM_NUMIRC_DT

    fl_list = [
        MathFunctionList("Add", "Addition", io_none, io_none, alldtypes, alldtypes),
        MathFunctionList("Mul", "Multiplication", io_none, io_none, alldtypes, alldtypes),
        MathFunctionList("Sub", "Subtraction", io_none, io_none, alldtypes, alldtypes),
        MathFunctionList("Div", "Division", io_float, io_float, ftypes, ftypes),
        # MathFunctionList("Pow", "Power", None),
        MathFunctionList("Mod", "Module", io_int, io_int, itypes),
        MathFunctionList("Tdv", "True Division", io_int, io_int, itypes),
        MathFunctionList("Bg", "Bigger Then", io_bool, io_none, boollist, alldtypes),
        MathFunctionList("Bge", "Bigger Then Or Equal", io_bool, io_none, boollist, alldtypes),
        MathFunctionList("Ls", "Less Then", io_bool, io_none, boollist, alldtypes),
        MathFunctionList("Lse", "Less Then Or Equal", io_bool, io_none, boollist, alldtypes),
        MathFunctionList("Eq", "Equal", io_bool, io_none, boollist, alldtypes),
        MathFunctionList("Neq", "Not Equal", io_bool, io_none, boollist, alldtypes),
    ]

    code = get_c_code(fl_list)
    cf_path = GEN_DIR
    if not os.path.isdir(cf_path):
        os.mkdir(cf_path)
    cf_file = '__nmath_generated.c'
    cf_path = os.path.join(cf_path, cf_file)
    with open(cf_path, 'w') as f:
        f.write(code)


    code = get_h_code(fl_list)
    hf_path = GEN_DIR
    if not os.path.isdir(hf_path):
        os.mkdir(hf_path)
    gf_file = '__nmath_generated.h'
    hf_path = os.path.join(hf_path, gf_file)
    with open(hf_path, 'w') as f:
        f.write(code)
    
if __name__ == '__main__':
    main()