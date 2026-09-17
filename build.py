import os
import shutil
import subprocess
import sys

cmake_error = (
    'NON HAI INSTALLATO CMAKE MALEDETTO MA CON COSA PENSAVI DI FARE LA BUILD CON TUA MAMMA?\n'
    'MA DOVE CREDI DI STARE NEL 1800? CREDI CHE QUA C\'E\' GENTE COME TE CHE USA MESON COME BUILD SYSTEM?\n'
    'COSA PENSAVI DI FARE DI CAVARTELA CON "MESON SETUP BUILDDIR"? INSTALLA IMMEDIATAMENTE CMAKE\n'
    'ALTRIMENTI VENGO LI E TI FACCIO IMPLEMENTARE IN HASKELL L\'HEAPSORT'
)

try:
    subprocess.run("cmake", stdout=open(os.devnull))
except Exception:
    exit(cmake_error)

lookup = {
    "--compile": lambda: subprocess.run(["cmake", "--build", "build"]), 
    "--run": lambda: subprocess.run("./build/bin/httpsrv"),         
}

if len(sys.argv) < 2:
    build_path = "./build"

    if os.path.exists(build_path):
        shutil.rmtree(build_path)
        
    os.makedirs(build_path)
    subprocess.run(["cmake", "-S", ".", "-B", "build"])
    subprocess.run(["cmake", "--build", "build"])
else:
    action = lookup[sys.argv[1]]
    if action:
        action()
    else:
        print("PARAMETRO INVALIDO")
