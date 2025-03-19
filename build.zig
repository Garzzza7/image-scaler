// THIS IS STILL WIP, DON'T RELAY ON IT TO WORK EVERY TIME, USE COMPILE.SH OR CMAKE
const std = @import("std");

pub fn build(b: *std.Build) void {
    const exe = b.addExecutable(.{
        .name = "image-scaler",
        .target = b.graph.host,
    });

    const exe_cflags = [_][]const u8{
        "-g",
        "-std=c++0x",
        "-Wall",
        "-pedantic",
        "-pthread",
        "-static",
    };

    const cpp_sources = [_][]const u8{
        "src/main.cpp",
    };

    exe.addCSourceFiles(.{
        .files = &cpp_sources,
        .flags = &exe_cflags,
    });
}
