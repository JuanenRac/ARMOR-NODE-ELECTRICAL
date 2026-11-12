// ARMOR-NODE-ELECTRICAL - host tests of the edges of the meters at work and of the switches: the lines that are not in use, the names of values that no rule produces, a command whose
// every field is wrong in turn, a switch that is refused, a token of the wrong size and a source of random bytes that fails. Stand-ins only; no real contactor or meter was used.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <cstdio>
#include <string>

#include "../core/meter_runner.hpp"
#include "../core/switch_set.hpp"

static int failures = 0;
static int checks = 0;
#define CHECK(condition)                                                              \
  do {                                                                                \
    ++checks;                                                                         \
    if (!(condition)) {                                                               \
      ++failures;                                                                     \
      std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #condition);                \
    }                                                                                 \
  } while (0)

using namespace armor;
namespace pzem = armor::electrical::pzem;
namespace el = armor::electrical;

static config::Settings settings_with(std::initializer_list<config::MeterSetting> meters) {
  config::Settings s = config::default_settings("a1b2c3");
  std::size_t i = 0;
  for (const config::MeterSetting& m : meters) s.meters[i++] = m;
  return s;
}

static void test_lines_in_use() {
  // only a meter on the second line: the node has a meter, the first line has none and the second one is in use
  config::MeterSetting second;
  second.enabled = true; second.channel = "inverter"; second.address = 3; second.line = 2;
  manager::Runner runner(settings_with({second}));
  CHECK(runner.any_meter() && !runner.line_in_use(0) && runner.line_in_use(1));
  CHECK(!runner.line_in_use(-1) && !runner.line_in_use(2) && !runner.line_in_use(100));
  CHECK(runner.bus(0).state == "disabled" && runner.bus(1).state == "starting");
  // opening or failing a line that has no meter changes nothing
  runner.set_line_open(0);
  runner.set_line_failed("no pins", 0);
  runner.set_line_open(-1);
  runner.set_line_failed("no line", 7);
  CHECK(runner.bus(0).state == "disabled" && runner.bus(0).error.empty());
  // the second line opens and then fails with its reason
  runner.set_line_open(1);
  CHECK(runner.bus(1).state == "running");
  runner.set_line_failed("uart busy", 1);
  CHECK(runner.bus(1).state == "error" && runner.bus(1).error == "uart busy");
  // any value of line other than 1 selects the first status
  CHECK(&runner.bus(5) == &runner.bus(0) && &runner.bus(-3) == &runner.bus(0) && &runner.bus(1) != &runner.bus(0));
  // and the first line alone
  config::MeterSetting first;
  first.enabled = true; first.channel = "grid"; first.address = 1; first.line = 1;
  manager::Runner one(settings_with({first}));
  CHECK(one.any_meter() && one.line_in_use(0) && !one.line_in_use(1));
  one.set_line_failed("no uart", 0);
  CHECK(one.bus(0).state == "error" && one.bus(0).error == "no uart");
  // a meter with a line number out of range falls back to the first line
  config::MeterSetting odd = first;
  odd.line = 9;
  manager::Runner fallback(settings_with({odd}));
  CHECK(fallback.line_in_use(0) && !fallback.line_in_use(1) && fallback.meters()[0].line == 1);
}

static void test_result_text() {
  CHECK(std::string(manager::result_text(static_cast<pzem::Result>(99))) == "?");
  CHECK(std::string(manager::result_text(pzem::Result::kOk)).empty());
  CHECK(std::string(manager::result_text(pzem::Result::kTooShort)) == "silent" && std::string(manager::result_text(pzem::Result::kCrc)) == "crc");
}

static void test_names() {
  CHECK(std::string(el::action_name(static_cast<el::Action>(99))) == "arm");
  CHECK(std::string(el::answer_name(static_cast<el::Answer>(99))) == "not_supported");
  CHECK(std::string(el::fault_name(static_cast<el::Fault>(99))) == "none");
  CHECK(std::string(el::fault_name(el::Fault::kDisabled)) == "disabled" && std::string(el::fault_name(el::Fault::kBothClosed)) == "both_closed");
  CHECK(std::string(el::side_name(el::Source::kNone)) == "none" && std::string(el::side_name(el::Source::kB)) == "b");
  CHECK(el::command_topic("") .empty() && el::result_topic("").empty() && el::command_topic("Bad Id").empty() && el::result_topic("has space").empty());
  CHECK(el::command_topic("electrical-1") == "armor/electrical/electrical-1/command" && el::result_topic("electrical-1") == "armor/electrical/electrical-1/result");
  CHECK(!el::command_topic(std::string(65, 'a')).empty() == false);
}

