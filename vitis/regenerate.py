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

import os, shutil, stat, pathlib

def print_error(message: str):
    print("\033[91m" + f"[regenerate.py] Error: {message}" + "\033[0m")

def print_info(message: str):
    print("\033[94m" + f"[regenerate.py] Info: {message}" + "\033[0m")

def print_success(message: str):
    print("\033[92m" + f"[regenerate.py] Info: {message}" + "\033[0m")

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
xsa_files = ["sw_impl_wrapper.xsa"]
for xsa in xsa_files:
    if xsa not in os.listdir("../vivado"):
        print_error(f"\"{xsa}\" file not found in vivado subfolder. Export first!")
        xsa_exist = False

if not xsa_exist:
    print_error("One or more XSA files were not found. Exiting!")
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

sw_impl_platform = client.create_platform_component(
    name = "sw_impl_platform",
    hw_design = "$COMPONENT_LOCATION/../../vivado/sw_impl_wrapper.xsa",
    os = "standalone",
    cpu = "ps7_cortexa9_0",
    domain_name = "standalone_ps7_cortexa9_0",
    generate_dtb = False,
    advanced_options = advanced_options,
    compiler = "gcc"
)

sw_impl_standalone_app = client.create_app_component(
    name = "sw_impl_standalone_app",
    platform = "$COMPONENT_LOCATION/../sw_impl_platform/export/sw_impl_platform/sw_impl_platform.xpfm",
    domain = "standalone_ps7_cortexa9_0"
)

# -----------------------------------------------------------------------------

# Link sources that are located outside the workspace directory
print_info("Linking sources outside the workspace to the applications.")

sw_impl_standalone_app.import_files("../src/sw_impl", ["*.cpp"], is_skip_copy_sources=True)
sw_impl_standalone_app.import_files("../src", ["common.cpp", "standalone.cpp"], is_skip_copy_sources=True)

print_success("Regeneration complete. You can start Vitis and set the vitis subfolder as the workspace.")

vitis.dispose()
