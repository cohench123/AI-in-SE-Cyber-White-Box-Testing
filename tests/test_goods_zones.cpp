// Unit tests for goods.cpp and the zone-growth predicates in residential.cpp,
// industrial.cpp, and commercial.cpp. Added in the human review pass: the
// LLM-generated suite had no tests for these modules.
//
// Expectations were derived from the source, not copied from the output.
// Where the program's behavior depends on rand(), the tests check invariants
// instead of specific values.

#include "test_harness.h"
#include "../goods.h"
#include "../residential.h"
#include "../industrial.h"
#include "../commercial.h"

#include <queue>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using Grid = vector<vector<pair<char, int>>>;
using Goods = vector<pair<string, int>>;

static int totalDemand(const Goods& g) {
    int sum = 0;
    for (const auto& p : g) sum += p.second;
    return sum;
}

static size_t countLinesContaining(const string& text, const string& needle) {
    size_t count = 0, pos = 0;
    while ((pos = text.find(needle, pos)) != string::npos) {
        count++;
        pos += needle.size();
    }
    return count;
}

// ---------------------------------------------------------------------------
// goods.cpp: fill functions
// ---------------------------------------------------------------------------

TEST(goods_fill_creates_twenty_goods_with_zero_demand) {
    Goods g;
    goodsFill(g);
    CHECK_EQ(g.size(), size_t(20));
    CHECK_EQ(g.front().first, string("Toys"));
    CHECK_EQ(g.back().first, string("Fabrics"));
    CHECK_EQ(totalDemand(g), 0);
}

TEST(supply_fill_has_same_names_in_same_order_as_goods) {
    Goods goods, supply;
    goodsFill(goods);
    supplyFill(supply);
    CHECK_EQ(supply.size(), goods.size());
    for (size_t i = 0; i < goods.size() && i < supply.size(); i++) {
        CHECK_EQ(supply[i].first, goods[i].first);
        CHECK_EQ(supply[i].second, 0);
    }
}

// ---------------------------------------------------------------------------
// goods.cpp: demand changes (addDemand / removeDemand)
// ---------------------------------------------------------------------------

TEST(add_demand_increments_good_and_enqueues_index) {
    Goods g;
    goodsFill(g);
    queue<int> q;
    CoutCapture quiet;
    addDemand(g, 3, q);
    CHECK_EQ(g[3].second, 1);
    CHECK_EQ(totalDemand(g), 1);
    CHECK_EQ(q.size(), size_t(1));
    CHECK_EQ(q.front(), 3);
}

TEST(add_demand_at_lower_boundary_index_zero) {
    Goods g;
    goodsFill(g);
    queue<int> q;
    CoutCapture quiet;
    addDemand(g, 0, q);
    CHECK_EQ(g[0].second, 1);
    CHECK_EQ(q.front(), 0);
}

TEST(add_demand_at_upper_boundary_index_nineteen) {
    Goods g;
    goodsFill(g);
    queue<int> q;
    CoutCapture quiet;
    addDemand(g, 19, q);
    CHECK_EQ(g[19].second, 1);
    CHECK_EQ(q.front(), 19);
}

TEST(add_demand_reports_new_level) {
    Goods g;
    goodsFill(g);
    queue<int> q;
    CoutCapture out;
    addDemand(g, 0, q);
    addDemand(g, 0, q);
    CHECK(out.str().find("New demand level: 2") != string::npos);
    CHECK_EQ(q.size(), size_t(2));
}

TEST(remove_demand_decrements_without_touching_queue) {
    Goods g;
    goodsFill(g);
    g[5].second = 2;
    CoutCapture quiet;
    removeDemand(g, 5);
    CHECK_EQ(g[5].second, 1);
}

TEST(remove_demand_function_allows_negative_demand) {
    // The guard lives in main.cpp (case 5 refuses goods with demand <= 0).
    // removeDemand itself does not check, so this documents that split of
    // responsibility rather than a defect.
    Goods g;
    goodsFill(g);
    CoutCapture quiet;
    removeDemand(g, 0);
    CHECK_EQ(g[0].second, -1);
}