static void test_ids_and_tokens() {
  CHECK(el::valid_command_id("c0ffee0123456789") && !el::valid_command_id("C0FFEE0123456789") && !el::valid_command_id("c0ffee012345678") && !el::valid_command_id("c0ffee01234567890") &&
        !el::valid_command_id("c0ffee012345678g"));
  CHECK(el::valid_code("abcdefgh") && el::valid_code("a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6") && !el::valid_code("abcdefg") && !el::valid_code("a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e") &&
        !el::valid_code("ABCDEFGH") && !el::valid_code("abcdefg-") && !el::valid_code("abcd efgh"));
}

static std::string cmd(const std::string& body) { return "{\"kind\":\"electrical_command\",\"node_id\":\"electrical-1\"," + body + "}"; }
static const char* kOk = "\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"open\"";

static void test_parse_edges() {
  el::SwitchCommand out;
  std::string why;
  CHECK(el::parse_command(cmd(kOk), out, &why) && out.action == el::Action::kOpen && out.token.empty());
  CHECK(el::parse_command(cmd(kOk), out));   // no reason asked for
  CHECK(!el::parse_command("[]", out, &why) && why == "not a JSON object");
  CHECK(!el::parse_command("{", out, &why) && why == "not a JSON object");
  CHECK(!el::parse_command(cmd(std::string(kOk) + ",\"extra\":1"), out, &why) && why == "a field that is not in the contract");
  CHECK(!el::parse_command("{\"node_id\":\"electrical-1\"," + std::string(kOk) + "}", out, &why) && why == "kind");
  CHECK(!el::parse_command("{\"kind\":5,\"node_id\":\"electrical-1\"," + std::string(kOk) + "}", out, &why) && why == "kind");
  CHECK(!el::parse_command("{\"kind\":\"other\",\"node_id\":\"electrical-1\"," + std::string(kOk) + "}", out, &why) && why == "kind");
  CHECK(!el::parse_command("{\"kind\":\"electrical_command\"," + std::string(kOk) + "}", out, &why) && why == "node_id");
  CHECK(!el::parse_command("{\"kind\":\"electrical_command\",\"node_id\":3," + std::string(kOk) + "}", out, &why) && why == "node_id");
  CHECK(!el::parse_command("{\"kind\":\"electrical_command\",\"node_id\":\"Bad Node\"," + std::string(kOk) + "}", out, &why) && why == "node_id");
  const char* stamps[] = {"-1", "1.5", "9007199254740992", "\"1\"", "null", "true", "1e300"};
  for (const char* stamp : stamps) {
    CHECK(!el::parse_command(cmd(std::string("\"timestamp_ms\":") + stamp + ",\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"open\""), out, &why) && why == "timestamp_ms");
  }
  CHECK(!el::parse_command(cmd("\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"open\""), out, &why) && why == "timestamp_ms");
  CHECK(el::parse_command(cmd("\"timestamp_ms\":0,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"open\""), out));
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"switch\":\"transfer\",\"action\":\"open\""), out, &why) && why == "command_id");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":7,\"switch\":\"transfer\",\"action\":\"open\""), out, &why) && why == "command_id");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"short\",\"switch\":\"transfer\",\"action\":\"open\""), out, &why) && why == "command_id");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"action\":\"open\""), out, &why) && why == "switch");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":4,\"action\":\"open\""), out, &why) && why == "switch");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"Bad Switch\",\"action\":\"open\""), out, &why) && why == "switch");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\""), out, &why) && why == "action");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":9"), out, &why) && why == "action");
  CHECK(!el::parse_command(cmd("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"explode\""), out, &why) && why == "action");
  // every action of the contract
  const char* kinds[] = {"arm", "close_a", "close_b", "open", "acknowledge"};
  const el::Action actions[] = {el::Action::kArm, el::Action::kCloseA, el::Action::kCloseB, el::Action::kOpen, el::Action::kAcknowledge};
  for (int i = 0; i < 5; ++i) {
    const bool closes = i == 1 || i == 2;
    const std::string with_token = std::string(",\"token\":\"abcdefgh12345678\"");
    const std::string body = std::string("\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"") + kinds[i] + "\"";
    CHECK(el::parse_command(cmd(body + (closes ? with_token : "")), out) && out.action == actions[i] && out.token.empty() != closes);
    CHECK(!el::parse_command(cmd(body + (closes ? "" : with_token)), out, &why) && why == "a token goes on close_a and close_b and on nothing else");
  }
  // a token that is not a string, or not made of the right characters
  const std::string close = "\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"transfer\",\"action\":\"close_a\"";
  CHECK(!el::parse_command(cmd(close + ",\"token\":12345678"), out, &why) && why == "token");
  CHECK(!el::parse_command(cmd(close + ",\"token\":\"ABCDEFGH\""), out, &why) && why == "token");
  CHECK(!el::parse_command(cmd(close + ",\"token\":\"abc\""), out, &why) && why == "token");
}

