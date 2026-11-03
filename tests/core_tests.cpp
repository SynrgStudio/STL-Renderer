// Pruebas del núcleo sin dependencias de OpenGL: CLI y lector STL.
#include "cli.h"
#include "stl_loader.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

bool throws(const std::function<void()>& fn) {
    try { fn(); } catch (const std::invalid_argument&) { return true; } catch (const std::out_of_range&) { return true; }
    return false;
}

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / fs::u8path(u8"stl_renderer_tests_ñ");
        fs::remove_all(path);
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
};

void writeFile(const fs::path& path, const std::string& bytes) {
    std::ofstream(path, std::ios::binary).write(bytes.data(), std::streamsize(bytes.size()));
}

void putU32(std::string& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) out.push_back(char((v >> (8 * i)) & 0xff));
}

void putF32(std::string& out, float f) {
    uint32_t bits;
    std::memcpy(&bits, &f, sizeof(bits));
    putU32(out, bits);
}

std::string binaryStl(const std::vector<std::array<float, 9>>& triangles, const std::string& header = "") {
    std::string out = header;
    out.resize(80, '\0');
    putU32(out, uint32_t(triangles.size()));
    for (const auto& t : triangles) {
        for (int i = 0; i < 3; ++i) putF32(out, 0.0f);   // normal: se recalcula
        for (float f : t) putF32(out, f);
        out.append(2, '\0');
    }
    return out;
}

const std::string kAscii =
    "solid cube\n"
    "  facet normal 0 0 1\n"
    "    outer loop\n"
    "      vertex 0 0 0\n"
    "      vertex 2 0 0\n"
    "      vertex 0 4 0\n"
    "    endloop\n"
    "  endfacet\n"
    "endsolid cube\n";

void testCli() {
    auto o = parseCli({"-c", "1", "0", "0.5", "--background", "0", "0", "0", "-a", "90", "--width", "640",
                       "--height", "480", "--transparent", "model.stl"});
    check(o.color && (*o.color)[2] == 0.5f, "color parsed");
    check(o.background && (*o.background)[0] == 0.0f, "background parsed");
    check(o.angle && *o.angle == 90.0f, "angle parsed");
    check(o.width == 640 && o.height == 480, "size parsed");
    check(o.transparent == true, "transparent parsed");
    check(o.inputs == std::vector<std::string>{"model.stl"}, "input parsed");

    check(parseCli({"--help"}).help && parseCli({"-v"}).version, "help/version flags");
    check(parseCli({"--", "-weird.stl"}).inputs == std::vector<std::string>{"-weird.stl"}, "-- ends options");
    check(parseCli({"-o", "out.png", "a.stl"}).output == "out.png", "output parsed");

    check(throws([] { parseCli({"--bogus"}); }), "unknown option rejected");
    check(throws([] { parseCli({"-c", "1", "0"}); }), "missing color component rejected");
    check(throws([] { parseCli({"-c", "1", "0", "2"}); }), "out-of-range color rejected");
    check(throws([] { parseCli({"-a", "nan"}); }), "NaN angle rejected");
    check(throws([] { parseCli({"-a", "12abc"}); }), "trailing garbage rejected");
    check(throws([] { parseCli({"--width", "0"}); }), "zero width rejected");
    check(throws([] { parseCli({"--width", "99999999999"}); }), "overflowing width rejected");
    check(throws([] { parseCli({"--silent"}); }), "silent without input rejected");
    check(throws([] { parseCli({"-o", "out.png", "a.stl", "b.stl"}); }), "output with many inputs rejected");
    check(throws([] { parseCli({"-o"}); }), "output without value rejected");
}

void testPaths() {
    check(isStlPath("a.stl") && isStlPath("A.STL") && isStlPath("x/y.Stl"), "STL extension case-insensitive");
    check(!isStlPath("a.obj") && !isStlPath("stl") && !isStlPath("a.stl.png"), "non-STL rejected");
    check(defaultOutputPath("dir/part.stl") == fs::path("dir/part_png.png"), "default output name");
    check(defaultOutputPath("part.v2.STL") == fs::path("part.v2_png.png"), "default output keeps inner dots");
}

