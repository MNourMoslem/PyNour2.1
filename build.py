#!/usr/bin/env python3
"""
PyNour Build System
A flexible build system for the PyNour project with support for:
- Building the library
- Running tests
- Running individual C files
- Code generation
"""

import os
import sys
import subprocess
import shutil
import platform
import argparse
from pathlib import Path
from typing import List, Optional
from dataclasses import dataclass


@dataclass
class BuildConfig:
    """Configuration for the build system."""
    core_dir: Path
    src_dir: Path
    build_dir: Path
    lib_dir: Path
    code_generated_dir: Path
    obj_dir: Path
    code_generators_dir: Path
    tests_dir: Path
    include_dirs: List[Path]
    
    # File names
    code_generator_main: str = "main.py"
    test_main: str = "main.c"
    
    @classmethod
    def create_default(cls) -> 'BuildConfig':
        """Create default build configuration."""
        root = Path(__file__).parent
        core_dir = root / "nour" / "_core"
        src_dir = core_dir / "src"
        build_dir = src_dir / "build"
        
        return cls(
            core_dir=core_dir,
            src_dir=src_dir,
            build_dir=build_dir,
            lib_dir=build_dir / "lib",
            code_generated_dir=build_dir / "generated", 
            obj_dir=build_dir / "obj",
            code_generators_dir=core_dir / "code_generators",
            tests_dir=core_dir / "tests",
            include_dirs=[core_dir / "include"]
        )


