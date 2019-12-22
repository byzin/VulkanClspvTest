# source this file into an existing shell.

export VULKAN_SDK="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/macOS
export PATH="${VULKAN_SDK}/bin:${PATH}"
export DYLD_LIBRARY_PATH="${VULKAN_SDK}/lib:${DYLD_LIBRARY_PATH}"
export VK_LAYER_PATH="${VULKAN_SDK}/etc/vulkan/explicit_layer.d"
export VK_ICD_FILENAMES="${VULKAN_SDK}/etc/vulkan/icd.d/MoltenVK_icd.json"
