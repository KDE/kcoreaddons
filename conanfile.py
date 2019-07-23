from conans import ConanFile, CMake


class KCoreAddonsConan(ConanFile):
    name = "kcoreaddons"
    version = "5.50.0"
    license = "GPLv2"
    url = "https://api.kde.org/frameworks/kcoreaddons/html/index.html"
    description = "Qt addon library with a collection of non-GUI utilities"

    settings = "os", "compiler", "build_type", "arch"

    requires = (
        "extra-cmake-modules/5.50.0@kde/testing", # CMakeLists.txt requires 5.49.0

        "Qt/5.11.1@bincrafters/stable"
        # "qt-gui/5.8.0@qt/testing",

        # fam/latest@foo/bar,
        # inotify/latest@foo/bar,
    )

    generators = "cmake"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
     }

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package_info(self):
        self.cpp_info.resdirs = ["share"]
