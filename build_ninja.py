from ninja_syntax import Writer
import os
import re
import sys

ninja_rule_file = "rules.ninja"
ninja_build_file = "build.ninja"

WORKSPACE_DIR = os.getcwd()
BUILD_DIR = WORKSPACE_DIR+"/build"
ELF_NAME = 'kernel.elf'
CROSS_COMPILER = 'clang'
CC = CROSS_COMPILER
LD = CROSS_COMPILER
OBJDUMP = 'llvm-objdump'
LINKER = 'linker.ld'
PREPROCESSED_LINKER = BUILD_DIR + '/linker.ld.gen'
CFLAGS =  '--target=aarch64-linux-gnu -march=armv8.2-a -mcpu=cortex-a72 -Os -Wall -Wextra -g -ffreestanding -nostdlib -fpic'
LDFLAGS = '--target=aarch64-linux-gnu -nostdlib -fuse-ld=lld -pie -Wl,-z,max-page-size=4096 -Wl,-z,separate-loadable-segments -Wl,-Map='+os.path.join(BUILD_DIR,ELF_NAME)+'.map'
OBJDUMP_NAME = 'kernel.dump'
ELFDUMP_NAME = 'kernel.elf.dump'
# Docs
DOXY_HTML_FILE = os.path.join(WORKSPACE_DIR,'docs','html','index.html')
DOXYGEN = 'doxygen'
DOXYGEN_CONFIG = os.path.join(WORKSPACE_DIR, 'okernel_doxy.config')
# formatting dummy file for dependency chain
FORMAT_FILE = '.format-stamp'
#generats rule.ninja
def generate_ninja_rules():
  """
  Generates a rules.ninja file for rules to build source files.
  """
  file = open(ninja_rule_file, "w")
  writer = Writer(file, width=78)
  writer.comment("Automatically generated ninja rules file.")
  writer.newline()
  #define the cflags variable
  writer.variable("cflags", CFLAGS)
  writer.newline()
  # Define the c files compile rule
  writer.rule(
      name="cc",
      command=f"{CC} $cflags -c $in -o $out",
      # depfile="$out.d",
      description="compile c/asm source files to create object file"
  )
  writer.newline()
  # Define linker rule 
  writer.rule(
      name="link",
      command=f"{LD} {LDFLAGS} -T {PREPROCESSED_LINKER} -o $out $in",
      description="linker to generate the elf"
  )
  writer.newline()
  #define generate linker script
  writer.rule(
      name="gen-link",
      command=f"{CC} -xc -P -E $in > $out",
      description="linker the preprocessed linker script"
  )
  writer.newline()
  # Define objdump rule 
  writer.rule(
      name="objdump",
      command=f"{OBJDUMP} -D $in > $out",
      description="objdump of kernel elf"
  )
  writer.newline()
  # Define readelf rule 
  writer.rule(
      name="readelf",
      command=f"llvm-readelf -a $in > $out",
      description="dumping elf information"
  )
  writer.newline()
  # Define generate docs rule 
  writer.rule(
      name="gen_docs",
      command=f"GIT_COMMIT_HASH=`git describe --tags --dirty --always` {DOXYGEN} {DOXYGEN_CONFIG} > /dev/null 2>&1",
      description="generating docs"
  )
  writer.newline()
  # format source files using clang-format 
  writer.rule(
      name="format_files",
      command=f"clang-format -i *.c *.h && touch {FORMAT_FILE}",
      description="formatting files using .clang-format"
  )
  #close the rules file
  file.close()

#generates build.ninja
def generate_ninja_build(source_files, extn, build_dir="build"):
  """
  Generates a ninja.build file for compiling source files.

  Args:
    source_files: List of source file paths.
    extn: list of file extensions to replace.
    build_dir: build directory location
  """

  file = open(ninja_build_file, "w")
  writer = Writer(file, width=78)

  # import rules.ninja
  if os.path.exists(ninja_rule_file):
      writer.include(ninja_rule_file)
      writer.newline()
  else:
    print("rules.ninja doesn't exist, exiting...")
    sys.exit(0)

  # format the source files first
  writer.build(FORMAT_FILE, rule="format_files", inputs=None)
  writer.newline()

  # Build targets for each source file
  extn_pattern = r"\.(" + "|".join(extn) + ")$"
  object_files = []
  for source_file in source_files:
    object_file = build_dir + "/" +re.sub(extn_pattern, ".o", os.path.basename(source_file))
    object_files.append(object_file)
    writer.build(object_file, rule="cc", inputs=[source_file], order_only=[FORMAT_FILE])
    writer.newline()
    # writer.build(object_file, rule="cc", inputs=[source_file], variables={'cflags' : flags})
  
  #generate linker script
  writer.build(PREPROCESSED_LINKER,rule="gen-link", inputs=[LINKER])
  writer.newline()
  #link to generate elf
  writer.build(os.path.join(build_dir,ELF_NAME),rule="link", inputs=object_files, order_only=[PREPROCESSED_LINKER])
  writer.newline()

  #objdump the kernel elf
  writer.build(os.path.join(build_dir, OBJDUMP_NAME), rule="objdump", inputs=[os.path.join(build_dir,ELF_NAME)], order_only=[os.path.join(build_dir,ELF_NAME)])
  #dump the kernel elf information
  writer.build(os.path.join(build_dir, ELFDUMP_NAME), rule="readelf", inputs=[os.path.join(build_dir,ELF_NAME)], order_only=[os.path.join(build_dir,ELF_NAME)])
  #generate the docs
  writer.build(DOXY_HTML_FILE, rule="gen_docs", inputs=None, order_only=[os.path.join(build_dir, OBJDUMP_NAME)])
  #close the build.ninja file
  file.close()

def find_files_with_extns(directory, extn):
  """Finds all C files within a given directory and its subdirectories.

  Args:
    directory: The root directory to search.

  Returns:
    A list of paths to all C files found.
  """

  c_files = []
  for root, dirs, files in os.walk(directory):
    for file in files:
      if file.endswith(extn):
        c_files.append(os.path.join(root, file))
  return c_files

#generate build instructions for source files
c_source_files = find_files_with_extns(WORKSPACE_DIR, extn=".c")
c_source_files.extend(find_files_with_extns(WORKSPACE_DIR, extn=".S"))

generate_ninja_rules()
generate_ninja_build(c_source_files, extn=["c","s","S"], build_dir=BUILD_DIR)

# print(c_source_files)
print("Generated ninja.build file!")