class BuildSystem:
    """Main build system class."""
    
    def __init__(self, config: BuildConfig):
        self.config = config


    def create_directories(self) -> None:
        """Ensure build-related directories exist."""
        dirs_to_create = [
            self.config.build_dir,
            self.config.obj_dir,
            self.config.lib_dir,
            self.config.code_generated_dir
        ]
        
        for directory in dirs_to_create:
            # Delete existing contents for a clean build
            if directory.exists():
                shutil.rmtree(directory)
            directory.mkdir(parents=True, exist_ok=True)
            print(f"Created directory: {directory}")

    def find_c_files(self, directory: Path) -> List[Path]:
        """Find all .c files in a directory recursively."""
        c_files = []
        if not directory.exists():
            return c_files
            
        for file_path in directory.rglob("*.c"):
            c_files.append(file_path)
        return c_files

    def run_code_generator(self) -> bool:
        """Run the Python code generator."""
        generator_path = self.config.code_generators_dir / self.config.code_generator_main
        
        if not generator_path.exists():
            print(f"Code generator not found: {generator_path}")
            return False
            
        print(f"Running code generator: {generator_path}")
        try:
            subprocess.run([sys.executable, str(generator_path)], check=True)
            print("Code generation completed successfully.")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Code generator failed: {e}")
            return False

    def compile_c_files(self, c_files: List[Path]) -> List[Path]:
        """Compile .c files to object files and return list of object paths."""
        if not c_files:
            print("No .c files to compile.")
            return []

        obj_files = []
        include_flags = []
        
        # Build include flags
        for inc_dir in self.config.include_dirs:
            include_flags.extend(["-I", str(inc_dir)])

        print(f"Compiling {len(c_files)} C files...")
        
        for c_file in c_files:
            obj_name = c_file.stem + ".o"
            obj_file = self.config.obj_dir / obj_name
            
            cmd = [
                "gcc", "-c", str(c_file), "-o", str(obj_file), 
                "-fPIC", "-Wall", "-Wextra"
            ] + include_flags
            
            print(f"  Compiling: {c_file.name}")
            try:
                subprocess.run(cmd, check=True)
                obj_files.append(obj_file)
            except subprocess.CalledProcessError as e:
                print(f"Compilation failed for {c_file}: {e}")
                raise
                
        print(f"Successfully compiled {len(obj_files)} object files.")
        return obj_files

    def create_library(self, obj_files: List[Path], lib_basename: str = "nour") -> Optional[Path]:
        """Create a static library (preferred) or shared library from object files."""
        if not obj_files:
            print("No object files to archive; skipping library creation.")
            return None

        # Try to create static library first
        ar_tool = shutil.which("ar")
        if ar_tool:
            lib_name = f"lib{lib_basename}.a"
            lib_path = self.config.lib_dir / lib_name
            cmd = [ar_tool, "rcs", str(lib_path)] + [str(obj) for obj in obj_files]
            
            print(f"Creating static library: {lib_name}")
            try:
                subprocess.run(cmd, check=True)
                print(f"Static library created successfully: {lib_path}")
                return lib_path
            except subprocess.CalledProcessError as e:
                print(f"Static library creation failed: {e}")
                return None
        else:
            # Fallback to shared library
            system = platform.system()
            if system == "Windows":
                lib_name = f"{lib_basename}.dll"
            else:
                lib_name = f"lib{lib_basename}.so"

            lib_path = self.config.lib_dir / lib_name
            cmd = ["gcc", "-shared", "-o", str(lib_path)] + [str(obj) for obj in obj_files]
            
            print(f"Creating shared library: {lib_name}")
            try:
                subprocess.run(cmd, check=True)
                print(f"Shared library created successfully: {lib_path}")
                return lib_path
            except subprocess.CalledProcessError as e:
                print(f"Shared library creation failed: {e}")
                return None

    def find_library(self) -> Optional[Path]:
        """Find the created library file."""
        candidates = [
            self.config.lib_dir / "libnour.a",
            self.config.lib_dir / "libnour.so", 
            self.config.lib_dir / "nour.dll"
        ]
        
        for candidate in candidates:
            if candidate.exists():
                return candidate
        return None

    def compile_and_run_single_file(self, c_file: Path) -> bool:
        """Compile and run a single C file."""
        if not c_file.exists():
            print(f"File not found: {c_file}")
            return False
            
        print(f"Compiling and running: {c_file}")
        
        # Create executable name
        exe_name = c_file.stem
        if platform.system() == "Windows":
            exe_name += ".exe"
        exe_path = self.config.build_dir / exe_name
        
        # Build include flags
        include_flags = []
        for inc_dir in self.config.include_dirs:
            include_flags.extend(["-I", str(inc_dir)])
        
        # Check if library exists to link against
        lib_path = self.find_library()
        
        # Compile command
        cmd = [
            "gcc", str(c_file), "-o", str(exe_path),
            "-Wall", "-Wextra"
        ] + include_flags
        
        if lib_path:
            cmd.append(str(lib_path))
            print(f"Linking against library: {lib_path}")
        
        try:
            print("Compiling...")
            subprocess.run(cmd, check=True)
            print(f"Compilation successful: {exe_path}")
            
            print("Running executable...")
            subprocess.run([str(exe_path)], check=True)
            print("Execution completed successfully.")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Failed to compile or run {c_file}: {e}")
            return False

    def run_tests(self) -> bool:
        """Compile and run all tests."""
        test_c_files = self.find_c_files(self.config.tests_dir)
        if not test_c_files:
            print(f"No test .c files found in {self.config.tests_dir}")
            return False

        test_main_path = self.config.tests_dir / self.config.test_main
        if not test_main_path.exists():
            print(f"No test main file found: {test_main_path}")
            return False

        print(f"Found {len(test_c_files)} test files.")
        
        # Build include flags
        include_flags = []
        for inc_dir in self.config.include_dirs:
            include_flags.extend(["-I", str(inc_dir)])

        # Compile test files to objects
        test_obj_files = []
        for c_file in test_c_files:
            obj_name = c_file.stem + ".o"
            obj_file = self.config.obj_dir / obj_name
            
            cmd = [
                "gcc", "-c", str(c_file), "-o", str(obj_file), 
                "-fPIC"
            ] + include_flags
            
            print(f"Compiling test: {c_file.name}")
            try:
                subprocess.run(cmd, check=True)
                test_obj_files.append(obj_file)
            except subprocess.CalledProcessError as e:
                print(f"Test compilation failed for {c_file}: {e}")
                return False

        # Create test executable
        test_exe = self.config.build_dir / "test_executable"
        if platform.system() == "Windows":
            test_exe = test_exe.with_suffix(".exe")

        # Link test executable
        lib_path = self.find_library()
        
        main_obj = self.config.obj_dir / (self.config.test_main.replace('.c', '.o'))
        ordered_objs = [obj for obj in test_obj_files if obj != main_obj]
        if main_obj.exists():
            ordered_objs.append(main_obj)

        cmd = ["gcc"] + [str(obj) for obj in ordered_objs] + ["-o", str(test_exe)]
        if lib_path:
            cmd.append(str(lib_path))

        print("Linking test executable...")
        try:
            subprocess.run(cmd, check=True)
            print(f"Test executable created: {test_exe}")
        except subprocess.CalledProcessError as e:
            print(f"Test linking failed: {e}")
            return False

        # Run tests
        print("Running tests...")
        try:
            subprocess.run([str(test_exe)], check=True)
            print("All tests passed successfully!")
            return True
        except subprocess.CalledProcessError as e:
            print(f"Tests failed: {e}")
            return False

    def build(self) -> bool:
        """Build the project (code generation + compilation + library creation)."""
        print("=== Building PyNour Project ===")
        
        # Create directories
        self.create_directories()
        
        # Run code generator
        if not self.run_code_generator():
            print("Build failed: Code generation error")
            return False
        
        # Find and compile C files
        c_files = self.find_c_files(self.config.src_dir)
        if not c_files:
            print("No .c files found in source directory.")
            return False
            
        print(f"Found {len(c_files)} C files to compile.")
        
        try:
            obj_files = self.compile_c_files(c_files)
        except subprocess.CalledProcessError:
            print("Build failed: Compilation error")
            return False
        
        # Create library
        lib_path = self.create_library(obj_files)
        if lib_path:
            print(f"Build completed successfully! Library: {lib_path}")
            return True
        else:
            print("Build failed: Library creation error")
            return False


