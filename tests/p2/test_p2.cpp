// tests/p2/test_p2.cpp

#include "core/conversation.h"
#include "core/message.h"

#include <string>
#include <string_view>

// Allows this test file to check pending_ for required test #9.
#define private public
#include "core/sentinel_scanner.h"
#undef private

#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <cassert>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>


// Helpers needed for the Harness tests
class TestInput : public InputSource {
public:
    TestInput(std::vector<std::string> lines) {
        lines_ = lines;
    }

    std::string read_line() override {
        if (index_ >= lines_.size()) {
            return "";
        }

        return lines_[index_++];
    }

    bool is_eof() const override {
        return index_ >= lines_.size();
    }

private:
    std::vector<std::string> lines_;
    std::size_t index_ = 0;
};


class TestOutput : public OutputSink {
public:
    void write(std::string_view text) override {
        text_ += text;
    }

    const std::string& text() const {
        return text_;
    }

private:
    std::string text_;
};


class TestModel : public ModelClient {
public:
    TestModel(std::vector<std::string> replies) {
        replies_ = replies;
    }

    void generate(const Conversation& conv, TokenSink& sink) override {
        if (index_ < replies_.size()) {
            sink.on_chunk(replies_[index_]);
            index_++;
        }

        sink.on_complete();
    }

private:
    std::vector<std::string> replies_;
    std::size_t index_ = 0;
};


// 1. Empty Conversation Bounds
void test_empty_conversation_bounds() {

    Conversation conv;

    assert(conv.size() == 0 &&
           "empty conversation must have size 0");

    assert(conv.begin() == conv.end() &&
           "begin and end must be equal when empty");

    bool threw_error = false;

    try {
        conv.at(0);
    }
    catch (const std::out_of_range&) {
        threw_error = true;
    }

    assert(threw_error &&
           "at(0) must throw when conversation is empty");
}



// 2. System Message Ordering
void test_system_message_ordering() {

    Conversation conv;

    conv.append(Message(Role::System, "System"));
    conv.append(Message(Role::User, "Hello"));
    conv.append(Message(Role::Assistant, "Hi"));

    assert(conv.at(0).role() == Role::System &&
           "system message must stay first");

    assert(conv.at(1).role() == Role::User);

    assert(conv.at(2).role() == Role::Assistant);
}


// 3. Rule of Five - Copy
void test_copy() {

    Conversation original;

    original.append(Message(Role::User, "Hello"));
    original.append(Message(Role::Assistant, "Hi"));

    Conversation copy(original);

    assert(copy.begin() != original.begin() &&
           "copy must have a different data pointer");

    assert(copy.size() == original.size() &&
           "copy must have the same size");

    assert(copy.at(0).content() == original.at(0).content());

    assert(copy.at(1).content() == original.at(1).content());
}


// 4. Rule of Five - Move
void test_move() {

    Conversation original;

    original.append(Message(Role::User, "Hello"));
    original.append(Message(Role::Assistant, "Hi"));

    const Message* old_pointer = original.begin();

    Conversation moved(std::move(original));

    assert(moved.begin() == old_pointer &&
           "move must steal the original data pointer");

    assert(original.begin() == nullptr &&
           "moved-from data pointer must be null");

    assert(original.size() == 0 &&
           "moved-from size must be zero");

    assert(moved.size() == 2 &&
           "moved conversation must keep the messages");
}


// 5. Growth Behavior
void test_growth_behavior() {

    Conversation conv;

    conv.append(Message(Role::User, "1"));
    const Message* pointer1 = conv.begin();

    conv.append(Message(Role::User, "2"));
    const Message* pointer2 = conv.begin();

    assert(pointer1 != pointer2 &&
           "array must grow when capacity changes from 1 to 2");

    conv.append(Message(Role::User, "3"));
    const Message* pointer3 = conv.begin();

    assert(pointer2 != pointer3 &&
           "array must grow when capacity changes from 2 to 4");

    conv.append(Message(Role::User, "4"));
    const Message* pointer4 = conv.begin();

    assert(pointer3 == pointer4 &&
           "array should not grow when capacity is still 4");

    conv.append(Message(Role::User, "5"));
    const Message* pointer5 = conv.begin();

    assert(pointer4 != pointer5 &&
           "array must grow when capacity changes from 4 to 8");

    assert(conv.size() == 5 &&
           "size must remain correct after growth");

    for (std::size_t i = 0; i < conv.size(); ++i) {
        assert(conv.at(i).content() == std::to_string(i + 1) &&
               "messages must remain correct after growth");
    }
}


// 6. Scanner - Clean Text
void test_scanner_clean_text() {

    const std::string sentinel = "<|end_conversation|>";

    SentinelScanner scanner(sentinel);

    auto out = scanner.feed("Hello world");

    assert(out.sentinel_found == false &&
           "clean text must not find the sentinel");

    assert(out.safe_text == "Hello world" &&
           "clean text must be safe to print");
}



