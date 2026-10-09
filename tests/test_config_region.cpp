// Unit tests for config.cpp and region.cpp.
// Run from the repository root so temporary files are created there.
//
// Tests named *_characterizes_* record current behavior that is questionable
// (see the static analysis findings). If behavior is later fixed, update the
// expectation rather than deleting the test.

#include "test_harness.h"
#include "../config.h"
#include "../region.h"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using Grid = vector<vector<pair<char, int>>>;

// Writes a file on construction and deletes it on destruction.
struct TempFile {
    string path;
    TempFile(const string& p, const string& content) : path(p) {
        ofstream f(path, ios::binary);
        f << content;
    }
    ~TempFile() { std::remove(path.c_str()); }
};

// Parses a config string and returns the Config and region name it produced.
static void parseConfig(const string& content, Config& cfg, string& regionName) {
    TempFile f("tc_tmp_config.txt", content);
    cfg.initiateConfig("tc_tmp_config.txt", regionName);
}

// ---------------------------------------------------------------------------
// Config::Config and Config::initiateConfig
// ---------------------------------------------------------------------------

TEST(config_constructor_sets_defaults) {
    Config c;
    CHECK_EQ(c.timeLimit, 0);
    CHECK_EQ(c.refreshRate, 0);
    CHECK(c.regionName.empty());
}

TEST(config_parses_all_three_keys) {
    Config c;
    string region;
    parseConfig("Region Layout:region1.csv\nTime Limit:20\nRefresh Rate:1\n", c, region);
    CHECK_EQ(region, string("region1.csv"));
    CHECK_EQ(c.timeLimit, 20);
    CHECK_EQ(c.refreshRate, 1);
}

TEST(config_strips_multiple_leading_spaces_from_value) {
    Config c;
    string region;
    parseConfig("Time Limit:     7\n", c, region);
    CHECK_EQ(c.timeLimit, 7);
}

TEST(config_accepts_value_with_no_space_after_colon) {
    Config c;
    string region;
    parseConfig("Time Limit:7\n", c, region);
    CHECK_EQ(c.timeLimit, 7);
}

TEST(config_ignores_lines_without_colon) {
    Config c;
    string region;
    parseConfig("this line has no separator\nTime Limit:5\n", c, region);
    CHECK_EQ(c.timeLimit, 5);
}

TEST(config_ignores_unknown_keys) {
    Config c;
    string region;
    parseConfig("Speed:9\nTime Limit:3\n", c, region);
    CHECK_EQ(c.timeLimit, 3);
    CHECK_EQ(c.refreshRate, 0);
    CHECK(region.empty());
}

TEST(config_region_value_keeps_later_colons) {
    Config c;
    string region;
    parseConfig("Region Layout:C:/maps/a.csv\n", c, region);
    CHECK_EQ(region, string("C:/maps/a.csv"));
}

TEST(config_duplicate_key_last_occurrence_wins) {
    Config c;
    string region;
    parseConfig("Time Limit:5\nTime Limit:9\n", c, region);
    CHECK_EQ(c.timeLimit, 9);
}

TEST(config_zero_refresh_rate_is_stored) {
    // Boundary: 0 is stored as-is. main.cpp divides by this value
    // (t % refreshRate), so a zero here is a crash risk tested at CLI level.
    Config c;
    string region;
    parseConfig("Refresh Rate:0\n", c, region);
    CHECK_EQ(c.refreshRate, 0);
}

TEST(config_negative_values_are_stored) {
    Config c;
    string region;
    parseConfig("Time Limit:-4\nRefresh Rate:-2\n", c, region);
    CHECK_EQ(c.timeLimit, -4);
    CHECK_EQ(c.refreshRate, -2);
}

TEST(config_value_on_last_line_without_newline_is_read) {
    Config c;
    string region;
    parseConfig("Time Limit:12", c, region);
    CHECK_EQ(c.timeLimit, 12);
}

TEST(config_characterizes_numeric_prefix_accepted) {
    // stoi parses the leading digits and ignores the rest.
    Config c;
    string region;
    parseConfig("Time Limit:20abc\n", c, region);
    CHECK_EQ(c.timeLimit, 20);
}

