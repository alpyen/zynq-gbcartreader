# This script is used to regenerate the Vitis project by executing these steps:
#
#   1. Perform several sanity checks to ensure proper operation
#   2. Delete all files in this vitis subfolder except regenerate.py
#   3. Run Vitis in CLI mode and create platform + application
#   4. Link sources from outside the repo's source folder into the projects
#
# Once that's done, you can start Vitis normally and set the vitis subfolder
# as the workspace and work on the project.
#
# Note: In order for this script to correctly regenerate the workspace,
#       you need to export the XSA files from Vivado for each platform first!
#       This script will sanity check that beforehand.

import os, sys, shutil, stat, pathlib

def print_error(message: str):
    print("\033[91m" + f"[regenerate.py] Error: {message}" + "\033[0m")

def print_warning(message: str):
    print("\033[93m" + f"[regenerate.py] Warning: {message}" + "\033[0m")

def print_info(message: str):
    print("\033[94m" + f"[regenerate.py] Info: {message}" + "\033[0m")

def print_success(message: str):
    print("\033[92m" + f"[regenerate.py] Info: {message}" + "\033[0m")

if len(sys.argv) != 2 or ("pynq-z2" not in sys.argv[1] and "basys3" not in sys.argv[1]):
    print_error("Board unknown, use \"pynq-z2\" for ARM or \"basys3\" for MicroBlaze-V. Exiting!");
    print_info("Try: \"vitis -s regenerate.py pynq-z2\".")
    exit(0)

board=sys.argv[1]

# Sanity check: Is this script being executed from the Vitis python interface?
try:
    import vitis
except ModuleNotFoundError:
    print_error(
        "Cannot find vitis module. Source settings64.sh from the vitis installation " \
        "and run this script with \"vitis -s regenerate.py\". Exiting!"
    )
    exit(0)

# Sanity check: Is the "vitis" subfolder the current working directory?
if pathlib.Path(".").absolute().name != "vitis":
    print_error(
        "Current working directory is not vitis. " \
        "Change into vitis subfolder first. Exiting!"
    )

# Sanity check: Were the XSA files exported properly?
xsa_exist = True
xsa_file = "pynq_z2_wrapper.xsa" if (board == "pynq-z2") else "basys3_wrapper.xsa"
if xsa_file not in os.listdir("../vivado"):
    print_error(f"\"{xsa_file}\" file not found in vivado subfolder. Exiting!")
    print_info("Try exporting it from Vivado first or placing a pre-built XSA file in the vivado subfolder.")
    exit(0)

# -----------------------------------------------------------------------------

# Delete all temporary workspace files as we will regenerate it from this script
# Some files are write-protected from Vitis so they need to be chmodded
print_info("Deleting previous workspace.")

def fix_permissions(func, path, excinfo):
    os.chmod(path, stat.S_IWRITE)
    func(path)

paths = os.listdir()
for path in paths:
    if path != "regenerate.py":
        if os.path.isdir(path):
            shutil.rmtree(path, onexc=fix_permissions)
        else:
            os.remove(path)

# -----------------------------------------------------------------------------

# Regenerate project (commands from workspace journal)
print_info("Regenerating projects with Vitis CLI.")

client = vitis.create_client()

# "client.set_workspace" only works if the path does not exist beforehand
client.update_workspace(path=".")

advanced_options = client.create_advanced_options_dict(dt_overlay="0")

if board == "pynq-z2":
    pynq_z2_platform = client.create_platform_component(
        name = "pynq_z2_platform",
        hw_design = "$COMPONENT_LOCATION/../../vivado/pynq_z2_wrapper.xsa",
        os = "standalone",
        cpu = "ps7_cortexa9_0",
        domain_name = "standalone_ps7_cortexa9_0",
        generate_dtb = False,
        advanced_options = advanced_options,
        compiler = "gcc"
    )

    pynq_z2_application = client.create_app_component(
        name = "pynq_z2_application",
        platform = "$COMPONENT_LOCATION/../pynq_z2_platform/export/pynq_z2_platform/pynq_z2_platform.xpfm",
        domain = "standalone_ps7_cortexa9_0"
    )

    application = pynq_z2_application
else: # board == "basys3"
    basys3_platform = client.create_platform_component(
        name = "basys3_platform",
        hw_design = "$COMPONENT_LOCATION/../../vivado/basys3_wrapper.xsa",
        os = "standalone",
        cpu = "microblaze_riscv_0",
        domain_name = "standalone_microblaze_riscv_0",
        generate_dtb = False,
        advanced_options = advanced_options,
        compiler = "gcc"
    )

    basys3_application = client.create_app_component(
        name="basys3_application",
        platform = "$COMPONENT_LOCATION/../basys3_platform/export/basys3_platform/basys3_platform.xpfm",
        domain = "standalone_microblaze_riscv_0"
    )

    application = basys3_application

    print_warning("Vitis 2025.1 does not support setting compilation parameters through the Python API.")
    print_warning("Set the following parameters manually:")
    print_warning("  MicroBlaze-V stack and heap are too small. Change them to 0x2000 in the linker script.")
    print_warning("  UartLite uses a different driver. Set the define UARTLITE in the application's UserConfig.cmake.")

# -----------------------------------------------------------------------------

# Link sources that are located outside the workspace directory
print_info("Linking sources outside the workspace to the applications.")

application.import_files("../src", ["*.cpp"], is_skip_copy_sources=True)

print_success("Regeneration complete. You can start Vitis and set the vitis subfolder as the workspace.")

vitis.dispose()
