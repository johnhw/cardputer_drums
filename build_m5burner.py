import os
import json
import zipfile

# Define paths
m5burner_json_path = 'm5burner/m5burner.json'
build_dir = 'build'
stack_dir = 'm5stack.esp32.m5stack_cardputer'
firmware_dir = 'firmware'

# Load the version from m5burner.json
with open(m5burner_json_path, 'r') as f:
    m5burner_data = json.load(f)
    version = m5burner_data['version']

# Replace '.' with '_' in the version for the zip filename
version_safe = version.replace('.', '_')

# Define the zip file name
zip_filename = f'bonnethead_{version_safe}.zip'
zip_filepath = os.path.join(build_dir, zip_filename)

# Define the files to be added to the zip
files_to_zip = {
    'm5burner/m5burner.json': 'm5burner.json',
    os.path.join(build_dir, stack_dir, 'cardputer_drums.ino.bin'): f'{firmware_dir}/bonnethead_0x10000.bin',
    os.path.join(build_dir, stack_dir, 'cardputer_drums.ino.partitions.bin'): f'{firmware_dir}/partitions_singleapp_0x8000.bin',
    os.path.join(build_dir, stack_dir, 'cardputer_drums.ino.bootloader.bin'): f'{firmware_dir}/bootloader_0x1000.bin'    
}

# Ensure the build directory exists
os.makedirs(build_dir, exist_ok=True)

# Create the zip file
with zipfile.ZipFile(zip_filepath, 'w') as zipf:
    for src, dst in files_to_zip.items():
        zipf.write(src, dst)

print(f'Release binary created: {zip_filepath}')