// ---------------------------------------------------------------------------
// goods.cpp: random demand generation
// ---------------------------------------------------------------------------

TEST(generate_demand_adds_one_unit_and_one_queue_entry) {
    Goods g;
    goodsFill(g);
    queue<int> q;
    generateDemand(g, q);
    CHECK_EQ(totalDemand(g), 1);
    CHECK_EQ(q.size(), size_t(1));
    // The queued index must be the good that received the demand.
    CHECK_EQ(g[q.front()].second, 1);
}

TEST(generate_demand_repeated_calls_keep_queue_and_demand_in_step) {
    Goods g;
    goodsFill(g);
    queue<int> q;
    for (int i = 0; i < 25; i++) generateDemand(g, q);
    CHECK_EQ(totalDemand(g), 25);
    CHECK_EQ(q.size(), size_t(25));
}

TEST(generate_demand_index_always_within_twenty_goods) {
    // rand() % 20 must never index outside the 20-element list.
    Goods g;
    goodsFill(g);
    queue<int> q;
    for (int i = 0; i < 200; i++) generateDemand(g, q);
    while (!q.empty()) {
        CHECK(q.front() >= 0 && q.front() < 20);
        q.pop();
    }
}

// ---------------------------------------------------------------------------
// goods.cpp: printing
// ---------------------------------------------------------------------------

TEST(goods_print_lists_all_twenty_entries) {
    Goods g;
    goodsFill(g);
    CoutCapture out;
    goodsPrint(g);
    CHECK_EQ(countLinesContaining(out.str(), "Current demand level:"), size_t(20));
}

TEST(supply_print_lists_all_twenty_entries) {
    Goods s;
    supplyFill(s);
    CoutCapture out;
    supplyPrint(s);
    CHECK_EQ(countLinesContaining(out.str(), "Current supply level:"), size_t(20));
}

TEST(demand_print_empty_queue_prints_header_only) {
    queue<int> q;
    CoutCapture out;
    demandPrint(q);
    CHECK_EQ(out.str(), string("Current demand queue: \n\n"));
}

TEST(demand_print_does_not_consume_queue) {
    queue<int> q;
    q.push(4);
    q.push(9);
    CoutCapture out;
    demandPrint(q);
    CHECK_EQ(q.size(), size_t(2));
    CHECK(out.str().find("4 9 ") != string::npos);
}

// ---------------------------------------------------------------------------
// residential.cpp: growth predicate per population level
// Grid cells are (zone, population). The predicate looks at the 8 neighbours.
// ---------------------------------------------------------------------------

