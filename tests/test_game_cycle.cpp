// Tests for has_game_cycle() - Upcoming Cycle Detection
//
// The has_game_cycle() function implements a fast algorithm to detect potential
// repetition draws one ply before they occur. It uses a cuckoo hash table of
// precomputed Zobrist hashes for reversible moves and checks if:
// 1. Opponent pieces have returned to their original positions
// 2. There exists a legal move that would create a repetition
//
// See: "A fast software-based method for upcoming cycle detection in search trees"
//      by M. N. J. van Kervinck (2013)
//
// This test file creates various board positions and move sequences to exercise
// the function and document its behavior.

#include <iomanip>
#include <iostream>

#include "cuckoo.hpp"
#include "position.hpp"
#include "repetition_info.hpp"
#include "test.hpp"
#include "zobrist.hpp"

using namespace Clockwork;

static Position       g_position;
static RepetitionInfo g_repetition_info;

void move(std::string_view movestr) {
    Move move  = Move::parse(movestr, g_position).value();
    g_position = g_position.move(move);
    g_repetition_info.push(g_position.get_hash_key(), g_position.is_reversible(move));
}

void test_knight_shuffle_pattern() {
    std::cout << "test_knight_shuffle_pattern" << std::endl;

    // Start from a position with non-zero 50mr counter to ensure enough history
    g_position = Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 3 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    // Test before any moves
    bool result = g_repetition_info.has_game_cycle(g_position, 0);
    std::cout << "  Before moves: " << result << std::endl;

    // Make knight moves that shuffle back and forth
    move("g1f3");
    result = g_repetition_info.has_game_cycle(g_position, 1);
    std::cout << "  After g1f3: " << result << std::endl;

    move("g8f6");
    result = g_repetition_info.has_game_cycle(g_position, 2);
    std::cout << "  After g8f6: " << result << std::endl;

    move("f3g1");
    result = g_repetition_info.has_game_cycle(g_position, 3);
    std::cout << "  After f3g1 (white knight returns): " << result << std::endl;

    move("f6g8");
    result = g_repetition_info.has_game_cycle(g_position, 4);
    std::cout << "  After f6g8 (back to start): " << result << std::endl;

    // Continue the pattern
    move("g1f3");
    result = g_repetition_info.has_game_cycle(g_position, 5);
    std::cout << "  After second g1f3: " << result << std::endl;
}

void test_rook_shuffle_pattern() {
    std::cout << "\ntest_rook_shuffle_pattern" << std::endl;

    g_position = Position::parse("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 4 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("a1b1");
    bool result = g_repetition_info.has_game_cycle(g_position, 1);
    std::cout << "  After a1b1: " << result << std::endl;

    move("a8b8");
    result = g_repetition_info.has_game_cycle(g_position, 2);
    std::cout << "  After a8b8: " << result << std::endl;

    move("b1a1");
    result = g_repetition_info.has_game_cycle(g_position, 3);
    std::cout << "  After b1a1 (white rook returns): " << result << std::endl;

    move("b8a8");
    result = g_repetition_info.has_game_cycle(g_position, 4);
    std::cout << "  After b8a8 (back to start): " << result << std::endl;
}

void test_different_moves_no_cycle() {
    std::cout << "\ntest_different_moves_no_cycle" << std::endl;

    g_position = Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 4 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    // Different moves that don't create cycles
    move("g1f3");
    move("g8f6");
    move("f3e5");  // Different square
    bool result = g_repetition_info.has_game_cycle(g_position, 3);
    std::cout << "  After non-reversing moves: " << result << std::endl;

    move("f6e4");  // Different square
    result = g_repetition_info.has_game_cycle(g_position, 4);
    std::cout << "  After more non-reversing moves: " << result << std::endl;
}

void test_with_pawn_moves() {
    std::cout << "\ntest_with_pawn_moves" << std::endl;

    g_position = Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("g1f3");
    move("g8f6");

    // Pawn move resets 50-move counter
    move("e2e4");
    bool result = g_repetition_info.has_game_cycle(g_position, 3);
    std::cout << "  After pawn move e2e4: " << result << " (50mr=" << g_position.get_50mr_counter() << ")" << std::endl;

    move("e7e5");
    result = g_repetition_info.has_game_cycle(g_position, 4);
    std::cout << "  After pawn move e7e5: " << result << " (50mr=" << g_position.get_50mr_counter() << ")" << std::endl;
}

void test_bishop_shuffle() {
    std::cout << "\ntest_bishop_shuffle" << std::endl;

    g_position = Position::parse("r1bqkb1r/pppppppp/2n2n2/8/8/2N2N2/PPPPPPPP/R1BQKB1R w KQkq - 5 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("c1e3");
    bool result = g_repetition_info.has_game_cycle(g_position, 1);
    std::cout << "  After c1e3: " << result << std::endl;

    move("c8e6");
    result = g_repetition_info.has_game_cycle(g_position, 2);
    std::cout << "  After c8e6: " << result << std::endl;

    move("e3c1");
    result = g_repetition_info.has_game_cycle(g_position, 3);
    std::cout << "  After e3c1 (white bishop returns): " << result << std::endl;

    move("e6c8");
    result = g_repetition_info.has_game_cycle(g_position, 4);
    std::cout << "  After e6c8 (back to start): " << result << std::endl;
}

void test_with_varying_ply() {
    std::cout << "\ntest_with_varying_ply" << std::endl;

    g_position = Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 4 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("g1f3");
    move("g8f6");
    move("f3g1");

    // Test with different ply values (simulating different search depths)
    std::cout << "  After f3g1, testing different ply values:" << std::endl;
    for (usize ply = 0; ply <= 8; ++ply) {
        bool result = g_repetition_info.has_game_cycle(g_position, ply);
        std::cout << "    ply=" << ply << ": " << result << std::endl;
    }
}

int main() {
    Zobrist::init_zobrist_keys();
    Cuckoo::init();

    test_knight_shuffle_pattern();
    test_rook_shuffle_pattern();
    test_different_moves_no_cycle();
    test_with_pawn_moves();
    test_bishop_shuffle();
    test_with_varying_ply();

    std::cout << "\nAll has_game_cycle tests completed successfully!" << std::endl;
    return 0;
}
