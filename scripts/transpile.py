# Usage: python scripts/transpile.py {target}
# e.g. python scripts/transpile.py src_test/Shaders/
#      python scripts/transpile.py examples/GPUParticle/Shaders/

import os, subprocess, shutil, glob, platform, argparse

aparser = argparse.ArgumentParser()
aparser.add_argument('target', help='target directory')

args = aparser.parse_args()

target_directory=os.path.join(os.getcwd(), args.target)
os.chdir(os.path.dirname(__file__))

transpiler_filename = "ShaderTranspiler.exe" if platform.system() == 'Windows' else "ShaderTranspiler"
transpiler_path = os.path.join("../build/tools/ShaderTranspiler/Debug", transpiler_filename)
if os.path.isfile(transpiler_path):
    shutil.copy(transpiler_path, ".")

verts = glob.glob(os.path.join(target_directory, 'HLSL_DX12/*.vert'), recursive=True)
frags = glob.glob(os.path.join(target_directory, 'HLSL_DX12/*.frag'), recursive=True)
comps = glob.glob(os.path.join(target_directory, 'HLSL_DX12/*.comp'), recursive=True)

for target,directory in [
    ('-M', 'Metal'),
    ('-V', 'GLSL_VULKAN'),
    ('-G', 'GLSL_GL')]:
    for kind,paths in [
        ('--vert', verts),
        ('--frag', frags),
        ('--comp', comps) ]:
        for f in paths:
            subprocess.call(['ShaderTranspiler', kind, target, '--input', f, '--output', os.path.join(target_directory, directory, os.path.basename(f))])

verts = glob.glob(os.path.join(target_directory, 'GLSL_VULKAN/*.vert'), recursive=True)
frags = glob.glob(os.path.join(target_directory, 'GLSL_VULKAN/*.frag'), recursive=True)
comps = glob.glob(os.path.join(target_directory, 'GLSL_VULKAN/*.comp'), recursive=True)

for f in (verts + frags + comps):
    subprocess.call(['glslangValidator', f, '-e', 'main', '-V', '-o', os.path.join(target_directory, 'SPIRV', os.path.basename(f)) + '.spv'])