TEST(residential_empty_cell_grows_next_to_powerline) {
    Grid g = {{{'T', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'R', 0}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(residential(1, 1, g));
}

TEST(residential_empty_cell_grows_next_to_powerline_over_road) {
    Grid g = {{{'-', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'R', 0}, {'#', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(residential(1, 1, g));
}

TEST(residential_empty_cell_with_no_power_or_neighbours_does_not_grow) {
    Grid g = {{{'-', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'R', 0}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(!residential(1, 1, g));
}

TEST(residential_empty_cell_grows_next_to_populated_neighbour_without_power) {
    Grid g = {{{'-', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'R', 0}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'R', 1}}};
    CHECK(residential(1, 1, g));
}

TEST(residential_population_one_needs_two_populated_neighbours) {
    Grid two = {{{'-', 0}, {'R', 1}, {'-', 0}},
                {{'R', 1}, {'R', 1}, {'-', 0}},
                {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(residential(1, 1, two));

    Grid one = {{{'-', 0}, {'-', 0}, {'-', 0}},
                {{'R', 1}, {'R', 1}, {'-', 0}},
                {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(!residential(1, 1, one));
}

TEST(residential_population_two_needs_four_neighbours_of_two_or_more) {
    Grid four = {{{'R', 2}, {'R', 2}, {'-', 0}},
                 {{'R', 2}, {'R', 2}, {'R', 2}},
                 {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(residential(1, 1, four));

    Grid three = {{{'R', 2}, {'R', 2}, {'-', 0}},
                  {{'R', 2}, {'R', 2}, {'-', 0}},
                  {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(!residential(1, 1, three));
}

TEST(residential_corner_cell_only_counts_in_bounds_neighbours) {
    // (0,0) has only three in-bounds neighbours, so it cannot reach four.
    Grid g = {{{'R', 2}, {'R', 2}},
              {{'R', 2}, {'-', 0}}};
    CHECK(!residential(0, 0, g));
}

TEST(residential_population_three_and_four_branches) {
    Grid three = {{{'R', 3}, {'R', 3}, {'R', 3}},
                  {{'R', 3}, {'R', 3}, {'R', 3}},
                  {{'R', 3}, {'R', 3}, {'R', 3}}};
    CHECK(residential(1, 1, three));     // 8 neighbours with population >= 3

    Grid four = {{{'R', 4}, {'R', 4}, {'R', 4}},
                 {{'R', 4}, {'R', 4}, {'R', 4}},
                 {{'R', 4}, {'R', 4}, {'R', 4}}};
    CHECK(residential(1, 1, four));      // 8 neighbours with population >= 4

    Grid fiveShort = {{{'R', 4}, {'R', 4}, {'R', 4}},
                      {{'R', 4}, {'R', 4}, {'R', 4}},
                      {{'R', 4}, {'R', 4}, {'R', 3}}};
    CHECK(!residential(1, 1, fiveShort)); // one neighbour below 4 breaks the count
}

// ---------------------------------------------------------------------------
// industrial.cpp: growth needs workers and nearby population
// ---------------------------------------------------------------------------

TEST(industrial_empty_cell_needs_two_workers_next_to_power) {
    vector<vector<int>> pollution(3, vector<int>(3, 0));
    Grid g = {{{'#', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'I', 0}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(industrial(g, pollution, 1, 1, 2, 0));
    CHECK(!industrial(g, pollution, 1, 1, 1, 0));
}

TEST(industrial_empty_cell_without_power_does_not_grow) {
    vector<vector<int>> pollution(3, vector<int>(3, 0));
    Grid g = {{{'-', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'I', 0}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(!industrial(g, pollution, 1, 1, 5, 0));
}

// ---------------------------------------------------------------------------
// commercial.cpp: growth needs workers AND goods
// ---------------------------------------------------------------------------

TEST(commercial_empty_cell_needs_worker_and_goods_next_to_power) {
    Grid g = {{{'T', 0}, {'-', 0}, {'-', 0}},
              {{'-', 0}, {'C', 0}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(commercial(g, 1, 1, 1, 1));
    CHECK(!commercial(g, 1, 1, 0, 1));   // no worker
    CHECK(!commercial(g, 1, 1, 1, 0));   // no goods
}

TEST(commercial_populated_cell_needs_two_neighbours_and_resources) {
    Grid g = {{{'-', 0}, {'R', 1}, {'-', 0}},
              {{'R', 1}, {'C', 1}, {'-', 0}},
              {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(commercial(g, 1, 1, 1, 1));
    // Only one populated neighbour: count < 2, so the else branch returns false.
    Grid one = {{{'-', 0}, {'-', 0}, {'-', 0}},
                {{'R', 1}, {'C', 1}, {'-', 0}},
                {{'-', 0}, {'-', 0}, {'-', 0}}};
    CHECK(!commercial(one, 1, 1, 1, 1));
    // Note: the populated branch with two neighbours but no workers or no goods
    // is deliberately NOT tested. In that case commercial() falls off the end of
    // a non-void function (undefined behaviour; see REVIEW.md). UBSan reports it.
}