void testLoader(const fs::path& dir) {
    StlLoader loader;

    const auto ascii = dir / "ascii.stl";
    writeFile(ascii, kAscii);
    check(loader.loadModel(ascii.u8string()), "ASCII STL loads");
    check(loader.getModel().triangles.size() == 1, "ASCII triangle count");
    check(std::abs(loader.getModel().scale - 0.25f) < 1e-6f, "scale uses largest extent");

    const auto crlf = dir / "crlf.stl";
    std::string withCrLf;
    for (char c : kAscii) { if (c == '\n') withCrLf += '\r'; withCrLf += c; }
    writeFile(crlf, withCrLf);
    check(loader.loadModel(crlf.u8string()), "ASCII STL with CRLF loads");

    const auto bin = dir / "binary.stl";
    writeFile(bin, binaryStl({{0, 0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 0, 0, 1}}));
    check(loader.loadModel(bin.u8string()), "binary STL loads");
    check(loader.getModel().triangles.size() == 2, "binary triangle count");
    const auto n = loader.getModel().triangles[0].vertices[0].normal;
    check(std::abs(glm::length(n) - 1.0f) < 1e-5f, "zero normals are recomputed as unit vectors");

    const auto tricky = dir / "solid_header.stl";
    writeFile(tricky, binaryStl({{0, 0, 0, 1, 0, 0, 0, 1, 0}}, "solid exported-by-cad"));
    check(loader.loadModel(tricky.u8string()), "binary STL whose header starts with 'solid' loads");

    const auto unicode = dir / fs::u8path(u8"piña árbol.stl");
    writeFile(unicode, kAscii);
    check(loader.loadModel(unicode.u8string()), "UTF-8 path loads");

    // Un fallo no debe pisar el modelo cargado previamente.
    const size_t before = loader.getModel().triangles.size();
    const auto truncated = dir / "truncated.stl";
    auto data = binaryStl({{0, 0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 1, 0, 0, 0, 1}});
    data.resize(data.size() - 10);
    writeFile(truncated, data);
    check(!loader.loadModel(truncated.u8string()), "truncated binary rejected");
    check(loader.getModel().triangles.size() == before, "failed load keeps previous model");

    const auto nan = dir / "nan.stl";
    const float qnan = std::numeric_limits<float>::quiet_NaN();
    writeFile(nan, binaryStl({{0, 0, 0, qnan, 0, 0, 0, 1, 0}}));
    check(!loader.loadModel(nan.u8string()), "NaN vertex rejected");

    const auto huge = dir / "huge_count.stl";
    auto lying = binaryStl({{0, 0, 0, 1, 0, 0, 0, 1, 0}});
    lying[80] = lying[81] = lying[82] = lying[83] = char(0xff);
    writeFile(huge, lying);
    check(!loader.loadModel(huge.u8string()), "lying triangle count rejected without huge allocation");

    const auto empty = dir / "empty.stl";
    writeFile(empty, "");
    check(!loader.loadModel(empty.u8string()), "empty file rejected");

    const auto noFacets = dir / "nofacets.stl";
    writeFile(noFacets, "solid x\nendsolid x\n");
    check(!loader.loadModel(noFacets.u8string()), "ASCII without facets rejected");

    const auto garbage = dir / "garbage.stl";
    writeFile(garbage, kAscii + "trailing junk\n");
    check(!loader.loadModel(garbage.u8string()), "trailing garbage rejected");

    const auto badAscii = dir / "bad.stl";
    writeFile(badAscii, "solid x\n facet normal 0 0 1\n outer loop\n vertex 0 0\n");
    check(!loader.loadModel(badAscii.u8string()), "malformed ASCII rejected");

    check(!loader.loadModel((dir / "missing.stl").u8string()), "missing file rejected");
}
} // namespace

int main() {
    TempDir temp;
    testCli();
    testPaths();
    testLoader(temp.path);
    if (failures) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "All core tests passed\n";
    return 0;
}