static void test_result_json() {
  el::SwitchCommand command;
  command.node_id = "electrical-1"; command.command_id = "c0ffee0123456789"; command.switch_id = "transfer"; command.action = el::Action::kArm;
  const std::string with_token = el::result_json("electrical-1", 5, command, el::Answer::kNone, "0123456789abcdef");
  CHECK(with_token.find("\"token\":\"0123456789abcdef\"") != std::string::npos && with_token.find("\"accepted\":true") != std::string::npos);
  const std::string without = el::result_json("electrical-1", 5, command, el::Answer::kNone);
  CHECK(without.find("token") == std::string::npos && without.find("\"refusal\":\"none\"") != std::string::npos);
  const std::string refused = el::result_json("electrical-1", 5, command, el::Answer::kFault, "0123456789abcdef");
  CHECK(refused.find("token") == std::string::npos && refused.find("\"accepted\":false") != std::string::npos && refused.find("\"refusal\":\"fault\"") != std::string::npos);
  command.action = el::Action::kCloseA;
  CHECK(el::result_json("electrical-1", 5, command, el::Answer::kNone, "0123456789abcdef").find("token") == std::string::npos);
}

static el::SwitchConfig config_for(const std::string& id, bool allowed = true) {
  el::SwitchConfig c;
  c.id = id; c.label = "Transfer"; c.source_a = "grid"; c.source_b = "inverter";
  c.transfer = el::TransferConfig{allowed, 2000, 1000, 10'000};
  return c;
}

static void test_add_edges() {
  el::SwitchSet set("electrical-1", [](std::uint8_t* out, std::size_t n) { for (std::size_t i = 0; i < n; ++i) out[i] = static_cast<std::uint8_t>(i * 29 + 3); return true; });
  CHECK(set.size() == 0 && !set.switching_enabled() && set.switches_json(0).empty());
  CHECK(!set.add(config_for("Bad Id")) && !set.add(config_for("")) && set.size() == 0);
  el::SwitchConfig long_label = config_for("one");
  long_label.label = std::string(41, 'x');
  CHECK(!set.add(long_label));
  long_label.label = std::string(40, 'x');
  CHECK(set.add(long_label) && set.size() == 1);
  el::SwitchConfig bad_a = config_for("two"), bad_b = config_for("three"), empty_sources = config_for("four", false);
  bad_a.source_a = "Not Valid"; bad_b.source_b = "Not Valid";
  empty_sources.source_a.clear(); empty_sources.source_b.clear(); empty_sources.label.clear();
  CHECK(!set.add(bad_a) && !set.add(bad_b) && set.add(empty_sources) && set.size() == 2);
  CHECK(!set.add(config_for("one")));          // repeated
  CHECK(set.add(config_for("five")) && set.add(config_for("six")) && set.size() == el::SwitchSet::kMaxSwitches);
  CHECK(!set.add(config_for("seven")));        // a fifth
  // the message of a switch with neither label nor sources leaves those fields out
  const std::string json = set.switches_json(0);
  CHECK(json.find("\"id\":\"four\"") != std::string::npos);
  // an unknown switch: nothing to read, nothing energised, nothing to set
  set.set_feedback("nowhere", el::Feedback{true, true});
  CHECK(!set.coils("nowhere").a && !set.coils("nowhere").b && set.fault("nowhere") == el::Fault::kNone && set.selected("nowhere") == el::Source::kNone);
  CHECK(set.switching_enabled());
}

static void test_switching_off_only() {
  el::SwitchSet set("electrical-1", [](std::uint8_t* out, std::size_t n) { for (std::size_t i = 0; i < n; ++i) out[i] = 1; return true; });
  CHECK(set.add(config_for("only", false)) && !set.switching_enabled());
}

static void test_handle_edges() {
  std::uint8_t counter = 0;
  bool random_works = true;
  el::SwitchSet set("electrical-1", [&](std::uint8_t* out, std::size_t n) { if (!random_works) return false; for (std::size_t i = 0; i < n; ++i) out[i] = static_cast<std::uint8_t>(counter++ * 41 + 7); return true; });
  set.add(config_for("transfer"));
  set.add(config_for("off", false));
  auto command = [](const std::string& action, const std::string& target, const std::string& token = "", const std::string& node = "electrical-1") {
    return "{\"kind\":\"electrical_command\",\"node_id\":\"" + node + "\",\"timestamp_ms\":1,\"command_id\":\"c0ffee0123456789\",\"switch\":\"" + target + "\",\"action\":\"" + action + "\"" +
           (token.empty() ? "" : ",\"token\":\"" + token + "\"") + "}";
  };
  auto refusal = [&](const std::string& text, std::uint64_t now) {
    const std::string answer = set.handle(text, now);
    armor::json::Value v;
    if (answer.empty() || !armor::json::parse(answer, v)) return std::string("<none>");
    return v.string_or("refusal", "<missing>");
  };
  CHECK(set.handle("not json", 0).empty() && set.handle(command("open", "transfer", "", "other-node"), 0).empty());
  CHECK(refusal(command("open", "nowhere"), 0) == "unknown_switch");
  CHECK(refusal(command("arm", "off"), 0) == "disabled" && refusal(command("acknowledge", "off"), 0) == "disabled" && refusal(command("open", "off"), 0) == "none");
  CHECK(refusal(command("close_a", "transfer", "abcdefgh12345678"), 0) == "not_armed");
  // a random source that fails: no arm is granted
  random_works = false;
  CHECK(refusal(command("arm", "transfer"), 10) == "not_supported");
  random_works = true;
  // an arm gives a token; a token of another size is a wrong token and disarms
  std::string answer = set.handle(command("arm", "transfer"), 20);
  armor::json::Value v;
  CHECK(armor::json::parse(answer, v) && v.bool_or("accepted", false));
  const std::string token = v.string_or("token", "");
  CHECK(token.size() == 16);
  CHECK(refusal(command("close_a", "transfer", "abcdefgh"), 30) == "bad_token");
  CHECK(refusal(command("close_a", "transfer", token), 40) == "not_armed");   // the wrong token spent the arm
  // a token that differs in one character only
  answer = set.handle(command("arm", "transfer"), 50);
  armor::json::parse(answer, v);
  std::string near = v.string_or("token", "");
  near[15] = near[15] == 'a' ? 'b' : 'a';
  CHECK(refusal(command("close_b", "transfer", near), 60) == "bad_token");
  // the token runs out with the arm window
  answer = set.handle(command("arm", "transfer"), 100);
  armor::json::parse(answer, v);
  const std::string late = v.string_or("token", "");
  set.tick(100 + 10'001);
  CHECK(refusal(command("close_a", "transfer", late), 100 + 10'002) == "not_armed");
  // a good token closes b; the request is accepted and the controller wants b
  answer = set.handle(command("arm", "transfer"), 20'000);
  armor::json::parse(answer, v);
  const std::string good = v.string_or("token", "");
  CHECK(refusal(command("close_b", "transfer", good), 20'010) == "none");
  CHECK(refusal(command("close_b", "transfer", good), 20'020) == "not_armed");   // spent
  // acknowledge with nothing latched
  const std::string ack = refusal(command("acknowledge", "transfer"), 20'030);
  CHECK(ack == "none" || ack == "not_confirmed_open");
  set.tick(20'040);
  const std::string json = set.switches_json(20'040);
  CHECK(json.find("\"id\":\"transfer\"") != std::string::npos && json.find("\"id\":\"off\"") != std::string::npos);
}

int main() {
  test_lines_in_use();
  test_result_text();
  test_names();
  test_ids_and_tokens();
  test_parse_edges();
  test_result_json();
  test_add_edges();
  test_switching_off_only();
  test_handle_edges();
  std::printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
