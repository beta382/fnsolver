#include <fnsolver/data/fnsite.h>

#include <fnsolver/data/precious_resource.h>

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace {
// cannot be derived from sites_mutable, since for convenience and readability, that relies on a mapping from site_id to
// site_idx, which this provides via idx_for_id
const std::array<FnSite::id_t, FnSite::num_sites> site_ids = {
  101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
  201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225,
  301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322,
  401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420,
  501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516
};
} // namespace

// must be initialized before sites_mutable, to translate neighbor_ids
// static
const std::unordered_map<FnSite::id_t, size_t> FnSite::idx_for_id = []() {
  std::unordered_map<id_t, size_t> idx_for_id;
  for (size_t site_idx = 0; site_idx < site_ids.size(); ++site_idx) {
    idx_for_id.emplace(site_ids.at(site_idx), site_idx);
  }
  return idx_for_id;
}();

namespace {
const std::unordered_map<char, uint32_t> productions = {
  {'A', 500},
  {'B', 350},
  {'C', 250}
};

const std::unordered_map<char, uint32_t> revenues = { // apparently the internal values are double the real yield
  {'S', 1700}, // Not shown in-game
  {'A', 1500},
  {'B', 1300},
  {'C', 1100},
  {'D', 900},
  {'E', 600},
  {'F', 400}
};

std::array<FnSite, FnSite::num_sites> sites_mutable = {
  // Primordia
  FnSite(101, productions.at('C'), revenues.at('S'), 1, {105}, {}), // 'A' revenue in-game
  FnSite(102, productions.at('C'), revenues.at('F'), 0, {104}, {}),
  FnSite(103, productions.at('C'), revenues.at('E'), 1, {105, 106, 222}, {}),
  FnSite(104, productions.at('C'), revenues.at('S'), 1, {102, 106}, {}), // 'A' revenue in-game
  FnSite(105, productions.at('A'), revenues.at('F'), 0, {101, 103, 109}, {}),
  FnSite(106, productions.at('B'), revenues.at('E'), 1, {103, 104, 107}, {{precious_resource::Type::arc_sand_ore, 16}}),
  FnSite(107, productions.at('A'), revenues.at('F'), 0, {106, 110}, {}),
  FnSite(108, productions.at('C'), revenues.at('F'), 0, {109}, {
    {precious_resource::Type::aurorite, 16},
    {precious_resource::Type::arc_sand_ore, 32},
    {precious_resource::Type::foucaultium, 20}
  }),
  FnSite(109, productions.at('C'), revenues.at('D'), 0, {105, 108}, {
    {precious_resource::Type::lionbone_bort, 72},
    {precious_resource::Type::foucaultium, 84},
    {precious_resource::Type::dawnstone, 84}
  }),
  FnSite(110, productions.at('C'), revenues.at('E'), 1, {107, 111, 112}, {
    {precious_resource::Type::aurorite, 32},
    {precious_resource::Type::arc_sand_ore, 64},
    {precious_resource::Type::white_cometite, 76},
    {precious_resource::Type::dawnstone, 112}
  }),
  FnSite(111, productions.at('C'), revenues.at('F'), 0, {110, 113}, {{precious_resource::Type::foucaultium, 20}}),
  FnSite(112, productions.at('A'), revenues.at('F'), 0, {110, 114, 115}, {}),
  FnSite(113, productions.at('C'), revenues.at('C'), 0, {111, 409}, {}),
  FnSite(114, productions.at('C'), revenues.at('E'), 0, {112, 116}, {}),
  FnSite(115, productions.at('C'), revenues.at('D'), 0, {112}, {
    {precious_resource::Type::arc_sand_ore, 48},
    {precious_resource::Type::white_cometite, 84},
    {precious_resource::Type::lionbone_bort, 72}
  }),
  FnSite(116, productions.at('A'), revenues.at('D'), 0, {114, 117}, {}),
  FnSite(117, productions.at('A'), revenues.at('D'), 1, {116, 118, 120}, {}),
  FnSite(118, productions.at('C'), revenues.at('E'), 0, {117, 121}, {
    {precious_resource::Type::aurorite, 48},
    {precious_resource::Type::white_cometite, 84},
    {precious_resource::Type::dawnstone, 57}
  }),
  FnSite(119, productions.at('C'), revenues.at('E'), 0, {120}, {}),
  FnSite(120, productions.at('B'), revenues.at('B'), 0, {117, 119}, {}),
  FnSite(121, productions.at('A'), revenues.at('E'), 0, {118, 301}, {}),
  // Noctilum
  FnSite(201, productions.at('C'), revenues.at('B'), 0, {206}, {}),
  FnSite(202, productions.at('C'), revenues.at('C'), 0, {203, 207, 208}, {
    {precious_resource::Type::cimmerian_cinnabar, 38},
    {precious_resource::Type::everfreeze_ore, 38}
  }),
  FnSite(203, productions.at('C'), revenues.at('A'), 0, {202, 204}, {
    {precious_resource::Type::cimmerian_cinnabar, 19}
  }),
  FnSite(204, productions.at('A'), revenues.at('C'), 0, {203, 205, 211, 212}, {}),
  FnSite(205, productions.at('A'), revenues.at('F'), 0, {204, 209}, {}),
  FnSite(206, productions.at('B'), revenues.at('A'), 0, {201, 207, 213}, {}),
  FnSite(207, productions.at('C'), revenues.at('C'), 0, {202, 206}, {
    {precious_resource::Type::infernium, 112},
    {precious_resource::Type::white_cometite, 36},
    {precious_resource::Type::cimmerian_cinnabar, 76},
    {precious_resource::Type::foucaultium, 36}
  }),
  FnSite(208, productions.at('B'), revenues.at('D'), 0, {202}, {{precious_resource::Type::foucaultium, 38}}),
  FnSite(209, productions.at('C'), revenues.at('F'), 0, {205}, {}),
  FnSite(210, productions.at('B'), revenues.at('D'), 0, {211}, {}),
  FnSite(211, productions.at('A'), revenues.at('D'), 0, {204, 210}, {}),
  FnSite(212, productions.at('B'), revenues.at('E'), 0, {204, 216}, {
    {precious_resource::Type::aurorite, 48},
    {precious_resource::Type::enduron_lead, 27},
    {precious_resource::Type::white_cometite, 27}
  }),
  FnSite(213, productions.at('C'), revenues.at('S'), 1, {206}, {}), // 'A' revenue in-game
  FnSite(214, productions.at('C'), revenues.at('D'), 2, {215}, {}),
  FnSite(215, productions.at('C'), revenues.at('D'), 0, {214, 218}, {
    {precious_resource::Type::aurorite, 64},
    {precious_resource::Type::enduron_lead, 36},
    {precious_resource::Type::everfreeze_ore, 112},
    {precious_resource::Type::foucaultium, 36}
  }),
  FnSite(216, productions.at('C'), revenues.at('A'), 1, {212, 218, 225}, {}),
  FnSite(217, productions.at('C'), revenues.at('C'), 0, {222}, {
    {precious_resource::Type::aurorite, 48},
    {precious_resource::Type::infernium, 57},
    {precious_resource::Type::cimmerian_cinnabar, 57}
  }),
  FnSite(218, productions.at('C'), revenues.at('E'), 0, {215, 216, 224}, {
    {precious_resource::Type::aurorite, 48},
    {precious_resource::Type::enduron_lead, 27},
    {precious_resource::Type::white_cometite, 27}
  }),
  FnSite(219, productions.at('C'), revenues.at('E'), 0, {220}, {
    {precious_resource::Type::enduron_lead, 18},
    {precious_resource::Type::white_cometite, 18}
  }),
  FnSite(220, productions.at('C'), revenues.at('C'), 1, {219, 221, 225}, {
    {precious_resource::Type::infernium, 56},
    {precious_resource::Type::everfreeze_ore, 56}
  }),
  FnSite(221, productions.at('C'), revenues.at('E'), 2, {220, 222}, {}),
  FnSite(222, productions.at('C'), revenues.at('D'), 1, {217, 221, 103}, {}),
  FnSite(223, productions.at('C'), revenues.at('F'), 1, {224}, {}),
  FnSite(224, productions.at('C'), revenues.at('A'), 0, {218, 223}, {}),
  FnSite(225, productions.at('C'), revenues.at('A'), 1, {216, 220}, {}),
  // Oblivia
  FnSite(301, productions.at('B'), revenues.at('D'), 0, {121, 302, 303}, {
    {precious_resource::Type::infernium, 27},
    {precious_resource::Type::arc_sand_ore, 96},
    {precious_resource::Type::lionbone_bort, 48}
  }),
  FnSite(302, productions.at('C'), revenues.at('E'), 0, {301}, {}),
  FnSite(303, productions.at('C'), revenues.at('E'), 0, {301, 306}, {
    {precious_resource::Type::aurorite, 32},
    {precious_resource::Type::white_cometite, 38}
  }),
  FnSite(304, productions.at('B'), revenues.at('A'), 0, {305, 306, 309}, {}),
  FnSite(305, productions.at('C'), revenues.at('E'), 0, {304, 308}, {
    {precious_resource::Type::aurorite, 72},
    {precious_resource::Type::arc_sand_ore, 48},
    {precious_resource::Type::enduron_lead, 114}
  }),
  FnSite(306, productions.at('C'), revenues.at('D'), 1, {303, 304, 307}, {}),
  FnSite(307, productions.at('C'), revenues.at('B'), 0, {306, 313}, {
    {precious_resource::Type::infernium, 36},
    {precious_resource::Type::arc_sand_ore, 64},
    {precious_resource::Type::enduron_lead, 76},
    {precious_resource::Type::white_cometite, 36}
  }),
  FnSite(308, productions.at('B'), revenues.at('C'), 0, {305}, {{precious_resource::Type::ouroboros_crystal, 28}}),
  FnSite(309, productions.at('C'), revenues.at('C'), 0, {304, 311}, {
    {precious_resource::Type::enduron_lead, 38},
    {precious_resource::Type::ouroboros_crystal, 38}
  }),
  FnSite(310, productions.at('C'), revenues.at('A'), 0, {311}, {}),
  FnSite(311, productions.at('C'), revenues.at('B'), 0, {309, 310}, {}),
  FnSite(312, productions.at('C'), revenues.at('D'), 0, {313, 315}, {
    {precious_resource::Type::infernium, 27},
    {precious_resource::Type::boiled_egg_ore, 57},
    {precious_resource::Type::lionbone_bort, 24}
  }),
  FnSite(313, productions.at('C'), revenues.at('E'), 2, {307, 312, 314}, {}),
  FnSite(314, productions.at('C'), revenues.at('B'), 0, {313}, {}),
  FnSite(315, productions.at('A'), revenues.at('S'), 2, {312, 316, 318, 321}, {}), // 'A' revenue in-game
  FnSite(316, productions.at('C'), revenues.at('D'), 0, {315}, {}),
  FnSite(317, productions.at('C'), revenues.at('A'), 1, {318, 319}, {}),
  FnSite(318, productions.at('C'), revenues.at('B'), 2, {315, 317}, {
    {precious_resource::Type::boiled_egg_ore, 114},
    {precious_resource::Type::white_cometite, 27},
    {precious_resource::Type::lionbone_bort, 48}
  }),
  FnSite(319, productions.at('C'), revenues.at('D'), 1, {317}, {
    {precious_resource::Type::infernium, 18},
    {precious_resource::Type::boiled_egg_ore, 38}
  }),
  FnSite(320, productions.at('C'), revenues.at('B'), 0, {321}, {
    {precious_resource::Type::aurorite, 32},
    {precious_resource::Type::ouroboros_crystal, 18}
  }),
  FnSite(321, productions.at('A'), revenues.at('D'), 0, {315, 320, 322}, {}),
  FnSite(322, productions.at('A'), revenues.at('A'), 0, {321}, {}),
  // Sylvalum
  FnSite(401, productions.at('C'), revenues.at('B'), 0, {402, 404}, {
    {precious_resource::Type::parhelion_platinum, 76},
    {precious_resource::Type::marine_rutile, 60}
  }),
  FnSite(402, productions.at('A'), revenues.at('B'), 0, {401, 408}, {}),
  FnSite(403, productions.at('A'), revenues.at('C'), 0, {405}, {}),
  FnSite(404, productions.at('B'), revenues.at('S'), 1, {401, 407}, {}), // 'A' revenue in-game
  FnSite(405, productions.at('A'), revenues.at('E'), 0, {403, 408, 409}, {{precious_resource::Type::arc_sand_ore, 16}}),
  FnSite(406, productions.at('C'), revenues.at('B'), 0, {408}, {}),
  FnSite(407, productions.at('A'), revenues.at('B'), 0, {404, 412}, {}),
  FnSite(408, productions.at('B'), revenues.at('D'), 1, {402, 405, 406, 413}, {
    {precious_resource::Type::aurorite, 72},
    {precious_resource::Type::arc_sand_ore, 24},
    {precious_resource::Type::everfreeze_ore, 57}
  }),
  FnSite(409, productions.at('B'), revenues.at('S'), 0, {113, 405, 411}, {}), // 'A' revenue in-game
  FnSite(410, productions.at('C'), revenues.at('S'), 1, {412}, {}), // 'A' revenue in-game
  FnSite(411, productions.at('A'), revenues.at('A'), 0, {409, 414}, {}),
  FnSite(412, productions.at('A'), revenues.at('B'), 0, {407, 410, 415}, {}),
  FnSite(413, productions.at('C'), revenues.at('A'), 1, {408, 416}, {}),
  FnSite(414, productions.at('C'), revenues.at('B'), 2, {411}, {
    {precious_resource::Type::parhelion_platinum, 38},
    {precious_resource::Type::marine_rutile, 60}
  }),
  FnSite(415, productions.at('C'), revenues.at('S'), 0, {412, 502}, {}), // 'A' revenue in-game
  FnSite(416, productions.at('C'), revenues.at('B'), 0, {413, 418, 419}, {}),
  FnSite(417, productions.at('B'), revenues.at('D'), 0, {419}, {
    {precious_resource::Type::everfreeze_ore, 38},
    {precious_resource::Type::boiled_egg_ore, 38}
  }),
  FnSite(418, productions.at('C'), revenues.at('C'), 0, {416}, {
    {precious_resource::Type::parhelion_platinum, 95},
    {precious_resource::Type::arc_sand_ore, 40},
    {precious_resource::Type::everfreeze_ore, 95},
    {precious_resource::Type::boiled_egg_ore, 95},
    {precious_resource::Type::marine_rutile, 95}
  }),
  FnSite(419, productions.at('C'), revenues.at('S'), 1, {416, 417, 420}, {}), // 'A' revenue in-game
  FnSite(420, productions.at('B'), revenues.at('C'), 0, {419}, {{precious_resource::Type::everfreeze_ore, 19}}),
  // Cauldros
  FnSite(501, productions.at('B'), revenues.at('F'), 0, {502}, {{precious_resource::Type::arc_sand_ore, 16}}),
  FnSite(502, productions.at('A'), revenues.at('C'), 1, {415, 501, 503}, {{precious_resource::Type::bonjelium, 40}}),
  FnSite(503, productions.at('C'), revenues.at('D'), 1, {502, 504}, {{precious_resource::Type::enduron_lead, 19}}),
  FnSite(504, productions.at('C'), revenues.at('C'), 0, {503, 508}, {
    {precious_resource::Type::bonjelium, 80},
    {precious_resource::Type::arc_sand_ore, 32},
    {precious_resource::Type::enduron_lead, 40},
    {precious_resource::Type::marine_rutile, 76}
  }),
  FnSite(505, productions.at('C'), revenues.at('B'), 2, {506, 509}, {}),
  FnSite(506, productions.at('C'), revenues.at('B'), 1, {505}, {
    {precious_resource::Type::bonjelium, 40},
    {precious_resource::Type::arc_sand_ore, 16}
  }),
  FnSite(507, productions.at('C'), revenues.at('A'), 1, {508}, {{precious_resource::Type::bonjelium, 20}}),
  FnSite(508, productions.at('A'), revenues.at('B'), 1, {504, 507, 509, 511}, {
    {precious_resource::Type::enduron_lead, 20},
    {precious_resource::Type::marine_rutile, 38}
  }),
  FnSite(509, productions.at('A'), revenues.at('A'), 0, {505, 508, 510, 513}, {}),
  FnSite(510, productions.at('C'), revenues.at('B'), 0, {509}, {{precious_resource::Type::bonjelium, 40}}),
  FnSite(511, productions.at('A'), revenues.at('C'), 0, {508, 512, 514}, {{precious_resource::Type::bonjelium, 40}}),
  FnSite(512, productions.at('C'), revenues.at('A'), 0, {511}, {}),
  FnSite(513, productions.at('C'), revenues.at('A'), 2, {509, 516}, {}),
  FnSite(514, productions.at('C'), revenues.at('A'), 1, {511, 515}, {}),
  FnSite(515, productions.at('C'), revenues.at('B'), 0, {514}, {}),
  FnSite(516, productions.at('B'), revenues.at('E'), 0, {513}, {})
};
} // namespace

