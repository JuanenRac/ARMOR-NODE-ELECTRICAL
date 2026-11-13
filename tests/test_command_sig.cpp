// ARMOR-NODE-ELECTRICAL - host tests of the signature of the commands of the server: the vectors of ARMOR-COMMON (conformance/signatures/command.json, written here as they are), the
// refusals one by one, the memory of nonces, the freshness against a valid clock, and the switch set that drops a command that does not verify. No hardware was used.
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

#include "../core/command_sig.hpp"
#include "../core/electrical_config.hpp"
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

// The key of "electrical-1", a close of its transfer switch signed with it (timestamp 1000002 ms), and the arm of "alarm-1" signed with the key of another node.
static const char* kKey = "9d2bf6e333a39afd9ff219bdda4df1833697f37f9c0d7577be589d592f702652";
static const char* kAlarmKey = "156a3ba893545ebfde6009ac5393395fcb033e467f7ee6fb1839548da19a66b3";
static const char* kClose =
    "{\"kind\":\"electrical_command\",\"node_id\":\"electrical-1\",\"timestamp_ms\":1000002,\"command_id\":\"c000000000000003\",\"switch\":\"transfer\",\"action\":\"close_a\","
    "\"token\":\"00112233445566778899aabbccddeeff\",\"nonce\":\"0f1e2d3c4b5a6978\",\"sig\":\"62c8b69062d15b4c2ae9f40b3ae1e8d5185c8fa8af90fc3f29996c46f0b9d7c0\"}";

static std::string replaced(std::string text, const std::string& from, const std::string& to) {
  const std::size_t at = text.find(from);
  if (at != std::string::npos) text.replace(at, from.size(), to);
  return text;
}

static void test_the_vector_verifies_and_every_change_of_it_does_not() {
  command_sig::NonceCache cache;
  const std::string good = kClose;
  CHECK(command_sig::verify(good, "", cache, 0) == command_sig::Verdict::kNoKey);
  CHECK(command_sig::verify(good, kAlarmKey, cache, 0) == command_sig::Verdict::kBadSignature);
  CHECK(command_sig::verify(replaced(good, "close_a", "close_b"), kKey, cache, 0) == command_sig::Verdict::kBadSignature);
  CHECK(command_sig::verify(replaced(good, "\"transfer\"", "\"other\""), kKey, cache, 0) == command_sig::Verdict::kBadSignature);
  CHECK(command_sig::verify(replaced(good, "00112233", "00112234"), kKey, cache, 0) == command_sig::Verdict::kBadSignature);   // the token is signed over too
  CHECK(command_sig::verify(replaced(good, "0f1e2d3c4b5a6978", "0f1e2d3c4b5a6979"), kKey, cache, 0) == command_sig::Verdict::kBadSignature);
  CHECK(command_sig::verify(good + "\n", kKey, cache, 0) == command_sig::Verdict::kNoSignature);
  CHECK(command_sig::verify(replaced(good, "62c8b690", "62C8B690"), kKey, cache, 0) == command_sig::Verdict::kNoSignature);
  CHECK(command_sig::verify("{\"kind\":\"electrical_command\"}", kKey, cache, 0) == command_sig::Verdict::kNoSignature);
  // every byte of the body, changed in a single bit, breaks it
  std::string body;
  std::string_view sig;
  CHECK(command_sig::split(good, body, sig));
  for (std::size_t i = 0; i + 2 < good.size() - 74; ++i) {
    std::string changed = good;
    changed[i] = static_cast<char>(changed[i] ^ 1);
    CHECK(command_sig::verify(changed, kKey, cache, 0) != command_sig::Verdict::kOk);
  }
  CHECK(command_sig::verify(good, kKey, cache, 0) == command_sig::Verdict::kOk);   // the refusals spent nothing
  CHECK(command_sig::verify(good, kKey, cache, 0) == command_sig::Verdict::kReplay);
}

