import os
import sys
import subprocess
import shutil
import platform

CORE_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__), "nour", "_core"))
SRC_DIR = os.path.join(CORE_DIR, "src")

BUILD_DIR = os.path.join(SRC_DIR, "build")
LIB_DIR = os.path.join(BUILD_DIR, "lib")
CODE_GENERATED_DIR = os.path.join(BUILD_DIR, "generated")
OBJ_DIR = os.path.join(BUILD_DIR, "obj")

CODE_GENERATORS_DIR = os.path.join(CORE_DIR, "code_generators")
CODE_GENERATOR_MAIN_FILE_NAME = "main.py"

TESTS_DIR = os.path.join(CORE_DIR, "tests")
TEST_MAIN_FILE_NAME = "main.c"

INCLUDE_DIRS = [
    os.path.join(CORE_DIR, "include"),
]


def create_directories():
    """Ensure build-related directories exist."""
    for d in (BUILD_DIR, OBJ_DIR, LIB_DIR, CODE_GENERATED_DIR):
        # delete existing contents for a clean build
        if os.path.exists(d):
            shutil.rmtree(d)

        os.makedirs(d, exist_ok=True)


def list_all_c_files(directory):
    c_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".c"):
                c_files.append(os.path.join(root, file))
    return c_files


def run_code_generator(file_name):
    """Run a Python code generator located in the project's core code_generators dir."""
    generator_path = os.path.join(CODE_GENERATORS_DIR, file_name)
    if os.path.exists(generator_path):
        print(f"Running code generator: {generator_path}")
        try:
            subprocess.run([sys.executable, generator_path], check=True)
        except subprocess.CalledProcessError as e:
            print(f"Code generator failed: {e}")
    else:
        print(f"Code generator {file_name} not found in {CODE_GENERATORS_DIR}")


def compile_c_files(c_files):
    """Compile .c files to object files in OBJ_DIR and return list of object paths."""
    obj_files = []
    if not c_files:
        return obj_files

    for c_file in c_files:
        base = os.path.basename(c_file)
        output_file = os.path.join(OBJ_DIR, base.replace('.c', '.o'))
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        # build include flags
        include_flags = []
        for inc in INCLUDE_DIRS:
            include_flags.extend(["-I", inc])

        cmd = ["gcc", "-c", c_file, "-o", output_file, "-fPIC"] + include_flags
        print("Compiling:", " ".join(cmd))
        subprocess.run(cmd, check=True)
        obj_files.append(output_file)
    return obj_files

def run_tests():
    """Compile and run tests located in the TESTS_DIR."""
    # Gather all .c files in the tests directory
    test_c_files = list_all_c_files(TESTS_DIR)
    if not test_c_files:
        print(f"No test .c files found in {TESTS_DIR}. Skipping tests.")
        return

    # Ensure main.c exists
    test_main_path = os.path.join(TESTS_DIR, TEST_MAIN_FILE_NAME)
    if not os.path.exists(test_main_path):
        print(f"No test main file found at {test_main_path}. Skipping tests.")
        return

    include_flags = []
    for inc in INCLUDE_DIRS:
        include_flags.extend(["-I", inc])

    # Compile each test .c file to an object in OBJ_DIR
    test_obj_files = []
    for c_file in test_c_files:
        base = os.path.basename(c_file)
        obj_file = os.path.join(OBJ_DIR, base.replace('.c', '.o'))
        cmd_compile = ["gcc", "-c", c_file, "-o", obj_file, "-fPIC"] + include_flags
        print("Compiling test file:", " ".join(cmd_compile))
        subprocess.run(cmd_compile, check=True)
        test_obj_files.append(obj_file)

    # Link the test executable from all test objects. Ensure test main is last if symbol resolution matters.
    test_executable = os.path.join(BUILD_DIR, "test_executable")
    if platform.system() == "Windows":
        test_executable += ".exe"

    # If a library was created, prefer linking against it as well
    lib_path = None
    # look for libnour.a or libnour.so or nour.dll
    candidates = [os.path.join(LIB_DIR, "libnour.a"), os.path.join(LIB_DIR, "libnour.so"), os.path.join(LIB_DIR, "nour.dll")]
    for cand in candidates:
        if os.path.exists(cand):
            lib_path = cand
            break

    # Order objects: other test objects first, then main (so main's references resolve to test implementations)
    # Ensure the main's object is present in test_obj_files
    main_obj = os.path.join(OBJ_DIR, TEST_MAIN_FILE_NAME.replace('.c', '.o'))
    if main_obj not in test_obj_files:
        # If main.c compiled earlier, it will be in the list; otherwise compile it explicitly
        if os.path.exists(test_main_path):
            cmd_compile = ["gcc", "-c", test_main_path, "-o", main_obj, "-fPIC"] + include_flags
            print("Compiling test main:", " ".join(cmd_compile))
            subprocess.run(cmd_compile, check=True)
            test_obj_files.append(main_obj)

    # Put main at the end
    ordered_objs = [o for o in test_obj_files if o != main_obj] + ([main_obj] if os.path.exists(main_obj) else [])

    cmd_link = ["gcc"] + ordered_objs + ["-o", test_executable]
    if lib_path:
        cmd_link += [lib_path]

    print("Linking test executable:", " ".join(cmd_link))
    subprocess.run(cmd_link, check=True)

    # Run the tests
    print("Running tests...")
    subprocess.run([test_executable], check=True)

def create_library(obj_files, lib_basename="nour"):
    """Create a static library (preferable) or a shared library from object files.

    If `ar` is available the function will produce lib<basename>.a. Otherwise it will
    try to create a shared library (lib<basename>.so on Unix, <basename>.dll on Windows).
    """
    if not obj_files:
        print("No object files to archive; skipping library creation.")
        return None

    ar = shutil.which("ar")
    if ar:
        lib_name = f"lib{lib_basename}.a"
        lib_path = os.path.join(LIB_DIR, lib_name)
        cmd = [ar, "rcs", lib_path] + obj_files
        print("Creating static library:", " ".join(cmd))
        subprocess.run(cmd, check=True)
        return lib_path
    else:
        # Fallback to shared library
        system = platform.system()
        if system == "Windows":
            ext = ".dll"
            lib_name = f"{lib_basename}{ext}"
        else:
            ext = ".so"
            lib_name = f"lib{lib_basename}{ext}"

        lib_path = os.path.join(LIB_DIR, lib_name)
        cmd = ["gcc", "-shared", "-o", lib_path] + obj_files
        print("Creating shared library:", " ".join(cmd))
        subprocess.run(cmd, check=True)
        return lib_path


def build_project():
    print("Building the project...")
    create_directories()

    print("Running code generator...")
    run_code_generator(CODE_GENERATOR_MAIN_FILE_NAME)

    print("Searching for .c files in source directory:", SRC_DIR)
    c_files = list_all_c_files(SRC_DIR)
    if not c_files:
        print("No .c files found. Nothing to compile.")
        return

    try:
        obj_files = compile_c_files(c_files)
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e}")
        return

    try:
        lib_path = create_library(obj_files)
        if lib_path:
            print(f"Library created at: {lib_path}")
            print("Build completed successfully.")

            print("Running tests...")
            try:
                run_tests()
                print("All tests passed successfully.")
            except subprocess.CalledProcessError as e:
                print(f"Tests failed: {e}") 

        else:
            print("Library creation was skipped or failed.")
    except subprocess.CalledProcessError as e:
        print(f"Library creation failed: {e}")


if __name__ == "__main__":
    build_project()