TEST(config_non_numeric_time_limit_throws) {
    // Known gap: no try/catch around stoi, so this aborts the program.
    Config c;
    string region;
    bool threw = false;
    try {
        parseConfig("Time Limit:abc\n", c, region);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}

TEST(config_out_of_range_time_limit_throws) {
    Config c;
    string region;
    bool threw = false;
    try {
        parseConfig("Time Limit:99999999999\n", c, region);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

TEST(config_empty_file_leaves_defaults) {
    Config c;
    string region;
    parseConfig("", c, region);
    CHECK_EQ(c.timeLimit, 0);
    CHECK_EQ(c.refreshRate, 0);
    CHECK(region.empty());
}

TEST(config_missing_file_leaves_defaults) {
    Config c;
    string region;
    c.initiateConfig("tc_does_not_exist.txt", region);
    CHECK_EQ(c.timeLimit, 0);
    CHECK_EQ(c.refreshRate, 0);
    CHECK(region.empty());
}

TEST(config_characterizes_crlf_keeps_carriage_return_in_region_name) {
    // Known gap: only leading spaces are trimmed, so a CRLF file leaves '\r'
    // on the region name, and the CSV then fails to open.
    Config c;
    string region;
    parseConfig("Region Layout:region1.csv\r\n", c, region);
    CHECK_EQ(region, string("region1.csv\r"));
}

// ---------------------------------------------------------------------------
// regionFill
// ---------------------------------------------------------------------------

TEST(region_dimensions_match_csv) {
    TempFile f("tc_tmp_region.csv", "R,I,C\nT,#,-\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK_EQ(g.size(), size_t(2));
    CHECK_EQ(g[0].size(), size_t(3));
    CHECK_EQ(g[1].size(), size_t(3));
}

TEST(region_stores_first_character_of_each_cell) {
    TempFile f("tc_tmp_region.csv", "Rzone,I9\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK(g[0][0].first == 'R');
    CHECK(g[0][1].first == 'I');
}

TEST(region_empty_cell_becomes_space) {
    TempFile f("tc_tmp_region.csv", "R,,C\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK_EQ(g[0].size(), size_t(3));
    CHECK(g[0][1].first == ' ');
}

TEST(region_leading_empty_cell_becomes_space) {
    TempFile f("tc_tmp_region.csv", ",R\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK(g[0][0].first == ' ');
    CHECK(g[0][1].first == 'R');
}

TEST(region_characterizes_trailing_comma_is_dropped) {
    // getline does not emit a final empty field after a trailing comma,
    // so "R,C," has two cells, not three.
    TempFile f("tc_tmp_region.csv", "R,C,\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK_EQ(g[0].size(), size_t(2));
}

TEST(region_all_populations_start_at_zero) {
    TempFile f("tc_tmp_region.csv", "R,I\nC,T\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    for (const auto& row : g)
        for (const auto& cell : row)
            CHECK_EQ(cell.second, 0);
}

TEST(region_empty_file_yields_no_rows) {
    TempFile f("tc_tmp_region.csv", "");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK(g.empty());
}

TEST(region_missing_file_yields_no_rows_without_error) {
    // Known gap: no is_open() check and no error reported to the caller.
    Grid g;
    CoutCapture quiet;
    regionFill("tc_does_not_exist.csv", g);
    CHECK(g.empty());
}

TEST(region_blank_line_produces_empty_row) {
    // Boundary: a blank line in the middle becomes a zero-width row.
    TempFile f("tc_tmp_region.csv", "R,C\n\nI,T\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK_EQ(g.size(), size_t(3));
    CHECK(g[1].empty());
}

TEST(region_refill_replaces_previous_contents) {
    TempFile a("tc_tmp_a.csv", "R,R,R\n");
    TempFile b("tc_tmp_b.csv", "I\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_a.csv", g);
    regionFill("tc_tmp_b.csv", g);
    CHECK_EQ(g.size(), size_t(1));
    CHECK_EQ(g[0].size(), size_t(1));
}

TEST(region_prints_initial_state_grid) {
    TempFile f("tc_tmp_region.csv", "R,I\nC,-\n");
    Grid g;
    CoutCapture out;
    regionFill("tc_tmp_region.csv", g);
    CHECK_EQ(out.str(), string("Initial Region State:\nR I \nC - \n"));
}

TEST(region_handles_crlf_line_endings_in_cells) {
    TempFile f("tc_tmp_region.csv", "R,I\r\nC,T\r\n");
    Grid g;
    CoutCapture quiet;
    regionFill("tc_tmp_region.csv", g);
    CHECK_EQ(g.size(), size_t(2));
    CHECK(g[0][1].first == 'I');
    CHECK(g[1][1].first == 'T');
}

// ---------------------------------------------------------------------------
// populationPrint
// ---------------------------------------------------------------------------

TEST(population_print_shows_zone_letter_when_population_zero) {
    Grid g = {{{'R', 0}, {'I', 0}, {'C', 0}}};
    CoutCapture out;
    populationPrint(g);
    CHECK_EQ(out.str(), string("R I C \n"));
}

TEST(population_print_shows_population_for_zoned_cells) {
    Grid g = {{{'R', 3}, {'I', 1}, {'C', 12}}};
    CoutCapture out;
    populationPrint(g);
    CHECK_EQ(out.str(), string("3 1 12 \n"));
}

TEST(population_print_ignores_population_on_non_zone_cells) {
    Grid g = {{{'#', 5}, {'T', 2}, {'-', 9}, {' ', 4}}};
    CoutCapture out;
    populationPrint(g);
    // Each cell prints its character followed by a space; the blank cell gives two spaces.
    CHECK_EQ(out.str(), string("# T -   \n"));
}

TEST(population_print_empty_region_prints_nothing) {
    Grid g;
    CoutCapture out;
    populationPrint(g);
    CHECK(out.str().empty());
}

// ---------------------------------------------------------------------------
// dataPrint
// ---------------------------------------------------------------------------

TEST(data_print_reports_all_five_values) {
    Grid g;
    CoutCapture out;
    dataPrint(g, 42, 7, 3, 5);
    string s = out.str();
    CHECK(s.find("Current Timestep: 7") != string::npos);
    CHECK(s.find("Current Population: 42") != string::npos);
    CHECK(s.find("Available Workers: 3") != string::npos);
    CHECK(s.find("Available Goods: 5") != string::npos);
}

TEST(data_print_zero_values_boundary) {
    Grid g;
    CoutCapture out;
    dataPrint(g, 0, 0, 0, 0);
    string s = out.str();
    CHECK(s.find("Current Timestep: 0") != string::npos);
    CHECK(s.find("Current Population: 0") != string::npos);
}

// ---------------------------------------------------------------------------
// searchPrint (caller is responsible for bounds, as in main.cpp)
// ---------------------------------------------------------------------------

TEST(search_print_single_cell_region) {
    Grid g = {{{'R', 0}, {'I', 0}}, {{'C', 0}, {'T', 0}}};
    CoutCapture out;
    searchPrint(g, 1, 1, 1, 1);
    CHECK_EQ(out.str(), string("T \n"));
}

TEST(search_print_extracts_only_requested_rectangle) {
    Grid g = {
        {{'R', 1}, {'R', 2}, {'R', 3}},
        {{'I', 4}, {'I', 5}, {'I', 6}},
        {{'C', 7}, {'C', 8}, {'C', 9}},
    };
    CoutCapture out;
    searchPrint(g, 1, 1, 2, 2);
    CHECK_EQ(out.str(), string("5 6 \n8 9 \n"));
}

TEST(search_print_full_region_matches_grid_size) {
    Grid g = {{{'R', 0}, {'I', 0}}, {{'C', 0}, {'-', 0}}};
    CoutCapture out;
    searchPrint(g, 0, 0, 1, 1);
    CHECK_EQ(out.str(), string("R I \nC - \n"));
}

// ---------------------------------------------------------------------------
// resetGrowth and pollutionFill
// ---------------------------------------------------------------------------

TEST(reset_growth_clears_all_flags_in_bounds) {
    vector<vector<bool>> growth(2, vector<bool>(3, true));
    resetGrowth(growth, 2, 3);
    for (const auto& row : growth)
        for (bool b : row)
            CHECK(!b);
}

TEST(reset_growth_zero_size_is_noop) {
    vector<vector<bool>> growth;
    resetGrowth(growth, 0, 0);
    CHECK(growth.empty());
}

TEST(pollution_fill_zeroes_all_cells_in_bounds) {
    vector<vector<int>> p(2, vector<int>(2, 7));
    pollutionFill(p, 2, 2);
    for (const auto& row : p)
        for (int v : row)
            CHECK_EQ(v, 0);
}