// 7. Scanner - Split Sentinel
void test_scanner_split_sentinel() {

    const std::string sentinel = "<|end_conversation|>";

    const std::string text = "Goodbye." + sentinel;

    for (std::size_t split = 0; split <= text.size(); ++split) {

        SentinelScanner scanner(sentinel);

        auto out1 = scanner.feed(text.substr(0, split));

        auto out2 = scanner.feed(text.substr(split));

        assert((out1.sentinel_found || out2.sentinel_found) &&
               "sentinel must be caught regardless of split point");

        assert(out1.safe_text + out2.safe_text == "Goodbye.");
    }
}


// 8. Scanner - False Alarms
void test_scanner_false_alarms() {

    const std::string sentinel = "<|end_conversation|>";

    SentinelScanner scanner(sentinel);

    auto out1 = scanner.feed("Hello <|end_world|>");

    auto out2 = scanner.flush();

    assert(out1.sentinel_found == false &&
           "false sentinel must not be detected");

    assert(out1.safe_text + out2.safe_text ==
           "Hello <|end_world|>");
}


// 9. Scanner - Bounded Memory
void test_scanner_bounded_memory() {

    const std::string sentinel = "<|end_conversation|>";

    SentinelScanner scanner(sentinel);

    const std::size_t stream_size = 4 * 1024 * 1024;

    for (std::size_t i = 0; i < stream_size; ++i) {

        scanner.feed("<");

        assert(scanner.pending_.size() <= sentinel.size() - 1 &&
               "pending must never exceed sentinel size minus one");
    }
}



// 10. Harness - Turn Limit
void test_harness_turn_limit() {

    std::vector<std::string> replies = {
        "Reply 1",
        "Reply 2",
        "Reply 3"
    };

    auto model = std::make_unique<TestModel>(replies);

    HarnessConfig config;
    config.max_turns = 2;

    Harness harness(std::move(model), config);

    TestInput input({"Hello", "Again", "More"});

    TestOutput output;

    StopReason reason = harness.run(input, output);

    assert(reason.kind == StopReason::Kind::TurnLimit &&
           "harness must stop when turn limit is reached");

    assert(harness.conversation().size() == 4 &&
           "two turns should create four messages");
}



// 11. Harness - Sentinel Halt
void test_harness_sentinel_halt() {

    std::vector<std::string> replies = {
        "Goodbye.<|end_conversation|>",
        "This should not happen"
    };

    auto model = std::make_unique<TestModel>(replies);

    HarnessConfig config;
    config.max_turns = 10;

    Harness harness(std::move(model), config);

    TestInput input({"Goodbye", "Hello again"});

    TestOutput output;

    StopReason reason = harness.run(input, output);

    assert(reason.kind == StopReason::Kind::Sentinel &&
           "harness must stop when sentinel is found");

    assert(output.text().find("<|end_conversation|>") ==
           std::string::npos &&
           "sentinel must not be printed");

    assert(output.text().find("This should not happen") ==
           std::string::npos &&
           "harness must stop immediately after sentinel");
}



// 12. Transcript Round-Trip
void test_transcript_round_trip() {

    const std::string filename = "test_transcript.txt";

    std::ofstream file(filename);

    file << "role: system\n";
    file << "Be concise.\n";
    file << "---\n";

    file << "role: user\n";
    file << "Hello\n";
    file << "---\n";

    file << "role: assistant\n";
    file << "Hi!\n";
    file << "---\n";

    file << "role: user\n";
    file << "Goodbye\n";
    file << "---\n";

    file << "role: assistant\n";
    file << "Goodbye.\n";

    file.close();

    ReplayModelClient replay(filename);

    assert(replay.system_message() == "Be concise." &&
           "system message must load correctly");

    Conversation conv;

    conv.append(Message(Role::System, "Be concise."));
    conv.append(Message(Role::User, "Hello"));

    Message reply1 = replay.generate(conv);

    assert(reply1.content() == "Hi!" &&
           "first assistant reply must match transcript");

    conv.append(reply1);

    conv.append(Message(Role::User, "Goodbye"));

    Message reply2 = replay.generate(conv);

    assert(reply2.content() == "Goodbye." &&
           "second assistant reply must match transcript");
}



// Run the 12 required tests
int main() {

    test_empty_conversation_bounds();

    test_system_message_ordering();

    test_copy();

    test_move();

    test_growth_behavior();

    test_scanner_clean_text();

    test_scanner_split_sentinel();

    test_scanner_false_alarms();

    test_scanner_bounded_memory();

    test_harness_turn_limit();

    test_harness_sentinel_halt();

    test_transcript_round_trip();

    return 0;
}