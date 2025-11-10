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
    std::cout << "move: " << movestr << " " << std::hex << std::setw(16)
              << g_position.get_hash_key() << std::endl;
}

void repeat_in_history() {
    std::cout << "repeat_in_history" << std::endl;
    g_position =
      Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);
    std::cout << "startpos: " << std::hex << std::setw(16) << g_position.get_hash_key()
              << std::endl;
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("g1f3");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("g8f6");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("f3g1");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("f6g8");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("g1f3");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("g8f6");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("f3g1");
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("f6g8");
    REQUIRE(g_repetition_info.detect_repetition(0) == true);
}

void repeat_in_search() {
    std::cout << "repeat_in_search" << std::endl;
    g_position =
      Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);
    std::cout << "startpos: " << std::hex << std::setw(16) << g_position.get_hash_key()
              << std::endl;
    REQUIRE(g_repetition_info.detect_repetition(0) == false);
    move("g1f3");
    REQUIRE(g_repetition_info.detect_repetition(1) == false);
    move("g8f6");
    REQUIRE(g_repetition_info.detect_repetition(2) == false);
    move("f3g1");
    REQUIRE(g_repetition_info.detect_repetition(3) == false);
    move("f6g8");
    REQUIRE(g_repetition_info.detect_repetition(4) == true);
    move("g1f3");
    REQUIRE(g_repetition_info.detect_repetition(5) == true);
    move("g8f6");
    REQUIRE(g_repetition_info.detect_repetition(6) == true);
    move("f3g1");
    REQUIRE(g_repetition_info.detect_repetition(7) == true);
    move("f6g8");
    REQUIRE(g_repetition_info.detect_repetition(8) == true);
}

void game_cycle_simple_knight_shuffle() {
    std::cout << "game_cycle_simple_knight_shuffle" << std::endl;
    g_position =
      Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 3 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("g1f3");
    move("g8f6");
    move("f3g1");
    move("f6g8");

    // After completing the shuffle and returning to start, check if another shuffle would repeat
    std::cout << "After f6g8, 50mr counter: " << g_position.get_50mr_counter() << std::endl;
    std::cout << "has_game_cycle result: " << g_repetition_info.has_game_cycle(g_position, 4) << std::endl;
}

void game_cycle_explore() {
    std::cout << "\n=== Exploring has_game_cycle behavior ===" << std::endl;

    // Test case 1: Simple knight shuffle
    std::cout << "\n** Test 1: Knight shuffle **" << std::endl;
    g_position = Position::parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 3 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("g1f3");
    move("g8f6");
    move("f3g1");
    std::cout << "After f3g1, has_game_cycle(ply=3): " << g_repetition_info.has_game_cycle(g_position, 3) << std::endl;
    std::cout << "After f3g1, has_game_cycle(ply=5): " << g_repetition_info.has_game_cycle(g_position, 5) << std::endl;

    move("f6g8");
    std::cout << "After f6g8 (back to start), has_game_cycle(ply=4): " << g_repetition_info.has_game_cycle(g_position, 4) << std::endl;

    // Test case 2: Rook shuffle
    std::cout << "\n** Test 2: Rook shuffle **" << std::endl;
    g_position = Position::parse("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 3 1").value();
    g_repetition_info.reset();
    g_repetition_info.push(g_position.get_hash_key(), false);

    move("a1b1");
    move("a8b8");
    move("b1a1");
    std::cout << "After b1a1, has_game_cycle(ply=3): " << g_repetition_info.has_game_cycle(g_position, 3) << std::endl;
}

int main() {
    Zobrist::init_zobrist_keys();
    Cuckoo::init();

    repeat_in_history();
    repeat_in_search();

    game_cycle_simple_knight_shuffle();
    game_cycle_explore();

    std::cout << "\n=== All tests completed ===" << std::endl;
    return 0;
}