def create_parser() -> argparse.ArgumentParser:
    """Create command line argument parser."""
    parser = argparse.ArgumentParser(
        description="PyNour Build System",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build.py build                    # Build the library
  python build.py test                     # Run tests only
  python build.py build test               # Build and run tests
  python build.py build -f myfile.c        # Build and run specific file
  python build.py -f myfile.c              # Just run specific file (with existing library)
  python build.py clean                    # Clean build artifacts
        """
    )
    
    parser.add_argument(
        "commands",
        nargs="*",  # Changed from "+" to "*" to make commands optional
        choices=["build", "test", "clean"],
        help="Commands to execute (can specify multiple). Optional when using -f."
    )
    
    parser.add_argument(
        "-f", "--file",
        type=str,
        help="Specific C file to compile and run (relative to current directory)"
    )
    
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    
    return parser


def main():
    """Main entry point."""
    parser = create_parser()
    args = parser.parse_args()
    
    # Validate arguments
    if not args.commands and not args.file:
        parser.error("Must specify either commands (build/test/clean) or -f option")
    
    # Create build configuration
    config = BuildConfig.create_default()
    build_system = BuildSystem(config)
    
    success = True
    
    # Handle case where only -f is provided (no commands)
    if not args.commands and args.file:
        print("=== Running Single File ===")
        file_path = Path(args.file)
        if not build_system.compile_and_run_single_file(file_path):
            success = False
    else:
        # Process commands in order
        for command in args.commands:
            if command == "clean":
                print("=== Cleaning Build Artifacts ===")
                if config.build_dir.exists():
                    shutil.rmtree(config.build_dir)
                    print(f"Cleaned: {config.build_dir}")
                else:
                    print("Nothing to clean.")
                    
            elif command == "build":
                if not build_system.build():
                    success = False
                    break
                    
            elif command == "test":
                if not build_system.run_tests():
                    success = False
                    break
        
        # Handle -f option after commands (if both are provided)
        if success and args.file and args.commands:
            print("=== Running Single File After Commands ===")
            file_path = Path(args.file)
            if not build_system.compile_and_run_single_file(file_path):
                success = False
    
    if success:
        print("All operations completed successfully!")
        sys.exit(0)
    else:
        print("Some operations failed!")
        sys.exit(1)


if __name__ == "__main__":
    main()