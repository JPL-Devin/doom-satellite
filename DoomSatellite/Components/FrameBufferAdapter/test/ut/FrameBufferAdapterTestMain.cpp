// ======================================================================
// \title  FrameBufferAdapterTestMain.cpp
// \brief  cpp file for FrameBufferAdapter component test main function
// ======================================================================

#include "FrameBufferAdapterTester.hpp"

TEST(Pack, Frame) {
    Components::FrameBufferAdapterTester tester;
    tester.testPacksFrame();
}

TEST(Pack, Palette) {
    Components::FrameBufferAdapterTester tester;
    tester.testPacksPalette();
}

TEST(Pack, RejectsInvalidFrame) {
    Components::FrameBufferAdapterTester tester;
    tester.testRejectsInvalidFrameIn();
}

TEST(Pack, RejectsWhileLent) {
    Components::FrameBufferAdapterTester tester;
    tester.testRejectsWhileLent();
}

TEST(Unpack, RoundTripsFrame) {
    Components::FrameBufferAdapterTester tester;
    tester.testRoundTripsFrame();
}

TEST(Unpack, RoundTripsPalette) {
    Components::FrameBufferAdapterTester tester;
    tester.testRoundTripsPalette();
}

TEST(Unpack, RejectsInvalidPacked) {
    Components::FrameBufferAdapterTester tester;
    tester.testRejectsInvalidPacked();
}

TEST(Unpack, RejectsInvalidFrame) {
    Components::FrameBufferAdapterTester tester;
    tester.testRejectsInvalidPackedFrame();
}

TEST(Unpack, RejectsInvalidPalette) {
    Components::FrameBufferAdapterTester tester;
    tester.testRejectsInvalidPackedPalette();
}

TEST(Echo, RelaysAndReturns) {
    Components::FrameBufferAdapterTester tester;
    tester.testEchoRelaysAndReturns();
}

TEST(Echo, ReturnsWhenUnconnected) {
    Components::FrameBufferAdapterTester tester;
    tester.testEchoReturnsWhenUnconnected();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