static void test_the_freshness_and_the_memory() {
  command_sig::NonceCache cache;
  CHECK(command_sig::verify(kClose, kKey, cache, 1000002 + 119999) == command_sig::Verdict::kOk);
  command_sig::NonceCache later;
  CHECK(command_sig::verify(kClose, kKey, later, 1000002 + 120001) == command_sig::Verdict::kStale && !later.seen("0f1e2d3c4b5a6978"));
  command_sig::NonceCache full;
  CHECK(command_sig::verify(kClose, kKey, full, 0) == command_sig::Verdict::kOk);
  for (unsigned i = 0; i < command_sig::kNonceMemory; ++i) full.remember(std::string("000000000000000") + "0123456789abcdef"[i % 16]);
  CHECK(!full.seen("0f1e2d3c4b5a6978"));
}

static void test_the_switch_set_takes_only_signed_commands_once_it_has_a_key() {
  const auto random = [](std::uint8_t* out, std::size_t n) { for (std::size_t i = 0; i < n; ++i) out[i] = static_cast<std::uint8_t>(i * 7 + 1); return true; };
  // no key: the command is taken as it always was, and the parser knows the fields nonce and sig
  {
    electrical::SwitchSet set("electrical-1", random);
    electrical::SwitchConfig config;
    config.id = "transfer";
    CHECK(set.add(config));
    CHECK(!set.handle(kClose, 0).empty() && set.last_verdict() == command_sig::Verdict::kOk);
  }
  // with the key: the signed one is answered once; a repeat, a changed one and an unsigned one get no answer
  {
    electrical::SwitchSet set("electrical-1", random);
    electrical::SwitchConfig config;
    config.id = "transfer";
    CHECK(set.add(config));
    set.set_command_key(kKey);
    CHECK(set.handle(replaced(kClose, "close_a", "close_b"), 0).empty() && set.last_verdict() == command_sig::Verdict::kBadSignature);
    const std::string unsigned_close = "{\"kind\":\"electrical_command\",\"node_id\":\"electrical-1\",\"timestamp_ms\":1000002,\"command_id\":\"c000000000000003\",\"switch\":\"transfer\",\"action\":\"open\"}";
    CHECK(set.handle(unsigned_close, 0).empty() && set.last_verdict() == command_sig::Verdict::kNoSignature);
    CHECK(!set.handle(kClose, 0).empty() && set.last_verdict() == command_sig::Verdict::kOk);
    CHECK(set.handle(kClose, 0).empty() && set.last_verdict() == command_sig::Verdict::kReplay);
    CHECK(set.handle(kClose, 0, 1000002 + 500000).empty());
    // taking the key away goes back to taking any command of the contract
    set.set_command_key("");
    CHECK(!set.handle(kClose, 0).empty() && set.last_verdict() == command_sig::Verdict::kOk);
  }
  // a command for another node is not answered even when it verifies with the key
  {
    electrical::SwitchSet set("electrical-2", random);
    electrical::SwitchConfig config;
    config.id = "transfer";
    CHECK(set.add(config));
    set.set_command_key(kKey);
    CHECK(set.handle(kClose, 0).empty());
  }
}

// Whether the settings are refused because of the command key (a node with no meter is refused for other reasons).
static bool key_refused(const config::Settings& s) {
  for (const config::Problem& problem : config::validate(s)) if (problem.path == "command_key") return true;
  return false;
}

static void test_the_key_is_a_secret_of_the_settings() {
  config::Settings s = config::default_settings("a1b2c3");
  s.command_key = kKey;
  const std::string shown = config::to_json(s, false);
  CHECK(shown.find(kKey) == std::string::npos && shown.find("\"command_key_set\":true") != std::string::npos);
  CHECK(config::to_json(s, true).find(std::string("\"command_key\":\"") + kKey + "\"") != std::string::npos);
  CHECK(!key_refused(s));
  s.command_key = "abc";
  CHECK(key_refused(s));
  s.command_key = std::string(64, 'G');
  CHECK(key_refused(s));
  s.command_key = std::string(64, 'A');
  CHECK(key_refused(s));   // capitals are not the format
  s.command_key.clear();
  CHECK(!key_refused(s) && config::to_json(s, false).find("\"command_key_set\":false") != std::string::npos);
}

int main() {
  test_the_vector_verifies_and_every_change_of_it_does_not();
  test_the_freshness_and_the_memory();
  test_the_switch_set_takes_only_signed_commands_once_it_has_a_key();
  test_the_key_is_a_secret_of_the_settings();
  std::printf("%d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