// kinda nasty, but gives me the semantics I want
// static
const std::array<FnSite, FnSite::num_sites> &FnSite::sites = sites_mutable;

// static
void FnSite::override_territories(id_t site_id, uint32_t territories) {
  sites_mutable[idx_for_id.at(site_id)].territories = territories;
}

FnSite::FnSite(
    id_t site_id,
    uint32_t production,
    uint32_t revenue,
    uint32_t territories,
    std::vector<id_t> neighbor_ids,
    std::unordered_map<precious_resource::Type, uint32_t> quantity_for_precious_resource_type)
    : site_id(site_id),
      production(production),
      revenue(revenue),
      territories(territories),
      neighbor_idxs([&]() {
        std::vector<size_t> neighbor_idxs;
        for (const id_t neighbor_id : neighbor_ids) {
          neighbor_idxs.push_back(idx_for_id.at(neighbor_id));
        }
        return neighbor_idxs;
      }()),
      precious_resource_quantities([&]() {
        std::array<uint32_t, precious_resource::count> precious_resource_quantities;
        precious_resource_quantities.fill(0);
        for (const auto &[precious_resource_type, quantity] : quantity_for_precious_resource_type) {
          precious_resource_quantities[static_cast<size_t>(precious_resource_type)] = quantity;
        }
        return precious_resource_quantities;
      }()) {}

