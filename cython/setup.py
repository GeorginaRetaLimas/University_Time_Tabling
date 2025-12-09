from setuptools import setup, Extension
from Cython.Build import cythonize

extensions = [
    Extension(
        "timetable_wrapper",
        sources=["timetable_wrapper.pyx", "../cpp/timetable_solver.cpp"],
        language="c++",
        extra_compile_args=["-std=c++17", "-O3"],
    )
]

setup(
    name="timetable_wrapper",
    ext_modules=cythonize(extensions),
)
